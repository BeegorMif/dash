#pragma once

#include <QColor>
#include <QMainWindow>
#include <QObject>
#include <QSettings>
#include <QString>

#include "app/action.hpp"
#include "app/session.hpp"
#include "app/pages/page.hpp"
#include "openauto/Service/InputService.hpp"

class MainWindow;

class Arbiter : public QObject {
    Q_OBJECT

   public:
    Arbiter(MainWindow *window);
    void set_mode(Session::Theme::Mode mode);
    void toggle_mode();
    void set_curr_page(Page *page);
    void set_curr_page(int id);
    void set_page(Page *page, bool enabled);
    void set_action(Action *action, QString key);

    QMainWindow *window();
    QSettings &settings() { return this->session_.settings_; }
    Session::Theme &theme() { return this->session_.theme_; }
    Session::Layout &layout() { return this->session_.layout_; }
    Session::System &system() { return this->session_.system_; }
    Session::Forge &forge() { return this->session_.forge_; }
    Session::Core &core() { return this->session_.core_; }
    Session::AndroidAuto &android_auto() { return this->session_.android_auto_; }
    void update() { this->session_.update(); }

   private:
    MainWindow *window_;
    Session session_;

   signals:
    void mode_changed(Session::Theme::Mode mode);
    void curr_page_changed(Page *page);
    void page_changed(Page *page, bool enabled);
    void action_changed(Action *action, QString key);
};
