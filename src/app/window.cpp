#include "app/window.hpp"
#include "app/pages/openauto.hpp"
#include "app/usb_monitor.hpp"
#include "app/arbiter.hpp"
#include "app/nodeBridge.hpp"
#include <QWebEngineView>
#include <QStackedLayout>
#include <QTimer>
#include <QDebug>
#include <QTcpSocket>
#include <QUrl>
#include <QResizeEvent>

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

    blackoutOverlay->setStyleSheet("background: rgba(0, 0, 0, 0%);");
    blackoutOverlay->hide();

    blackoutOverlay->installEventFilter(this);
    
    loadWebUi();

    QTimer::singleShot(0, this, [this]() {
        this->onTabChanged("android_auto");
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
    qDebug() << "[Dash] MainWindow shown";
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
        blackoutOverlay->setGeometry(this->rect());
        blackoutOverlay->raise();
        blackoutOverlay->show();
    } else {
        blackoutOverlay->hide();
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == blackoutOverlay && blackoutMode) {

        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::TouchBegin)
        {
            if (nodeBridge_) 
                nodeBridge_->sendCustomMessage(R"({"type":"blackout","enabled":false})");
            this->setBlackout(false);
            return true; // block event so it doesn't hit UI underneath
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onTabChanged(const QString &tabName)
{
    currentTab = tabName;

    if(tabName == "android_auto") {
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