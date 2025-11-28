#include "app/session.hpp"
#include "app/window.hpp"

#include "app/arbiter.hpp"

Arbiter::Arbiter(MainWindow *window)
    : QObject()
    , window_(window)
    , session_(*this)
{
}

void Arbiter::set_mode(Session::Theme::Mode mode)
{
    this->theme().mode = mode;
    this->settings().setValue("Theme/mode", mode);

    this->session_.update();

    emit mode_changed(mode);
}

void Arbiter::toggle_mode()
{
    auto mode = (this->theme().mode == Session::Theme::Light) ? Session::Theme::Dark : Session::Theme::Light;
    this->set_mode(mode);
}

void Arbiter::set_curr_page(Page *page)
{
    if (this->layout().page_id(page) < 0 || !page->enabled())
        return;

    this->layout().curr_page->container()->reset();

    this->layout().curr_page = page;

    emit curr_page_changed(page);
}

void Arbiter::set_curr_page(int id)
{
    this->set_curr_page(this->layout().page(id));
}

void Arbiter::set_page(Page *page, bool enabled)
{
    auto id = this->layout().page_id(page);
    if (id < 0 || !page->toggleale())
        return;

    page->enable(enabled);
    this->settings().beginGroup("Layout");
    this->settings().beginGroup("Page");
    this->settings().setValue(QString::number(id), page->enabled());
    this->settings().endGroup();
    this->settings().endGroup();

    emit page_changed(page, enabled);
}

void Arbiter::set_action(Action *action, QString key)
{
    auto id = QString::number(this->core().action_id(action));
    if (id < 0)
        return;

    action->set(key);
    this->settings().beginGroup("Core");
    this->settings().beginGroup("Action");
    if (key.isNull())
        this->settings().remove(id);
    else
        this->settings().setValue(id, key);
    this->settings().endGroup();
    this->settings().endGroup();

    emit action_changed(action, key);
}

QMainWindow *Arbiter::window()
{
    return this->window_;
}
