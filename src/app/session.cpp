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
#include "app/pages/settings.hpp"
#include "app/pages/blackout.hpp"
#include "app/pages/webview.hpp"
#include "app/utilities/icon_engine.hpp"
#include "aasdk_proto/ButtonCodeEnum.pb.h"

#include "app/session.hpp"


Session::Theme::Mode Session::Theme::from_str(QString mode)
{
    // defaults to light mode if unknown
    return (mode == "Dark") ? Session::Theme::Dark : Session::Theme::Light;
}

QString Session::Theme::to_str(Session::Theme::Mode mode)
{
    switch (mode) {
        case Session::Theme::Light:
            return "Light";
            break;
        case Session::Theme::Dark:
            return "Dark";
            break;
        default:
            return QString();
    }
}

Session::Theme::Theme(QSettings &settings)
    : mode(static_cast<Theme::Mode>(settings.value("Theme/mode", Session::Theme::Light).toUInt()))
{
    this->colors_[Session::Theme::Light] = QColor(settings.value("Theme/Color/light", "#000000").toString());
    this->colors_[Session::Theme::Dark] = QColor(settings.value("Theme/Color/dark", "#ffffff").toString());
}

QPalette Session::Theme::palette() const
{
    QPalette palette;
    auto color = this->color();
    palette.setColor(QPalette::Base, color);
    color.setAlphaF(.5);
    palette.setColor(QPalette::AlternateBase, color);

    return palette;
}

