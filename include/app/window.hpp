#pragma once

#include <QMainWindow>
#include <QString>
#include <QEvent>
#include <QResizeEvent>

#include "app/arbiter.hpp"

class QWebEngineView;
class QStackedLayout;
class QWidget;
class QResizeEvent;

class OpenAutoPage;
class NodeBridge;

class ClickFilter : public QObject {
    Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            qDebug() << "Clicked on" << obj->objectName();
        }
        return QObject::eventFilter(obj, event);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QRect geometry, QWidget *parent = nullptr);
    MainWindow* init(QRect geometry);

    NodeBridge* nodeBridge() const { return nodeBridge_; }

    /* ---------- Android Auto ---------- */
    void onTabChanged(const QString &tabName, bool aaConnected);
    void onAAStatusChanged(bool connected);
    void updateAAFrameVisibility();

    /* ---------- Overlays ---------- */
    void setDim(bool enable);
    void setBlackout(bool enable);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    /* ---------- Helpers ---------- */
    void loadWebUi();
    void syncOverlayZOrder();

    /* ---------- Core widgets ---------- */
    QWebEngineView* webView = nullptr;
    OpenAutoPage* openAutoFrame = nullptr;

    /* ---------- Overlays ---------- */
    QWidget* dimOverlay = nullptr;
    QWidget* blackoutOverlay = nullptr;

    /* ---------- Layout ---------- */
    QStackedLayout* stack = nullptr;

    /* ---------- Backend ---------- */
    NodeBridge* nodeBridge_ = nullptr;
    Arbiter arbiter;

    /* ---------- State ---------- */
    QString currentTab = "android_auto";
    bool aaConnected = false;

    /* ---------- Blackout input handler ---------- */
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
