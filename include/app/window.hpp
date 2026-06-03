#pragma once

#include <QMainWindow>
#include <QWebEngineView>
#include <QStackedLayout>
#include <QTimer>
#include "AAHandler.hpp"
#include "app/arbiter.hpp"
#include <QEvent>
#include <QWidget>
#include <QMetaObject>

class OpenAutoPage;
class Arbiter;
class NodeBridge;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QRect geometry, QWidget *parent = nullptr);
    MainWindow* init(QRect geometry);
    NodeBridge* nodeBridge() const { return nodeBridge_; }
    OpenAutoPage* openAutoPage() const { return openAutoFrame; }
    bool blackoutMode = false;
    QWidget* blackoutOverlay = nullptr;
    QWidget* dimOverlay = nullptr;
    void onTabChanged(const QString &tabName, bool aaConnected);
    void onAAStatusChanged(bool connected);
    void updateAAFrameVisibility();
    void setBlackout(bool enable);
    void setDim(bool enable);
    void setNotificationPassthrough(bool passthrough);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:

private:
    void loadWebUi();
    void loadNotificationUi();
    QString resolveBaseUrl();

    QWebEngineView *webView = nullptr;
    QWebEngineView *notificationView = nullptr;

    QWidget *debugContainer = nullptr;
    OpenAutoPage *openAutoFrame = nullptr;

    QStackedLayout *stack = nullptr;
    NodeBridge* nodeBridge_ = nullptr;

    Arbiter arbiter;

public:
    QString currentTab = "android_auto";
    bool aaConnected = false;

    class BlackoutEventFilter : public QObject {
    public:
        BlackoutEventFilter(QObject* parent, MainWindow* window)
            : QObject(parent), mainWindow(window) {}
    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;
    private:
        MainWindow* mainWindow = nullptr;
    };

    void enableBlackoutTouchHandler(QWidget* targetFrame);
};