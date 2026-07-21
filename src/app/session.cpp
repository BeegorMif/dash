#include <algorithm>

#include <QCoreApplication>
#include <QFile>
#include <QHBoxLayout>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSize>
#include <QSlider>
#include <QTextStream>

#include "app/arbiter.hpp"
#include "aasdk_proto/ButtonCodeEnum.pb.h"

#include "app/session.hpp"


Session::Layout::Layout(QSettings &settings, Arbiter &arbiter)
    : scale(settings.value("Layout/scale", 1.0).toDouble())
    , openauto_page(new OpenAutoPage(arbiter))
    , curr_page(nullptr)
{
    this->pages_ = {
        this->openauto_page,
    };

    for (auto page : this->pages_) {
        if (page->enabled()) {
            this->curr_page = page;
            break;
        }
    }
}

Page *Session::Layout::next_enabled_page(Page *page) const
{
    auto id = this->page_id(page);
    do {
        id = (id + 1) % this->pages_.size();
    } while (!this->page(id)->enabled());

    return this->page(id);
}


Session::System::System(QSettings &settings, Arbiter &arbiter)
{
}

QFrame *Session::Forge::br(bool vertical)
{
    auto br = new QFrame();
    br->setLineWidth(2);
    br->setFrameShape(vertical ? QFrame::VLine : QFrame::HLine);
    br->setFrameShadow(QFrame::Plain);

    return br;
}

void Session::Forge::to_touch_scroller(QAbstractScrollArea *area)
{
    auto policy = QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff);
    auto properties = QScroller::scroller(area->viewport())->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, policy);
    properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, policy);

    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(area->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller::scroller(area->viewport())->setScrollerProperties(properties);
}

void Session::Forge::symbolize(QAbstractButton *button)
{
    auto policy = button->sizePolicy();
    policy.setRetainSizeWhenHidden(true);
    button->setSizePolicy(policy);
    button->setFocusPolicy(Qt::NoFocus);
    button->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

Session::Forge::Forge(Arbiter &arbiter)
    : arbiter_(arbiter)
{
}

Session::AndroidAuto::AndroidAuto(Arbiter &arbiter)
    : handler(new AAHandler())
{

}

Session::Core::Core(QSettings &settings, Arbiter &arbiter)
{
}

Session::Session(Arbiter &arbiter)
    : settings_()
    , layout_(settings_, arbiter)
    , system_(settings_, arbiter)
    , forge_(arbiter)
    , android_auto_(arbiter)
    , core_(settings_, arbiter)
{
}