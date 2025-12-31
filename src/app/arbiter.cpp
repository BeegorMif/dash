#include "app/session.hpp"
#include "app/window.hpp"

#include "app/arbiter.hpp"

Arbiter::Arbiter(MainWindow *window)
    : QObject()
    , window_(window)
    , session_(*this)
{
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
    emit page_changed(page, enabled);
}

QMainWindow *Arbiter::window()
{
    return this->window_;
}
