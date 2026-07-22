#include "app/session.hpp"
#include "app/window.hpp"

#include "app/arbiter.hpp"

Arbiter::Arbiter(MainWindow *window)
    : QObject()
    , window_(window)
    , session_(*this)
{
}

QMainWindow *Arbiter::window()
{
    return this->window_;
}
