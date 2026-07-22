#pragma once

#include <QColor>
#include <QMainWindow>
#include <QObject>
#include <QSettings>
#include <QString>

#include "app/session.hpp"
#include "app/pages/page.hpp"
#include "openauto/Service/InputService.hpp"

class MainWindow;

class Arbiter : public QObject {
    Q_OBJECT

   public:
    Arbiter(MainWindow *window);

    QMainWindow *window();
    QSettings &settings() { return this->session_.settings_; }
    Session::Layout &layout() { return this->session_.layout_; }
    Session::System &system() { return this->session_.system_; }
    Session::Forge &forge() { return this->session_.forge_; }
    Session::Core &core() { return this->session_.core_; }
    Session::AndroidAuto &android_auto() { return this->session_.android_auto_; }
    void update() { this->session_.update(); }

   private:
    MainWindow *window_;
    Session session_;

};
