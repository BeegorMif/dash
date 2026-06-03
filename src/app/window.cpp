#include "app/window.hpp"
#include "app/pages/openauto.hpp"
#include "app/arbiter.hpp"
#include "app/nodeBridge.hpp"
#include <QWebEngineView>
#include <QStackedLayout>
#include <QTimer>
#include <QDebug>
#include <QTcpSocket>
#include <QUrl>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QTouchEvent>

MainWindow::MainWindow(QRect geometry, QWidget *parent)
    : QMainWindow(parent)
    , arbiter(this)
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    auto container = new QWidget(this);
    stack = new QStackedLayout(container);
    stack->setStackingMode(QStackedLayout::StackAll);
    stack->setContentsMargins(0,0,70,0);
    stack->setSpacing(0);

    webView = new QWebEngineView(container);
    webView->setObjectName("WebView");
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stack->addWidget(webView);

    int menuWidth = 70;
    debugContainer = new QWidget(container);
    debugContainer->setObjectName("OA_DebugContainer");
    debugContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    debugContainer->setGeometry(0, 0, container->width(), container->height());

    stack->addWidget(debugContainer);

    QWidget *menuSpacer = new QWidget(debugContainer);
    menuSpacer->setFixedWidth(menuWidth);
    menuSpacer->setGeometry(debugContainer->width() - menuWidth, 0, menuWidth, debugContainer->height());
    menuSpacer->setAttribute(Qt::WA_TransparentForMouseEvents);
    menuSpacer->setStyleSheet("background: transparent;");
    menuSpacer->raise();

    nodeBridge_ = new NodeBridge(this, this);
    nodeBridge_->connectToServer(QUrl("ws://localhost:3001"));
    nodeBridge_->setMainWindow(this);

    openAutoFrame = new OpenAutoPage(arbiter, debugContainer, nodeBridge_);
    openAutoFrame->setNodeBridge(nodeBridge_);
    openAutoFrame->init();
    openAutoFrame->setParent(debugContainer);
    openAutoFrame->setVisible(false);
    openAutoFrame->raise();
    this->setCentralWidget(container);

    blackoutOverlay = new QWidget(this);
    blackoutOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    blackoutOverlay->setAttribute(Qt::WA_AcceptTouchEvents);

    blackoutOverlay->setStyleSheet("background: rgba(0, 0, 0, 150);");
    blackoutOverlay->hide();

    dimOverlay = new QWidget(this);
    dimOverlay->setObjectName("DimOverlay");
    dimOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    dimOverlay->setAttribute(Qt::WA_AcceptTouchEvents, false);
    dimOverlay->setStyleSheet("background: rgba(0, 0, 0, 125);"); // softer than blackout
    dimOverlay->hide();

    loadWebUi();

    QTimer::singleShot(0, this, [this]() {
        this->onTabChanged("android_auto", false);
    });
    connect(openAutoFrame, &OpenAutoPage::aaStatusChanged,
        this, [this](bool connected){
    DASH_LOG(debug) << "[MainWindow] AA Status changed: " << connected;
        this->onAAStatusChanged(connected);
    });
}

MainWindow* MainWindow::init(QRect geometry)
{
    this->setGeometry(geometry);
    return this;
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    int menuWidth = 70;
    if(debugContainer) {
        debugContainer->setGeometry(0, 0, this->centralWidget()->width() - menuWidth, this->centralWidget()->height());
        if(openAutoFrame) {
            openAutoFrame->setGeometry(0, 0, debugContainer->width(), debugContainer->height());
        }
    }
    if (blackoutOverlay)
        blackoutOverlay->setGeometry(rect());
    if (dimOverlay)
        dimOverlay->setGeometry(rect());
}

void MainWindow::loadWebUi()
{
    if(!webView) return;

    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 5173);
    if(socket.waitForConnected(100)) {
        socket.disconnectFromHost();
        webView->load(QUrl("http://127.0.0.1:5173"));
    } else {
        // fallback to prod port
        webView->load(QUrl("http://127.0.0.1:3000"));
    }
}

void MainWindow::setBlackout(bool enable)
{
    blackoutMode = enable;

    if (blackoutMode) {
        QWidget* topFrame = (openAutoFrame && openAutoFrame->isVisible()) ? openAutoFrame : debugContainer;
        enableBlackoutTouchHandler(blackoutOverlay);
        blackoutOverlay->setGeometry(this->rect());
        blackoutOverlay->raise();
        blackoutOverlay->show();
    } else {
        blackoutOverlay->hide();
    }
}

void MainWindow::enableBlackoutTouchHandler(QWidget* targetFrame)
{
    if (!targetFrame) return;

    auto filter = new BlackoutEventFilter(targetFrame, this);
    targetFrame->installEventFilter(filter);
}

bool MainWindow::BlackoutEventFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (!mainWindow) return QObject::eventFilter(obj, event);

    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::TouchBegin)
    {
        if (mainWindow->nodeBridge())
            mainWindow->nodeBridge()->sendCustomMessage(R"({"type":"blackout","action":"update","payload":false})");

        mainWindow->setBlackout(false);
        obj->removeEventFilter(this);
        delete this;

        return true;
    }

    return QObject::eventFilter(obj, event);
}

void MainWindow::onTabChanged(const QString &tabName, bool aaConnectedFlag)
{
    currentTab = tabName;
    aaConnected = aaConnectedFlag; // store the latest AA status
    DASH_LOG(debug) << "Tab Change To:" << tabName.toStdString()
                    << " AA Connected:" << aaConnected;

    updateAAFrameVisibility();
}
void MainWindow::onAAStatusChanged(bool connected)
{
    aaConnected = connected;
    updateAAFrameVisibility();
}
void MainWindow::updateAAFrameVisibility()
{
    const bool showAAFrame = (currentTab == "android_auto") && aaConnected;

    if(showAAFrame) {
        openAutoFrame->setVisible(true);
        openAutoFrame->setParent(debugContainer);
        openAutoFrame->raise();
        debugContainer->setVisible(true);
        debugContainer->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        debugContainer->raise();
    } else {
        openAutoFrame->setVisible(false);
        debugContainer->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
}
void MainWindow::setDim(bool enable)
{
    if (enable) {
        dimOverlay->setGeometry(this->rect());
        dimOverlay->raise();
        dimOverlay->show();
    } else {
        dimOverlay->hide();
    }
}