Session::Layout::Layout(QSettings &settings, Arbiter &arbiter)
    : scale(settings.value("Layout/scale", 1.0).toDouble())
    , openauto_page(new OpenAutoPage(arbiter))
    , curr_page(nullptr)
{
    this->pages_ = {
        this->openauto_page,
        new SettingsPage(arbiter),
        new BlackoutPage(arbiter),
        new WebviewPage(arbiter)
    };

    settings.beginGroup("Layout");
    settings.beginGroup("Page");
    for (int i = 0; i < this->pages_.size(); i++) {
        auto page = this->page(i);
        if (page->toggleale()) {
            if (!settings.value(QString::number(i), true).toBool())
                page->enable(false);
        }
    }
    settings.endGroup();
    settings.endGroup();

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

const char *Session::System::SCREENBLANK_CMD = "sudo ddcutil setvcp D6 04";
const char *Session::System::SCREENBLANK_OFF_CMD = "sudo ddcutil setvcp D6 01";
const char *Session::System::SHUTDOWN_CMD = "sudo shutdown -h --no-wall now";
const char *Session::System::REBOOT_CMD = "sudo shutdown -r now";


Session::System::System(QSettings &settings, Arbiter &arbiter)
    : clock()
    , bluetooth(arbiter)
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

void Session::Forge::iconize(QString name, QAbstractButton *button, uint8_t size) const
{
    this->iconize(name, QString(), button, size);
}

void Session::Forge::iconize(QString name, QString alt_name, QAbstractButton *button, uint8_t size) const
{
    QIcon icon(new IconEngine(this->arbiter_, QString(":/icons/%1.svg").arg(name), false));
    if (!alt_name.isNull())
        icon.addFile(QString(":/icons/%1.svg").arg(alt_name), QSize(), QIcon::Normal, QIcon::On);
    this->iconize(icon, button, size);
}

void Session::Forge::iconize(QIcon &icon, QAbstractButton *button, uint8_t size) const
{
    auto scaled = size * this->arbiter_.layout().scale;
    button->setIconSize(QSize(scaled, scaled));
    button->setIcon(icon);
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
    this->stylesheets_[Session::Theme::Light] = this->parse_stylesheet(":/stylesheets/light.qss");
    this->stylesheets_[Session::Theme::Dark] = this->parse_stylesheet(":/stylesheets/dark.qss");
    AAHandler *aa_handler = arbiter.android_auto().handler;
    this->actions_ = {
        new Action("Android Auto Left", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::LEFT, actionState); }, arbiter.window()),
        new Action("Android Auto Enter", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::ENTER, actionState); }, arbiter.window()),
        new Action("Android Auto Right", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::RIGHT, actionState); }, arbiter.window()),
        new Action("Android Auto Up", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::UP, actionState); }, arbiter.window()),
        new Action("Android Auto Down", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::DOWN, actionState); }, arbiter.window()),
        new Action("Android Auto Back", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::BACK, actionState); }, arbiter.window()),
        new Action("Android Auto Home", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::HOME, actionState); }, arbiter.window()),
        new Action("Android Auto Phone", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::PHONE, actionState); }, arbiter.window()),
        new Action("Android Auto Call End", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::CALL_END, actionState); }, arbiter.window()),
        new Action("Android Auto Play", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::PLAY, actionState); }, arbiter.window()),
        new Action("Android Auto Pause", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::PAUSE, actionState); }, arbiter.window()),
        new Action("Android Auto Prev Track", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::PREV, actionState); }, arbiter.window()),
        new Action("Android Auto Next Track", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::NEXT, actionState); }, arbiter.window()),
        new Action("Android Auto Toggle Play", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::TOGGLE_PLAY, actionState); }, arbiter.window()),
        new Action("Android Auto Voice", [&arbiter, aa_handler](Action::ActionState actionState){ aa_handler->injectButtonPressHelper(aasdk::proto::enums::ButtonCode::MICROPHONE_1, actionState); }, arbiter.window()),
        new Action("Android Auto Scroll Up", [&arbiter, aa_handler](Action::ActionState actionState){ if(actionState == Action::ActionState::Activated || actionState == Action::ActionState::Triggered) aa_handler->injectButtonPress(aasdk::proto::enums::ButtonCode::SCROLL_WHEEL, openauto::projection::WheelDirection::RIGHT); }, arbiter.window()),
        new Action("Android Auto Scroll Down", [&arbiter, aa_handler](Action::ActionState actionState){ if(actionState == Action::ActionState::Activated || actionState == Action::ActionState::Triggered) aa_handler->injectButtonPress(aasdk::proto::enums::ButtonCode::SCROLL_WHEEL, openauto::projection::WheelDirection::LEFT); }, arbiter.window()),

        new Action("Toggle Dark Mode", [&arbiter](Action::ActionState actionState){ if(actionState == Action::ActionState::Triggered || actionState == Action::ActionState::Activated) arbiter.toggle_mode(); }, arbiter.window()),
    };

    for (auto page : arbiter.layout().pages()) {
        auto callback = [&arbiter, page](Action::ActionState actionState){
            if (actionState == Action::ActionState::Triggered || actionState == Action::ActionState::Activated) {
                QMetaObject::invokeMethod(&arbiter, [&arbiter, page]{
                    arbiter.set_curr_page(page);
                }, Qt::QueuedConnection);
            }
        };
        this->actions_.append(new Action(QString("Show %1 Page").arg(page->name()), callback, arbiter.window()));
    }

    {
        auto callback = [&arbiter](Action::ActionState actionState){
            if (actionState == Action::ActionState::Triggered || actionState == Action::ActionState::Activated) {
                QMetaObject::invokeMethod(&arbiter, [&arbiter]{
                    arbiter.set_curr_page(arbiter.layout().next_enabled_page(arbiter.layout().curr_page));
                }, Qt::QueuedConnection);
            }
        };
        this->actions_.append(new Action("Cycle Page", callback, arbiter.window()));
    }


    settings.beginGroup("Core");
    settings.beginGroup("Action");
    for (auto i = 0; i < this->actions_.size(); i++) {
        auto key = settings.value(QString::number(i), QString()).toString();
        if (!key.isNull())
            this->action(i)->set(key);
    }
    settings.endGroup();
    settings.endGroup();

    QFontDatabase::addApplicationFont(":/fonts/Titillium_Web/TitilliumWeb-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Montserrat/Montserrat-LightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Montserrat/Montserrat-Regular.ttf");

    if (qApp)
        qApp->setFont(arbiter.forge().font(14));

}

QString Session::Core::stylesheet(Theme::Mode mode, float scale) const
{
    QRegularExpression regex(" (-?\\d+)px");

    auto stylesheet = this->stylesheet(mode);
    auto it = regex.globalMatch(stylesheet);
    while (it.hasNext()) {
        auto match = it.next();
        if (match.hasMatch()) {
            int scaled_px = std::ceil(match.captured(1).toInt() * scale);
            stylesheet.replace(match.captured(), QString("%1px").arg(scaled_px));
        }
    }

    return stylesheet;
}

QString Session::Core::parse_stylesheet(QString path) const
{
    QFile file(path);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);

    return stream.readAll();
}

Session::Session(Arbiter &arbiter)
    : settings_()
    , theme_(settings_)
    , layout_(settings_, arbiter)
    , system_(settings_, arbiter)
    , forge_(arbiter)
    , core_(settings_, arbiter)
    , android_auto_(arbiter)
{
}

void Session::update()
{
    if (qApp) {
        qApp->setPalette(this->theme_.palette());
        qApp->setStyleSheet(this->core_.stylesheet(this->theme_.mode, this->layout_.scale));
    }
}
