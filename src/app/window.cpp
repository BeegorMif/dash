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

static constexpr int MENU_WIDTH = 70;

MainWindow::MainWindow(QRect geometry, QWidget *parent)
    : QMainWindow(parent)
    , arbiter(this)
{
    setAttribute(Qt::WA_TranslucentBackground, true);

    /* ---------------- Central + stack ---------------- */

    auto central = new QWidget(this);
    central->setObjectName("CentralWidget");
    setCentralWidget(central);


    stack = new QStackedLayout(central);
    stack->setStackingMode(QStackedLayout::StackAll);
    stack->setContentsMargins(0, 0, 300, 0);
    stack->setSpacing(0);

    /* ---------------- Web UI ---------------- */

    webView = new QWebEngineView(central);
    webView->setObjectName("WebView");
    stack->addWidget(webView);

    /* ---------------- Android Auto frame ---------------- */

    nodeBridge_ = new NodeBridge(this, this);
    nodeBridge_->connectToServer(QUrl("ws://localhost:3001"));
    nodeBridge_->setMainWindow(this);

    openAutoFrame = new OpenAutoPage(arbiter, central, nodeBridge_);
    openAutoFrame->setObjectName("OpenAutoFrame");
    openAutoFrame->setNodeBridge(nodeBridge_);
    openAutoFrame->init();
    openAutoFrame->hide();
    stack->addWidget(openAutoFrame);

    /* ---------------- Dim overlay (visual only) ---------------- */

    dimOverlay = new QWidget(central);
    dimOverlay->setObjectName("DimOverlay");
    dimOverlay->setStyleSheet("background: rgba(0,0,0,150);");
    dimOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    dimOverlay->setParent(central);
    dimOverlay->hide();
    stack->addWidget(dimOverlay);

    /* ---------------- Blackout overlay (modal) ---------------- */

    blackoutOverlay = new BlackoutOverlayWidget(central, this);
    blackoutOverlay->setObjectName("BlackoutOverlay");
    blackoutOverlay->setStyleSheet("background: rgba(0,0,0,180);");
    blackoutOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    blackoutOverlay->setParent(central);
    blackoutOverlay->hide();
    stack->addWidget(blackoutOverlay);

    qApp->installEventFilter(new ClickFilter());


    loadWebUi();

    QTimer::singleShot(0, this, [this]() {
        onTabChanged("android_auto", false);
    });

    connect(openAutoFrame, &OpenAutoPage::aaStatusChanged,
            this, &MainWindow::onAAStatusChanged);
}

/* ========================================================= */
/* ==================== Geometry ============================ */
/* ========================================================= */

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    QRect full = centralWidget()->rect();

    // webView->setGeometry(full);
    dimOverlay->setGeometry(full);
    blackoutOverlay->setGeometry(full);

    // openAutoFrame->setGeometry(
    //     0,
    //     0,
    //     full.width() - MENU_WIDTH,
    //     full.height()
    // );
}

/* ========================================================= */
/* ==================== Z-Order ============================== */
/* ========================================================= */

void MainWindow::syncOverlayZOrder()
{
    // Bottom → top (authoritative)
    webView->raise();

    if (openAutoFrame->isVisible())
        openAutoFrame->raise();

    if (dimOverlay->isVisible())
        dimOverlay->raise();

    if (blackoutOverlay->isVisible())
        blackoutOverlay->raise();
}

/* ========================================================= */
/* ==================== AA Visibility ======================= */
/* ========================================================= */

void MainWindow::onTabChanged(const QString &tabName, bool aaConnectedFlag)
{
    currentTab = tabName;
    aaConnected = aaConnectedFlag;
    updateAAFrameVisibility();
}

void MainWindow::onAAStatusChanged(bool connected)
{
    aaConnected = connected;
    updateAAFrameVisibility();
}

void MainWindow::updateAAFrameVisibility()
{
    const bool showAA =
        (currentTab == "android_auto") && aaConnected;

    if (showAA) {
        openAutoFrame->show();
    } else {
        openAutoFrame->hide();
    }

    syncOverlayZOrder();
}

/* ========================================================= */
/* ==================== Overlays ============================ */
/* ========================================================= */

void MainWindow::setDim(bool enable)
{
    dimOverlay->setVisible(enable);
    syncOverlayZOrder();
}

void MainWindow::setBlackout(bool enable)
{
    blackoutOverlay->setVisible(enable);
    syncOverlayZOrder();
}

/* ========================================================= */
/* ==================== Web UI ============================== */
/* ========================================================= */

void MainWindow::loadWebUi()
{
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 5173);

    if (socket.waitForConnected(100)) {
        socket.disconnectFromHost();
        webView->load(QUrl("http://127.0.0.1:5173"));
    } else {
        webView->load(QUrl("http://127.0.0.1:3000"));
    }
}
