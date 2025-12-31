#include <algorithm>

#include <QCoreApplication>
#include <QFile>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSize>
#include <QSlider>
#include <QTextStream>

#include "app/arbiter.hpp"
#include "app/pages/webview.hpp"
#include "aasdk_proto/ButtonCodeEnum.pb.h"

#include "app/session.hpp"


Session::Layout::Layout(QSettings &settings, Arbiter &arbiter)
    : scale(settings.value("Layout/scale", 1.0).toDouble())
    , openauto_page(new OpenAutoPage(arbiter))
    , curr_page(nullptr)
{
    this->pages_ = {
        this->openauto_page,
        new WebviewPage(arbiter)
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

QFont Session::Forge::font(int size, bool mono) const
{
    auto name = mono ? "Titillium Web" : "Montserrat";
    auto scaled = size * this->arbiter_.layout().scale;

    return QFont(name, scaled);
}

Session::AndroidAuto::AndroidAuto(Arbiter &arbiter)
    : handler(new AAHandler())
{

}

Session::Core::Core(QSettings &settings, Arbiter &arbiter)
{
    AAHandler *aa_handler = arbiter.android_auto().handler;

    QFontDatabase::addApplicationFont(":/fonts/Titillium_Web/TitilliumWeb-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Montserrat/Montserrat-LightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Montserrat/Montserrat-Regular.ttf");

    if (qApp)
        qApp->setFont(arbiter.forge().font(14));

}

Session::Session(Arbiter &arbiter)
    : settings_()
    , layout_(settings_, arbiter)
    , system_(settings_, arbiter)
    , forge_(arbiter)
    , core_(settings_, arbiter)
    , android_auto_(arbiter)
{
}