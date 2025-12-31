#pragma once

#include <array>

#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QPalette>
#include <QWidget>

#include "app/pages/openauto.hpp"
#include "app/pages/page.hpp"
#include "app/services/bluetooth.hpp"
#include "app/services/clock.hpp"
#include "AAHandler.hpp"

class Arbiter;

class Session {
   public:

    struct Layout {

        double scale;
        OpenAutoPage *openauto_page;
        Page *curr_page;

        Layout(QSettings &settings, Arbiter &arbiter);
        Page *next_enabled_page(Page *page) const;

        const QList<Page *> &pages() const { return this->pages_; }
        Page *page(int id) const { return this->pages_.value(id, nullptr); }
        int page_id(Page *page) const { return this->pages_.indexOf(page); }

       private:
        QList<Page *> pages_;
    };

    struct System {
        static const char *SHUTDOWN_CMD;
        static const char *SCREENBLANK_CMD;
        static const char *SCREENBLANK_OFF_CMD;
        static const char *REBOOT_CMD;

        Clock clock;
        Bluetooth bluetooth;

        System(QSettings &settings, Arbiter &arbiter);
    };

    struct Forge {
        static QFrame *br(bool vertical = false);
        static void to_touch_scroller(QAbstractScrollArea *area);
        static void symbolize(QAbstractButton *button);

        Forge(Arbiter &arbiter);
        void iconize(QString name, QAbstractButton *button, uint8_t size) const;
        void iconize(QString name, QString alt_name, QAbstractButton *button, uint8_t size) const;
        void iconize(QIcon &icon, QAbstractButton *button, uint8_t size) const;
        QFont font(int size, bool mono = false) const;

       private:
        Arbiter &arbiter_;
    };

    struct AndroidAuto {
        AAHandler *handler;

        AndroidAuto(Arbiter &arbiter);
    };

    struct Core {
        bool cursor;

        Core(QSettings &settings, Arbiter &arbiter);
        void set_cursor() const;

    };

    Session(Arbiter &arbiter);
    void update();

    friend class Arbiter;

   private:
    QSettings settings_;
    Layout layout_;
    System system_;
    Forge forge_;
    AndroidAuto android_auto_;
    Core core_;
};
