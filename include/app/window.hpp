#pragma once

#include <QMainWindow>
#include <QWebEngineView>
#include <QWebChannel>
#include <QStackedLayout>
#include <QTimer>
#include "AAHandler.hpp"
#include "app/arbiter.hpp"

class WebInterface;
class OpenAutoPage;
class UsbMonitor;
class ShutdownPage;
class Arbiter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QRect geometry, QWidget *parent = nullptr);
    MainWindow* init(QRect geometry);
    void onTabChanged(const QString &tabName);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:

private:
    void loadWebUi();

private:
    QWebEngineView *webView = nullptr;
    QWebChannel *channel = nullptr;
    WebInterface *webInterface = nullptr;

    QWidget *debugContainer = nullptr;
    OpenAutoPage *openAutoFrame = nullptr;

    UsbMonitor *usbMonitor = nullptr;
    ShutdownPage *shutdownPage = nullptr;
    QTimer *shutdownDelayTimer = nullptr;

    QStackedLayout *stack = nullptr;

    Arbiter arbiter;

public:
    QString currentTab = "android_auto";
};
