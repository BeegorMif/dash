#include "app/window.hpp"
#include "app/webInterface.hpp"
#include "app/pages/openauto.hpp"
#include "app/usb_monitor.hpp"
#include "app/arbiter.hpp"
#include <QWebEngineView>
#include <QWebChannel>
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

    openAutoFrame = new OpenAutoPage(arbiter, debugContainer);
    openAutoFrame->init();
    openAutoFrame->setParent(debugContainer);
    openAutoFrame->setVisible(false);
    openAutoFrame->raise();
    this->setCentralWidget(container);

    channel = new QWebChannel(webView->page());
    webInterface = new WebInterface(this);
    channel->registerObject("qtBridge", webInterface);
    webView->page()->setWebChannel(channel);

    connect(webInterface, &WebInterface::tabChangedSignal,
        this, &MainWindow::onTabChanged);

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
    qDebug() << "[Dash] openAutoFrame geometry:" << openAutoFrame->geometry();
    qDebug() << "[Dash] debugContainer geometry:" << debugContainer->geometry();

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

void MainWindow::onTabChanged(const QString &tabName)
{
    currentTab = tabName;

    if(tabName == "android_auto") {
        openAutoFrame->setVisible(true);
        openAutoFrame->setParent(debugContainer);
        openAutoFrame->raise();
        debugContainer->setVisible(true);
        debugContainer->raise();
    } else {
        openAutoFrame->setVisible(false);
    }
}