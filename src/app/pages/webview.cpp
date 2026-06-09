#include "app/arbiter.hpp"
#include "app/pages/webview.hpp"

#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProcess>
#include <QDebug>

// ---------------------- Subclass QWebEnginePage for console logging ----------------------
class DebugWebEnginePage : public QWebEnginePage {
    Q_OBJECT
public:
    DebugWebEnginePage(QObject* parent = nullptr) : QWebEnginePage(parent) {}

protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                  const QString &message,
                                  int lineNumber,
                                  const QString &sourceID) override
    {
        QString levelStr;
        switch(level) {
            case QWebEnginePage::InfoMessageLevel: levelStr = "Info"; break;
            case QWebEnginePage::WarningMessageLevel: levelStr = "Warning"; break;
            case QWebEnginePage::ErrorMessageLevel: levelStr = "Error"; break;
        }
        // qDebug() << "[JS Console][" << levelStr << "]" << message
        //          << "(Line:" << lineNumber << "Source:" << sourceID << ")";
    }
};


// ---------------------- WebviewPage ----------------------
WebviewPage::WebviewPage(Arbiter &arbiter, QWidget *parent)
    : QStackedWidget(parent)
    , Page(arbiter, "Webview", true, this)
    , view_(nullptr)
    , nodeOnline_(false)
    , pageLoaded_(false)
    , arbiter_(arbiter)
{
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu-sandbox --no-sandbox --use-gl=egl");
}

void WebviewPage::init() {
    QWidget* widget = loadWebview();
    this->addWidget(widget);
    this->setCurrentWidget(widget);

    startNodeReconnect();
    startMediaMetadataListener();
}

QWidget* WebviewPage::loadWebview() {
    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0,0,0,0);

    // Use debug page for console logs
    DebugWebEnginePage* page = new DebugWebEnginePage();
    view_ = new QWebEngineView(container);
    view_->setPage(page);

    QWebEngineSettings* settings = view_->settings();
    settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineProfile* profile = QWebEngineProfile::defaultProfile();
    profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);

        
    // Page loaded signal
    connect(view_, &QWebEngineView::loadFinished, this, [this](bool ok){
        pageLoaded_ = ok;
        if(ok){
            qDebug() << "[WebView] Page loaded";

            // Process queued media events
            QTimer::singleShot(50, this, [this](){
                while(!mediaEventQueue_.isEmpty()){
                    processMediaEvent(mediaEventQueue_.dequeue());
                }
            });
        } else {
            qWarning() << "[WebView] Failed to load page";
        }
    });

    view_->load(QUrl("http://localhost:3000"));
    view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(view_);
    container->setLayout(layout);
    return container;
}

void WebviewPage::showEvent(QShowEvent* event) {
    QStackedWidget::showEvent(event);
    if(view_) view_->reload();
}

// ---------------------- Node.js auto-reconnect ----------------------
void WebviewPage::startNodeReconnect() {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this); // create once
    QTimer* reconnectTimer = new QTimer(this);
    connect(reconnectTimer, &QTimer::timeout, this, [this, manager](){
        if(!view_) return;
        QNetworkRequest request(QUrl("http://127.0.0.1:3000"));
        QNetworkReply* reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, this, [this, reply](){
            if(reply->error() == QNetworkReply::NoError){
                if(!nodeOnline_){
                    nodeOnline_ = true;
                    view_->reload();
                }
            } else {
                nodeOnline_ = false;
            }
            reply->deleteLater();
        });
    });
    reconnectTimer->start(5000);
}

// ---------------------- Media metadata listener ----------------------
void WebviewPage::startMediaMetadataListener() {
    QProcess* logProcess = new QProcess(this);
    logProcess->start("tail", {"-F", "/path/to/openauto/log.txt"}); // replace path

    connect(logProcess, &QProcess::readyReadStandardOutput, this, [this, logProcess](){
        while(logProcess->canReadLine()){
            QString strLine = QString::fromUtf8(logProcess->readLine()).trimmed();
            if(strLine.contains("[MediaStatusService] Metadata update")){
                QRegExp rx("track: (.*), artist: (.*), length: (\\d+)");
                if(rx.indexIn(strLine) != -1){
                    MediaEvent event;
                    event.track = rx.cap(1);
                    event.artist = rx.cap(2);
                    event.length = rx.cap(3).toInt();
                    qDebug() << "[Media Metadata]" << event.track << "-" << event.artist << "(" << event.length << "s)";
                    queueOrProcessMediaEvent(event);
                }
            }
        }
    });
}

// ---------------------- Queue or process ----------------------
void WebviewPage::queueOrProcessMediaEvent(const MediaEvent &event){
    if(pageLoaded_){
        processMediaEvent(event);
    } else {
        mediaEventQueue_.enqueue(event);
    }
}

// ---------------------- Run JS ----------------------
void WebviewPage::processMediaEvent(const MediaEvent &event){
    if(!view_) return;

    QString track = event.track.toHtmlEscaped();
    QString artist = event.artist.toHtmlEscaped();

    QString js = QString("updateNowPlaying(\"%1\",\"%2\",%3)")
                     .arg(track)
                     .arg(artist)
                     .arg(event.length);

    view_->page()->runJavaScript(js, [js](const QVariant &v){
        qDebug() << "[WebView JS]" << js << "executed";
    });
}

#include "webview.moc"
