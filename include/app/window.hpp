#pragma once

#include <QButtonGroup>
#include <QKeyEvent>
#include <QMainWindow>
#include <QObject>
#include <QShowEvent>
#include <QStackedLayout>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QStackedWidget>

#include "app/config.hpp"
#include "app/pages/openauto.hpp"
#include "app/pages/page.hpp"
#include "app/pages/shutdown_page.hpp"
#include "app/usb_monitor.hpp"

#include "app/arbiter.hpp"

class FullscreenToggle;

class Dash : public QWidget {
    Q_OBJECT

   public:
    Dash(Arbiter &arbiter);
    void init();

   private:
    struct NavRail {
        QButtonGroup group;
        QElapsedTimer timer;
        QVBoxLayout *layout;

        NavRail();
    };

    struct Body {
        QVBoxLayout *layout;
        QStackedLayout *frame;

        Body();
    };

    Arbiter &arbiter;
    NavRail rail;
    Body body;

    void set_page(Page *page);
    QWidget *power_control() const;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(QRect geometry);
    void set_fullscreen(Page *page);

   protected:
    void showEvent(QShowEvent *event) override;

   private slots:
    void startShutdownCountdown();
    void cancelShutdownCountdown();
    void performShutdown();
    
    private:
    Arbiter arbiter;
    QStackedWidget *stack;
    ShutdownPage *shutdownPage = nullptr;
    UsbMonitor *usbMonitor = nullptr;
    QTimer *shutdownDelayTimer = nullptr;

    MainWindow *init(QRect geometry);
};
