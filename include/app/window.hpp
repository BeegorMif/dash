#pragma once

#include <QMainWindow>
#include <QWebEngineView>
#include <QStackedLayout>
#include <QTimer>
#include "AAHandler.hpp"
#include "app/arbiter.hpp"

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
    bool blackoutMode = false;
    QWidget* blackoutOverlay = nullptr;
    void onTabChanged(const QString &tabName, bool aaConnected);
    void onAAStatusChanged(bool connected);
    void updateAAFrameVisibility();
    void setBlackout(bool enable);
    bool eventFilter(QObject* obj, QEvent* event) override;

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:

private:
    void loadWebUi();

private:
    QWebEngineView *webView = nullptr;

    QWidget *debugContainer = nullptr;
    OpenAutoPage *openAutoFrame = nullptr;

    QStackedLayout *stack = nullptr;
    NodeBridge* nodeBridge_ = nullptr;

    Arbiter arbiter;

public:
    QString currentTab = "android_auto";
    bool aaConnected = false;
};
