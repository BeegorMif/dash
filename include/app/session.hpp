#pragma once

#include <array>

#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QList>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QPalette>
#include <QWidget>

#include "app/pages/openauto.hpp"
#include "app/pages/page.hpp"
#include "AAHandler.hpp"

class Arbiter;

class Session {
   public:

    struct Layout {

        double scale;
        OpenAutoPage *openauto_page;

        Layout(QSettings &settings, Arbiter &arbiter);

    };

    struct System {
        System(QSettings &settings, Arbiter &arbiter);
    };

    struct Forge {
        static QFrame *br(bool vertical = false);
        static void to_touch_scroller(QAbstractScrollArea *area);
        static void symbolize(QAbstractButton *button);

        Forge(Arbiter &arbiter);

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
