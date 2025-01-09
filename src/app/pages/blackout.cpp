#include "app/arbiter.hpp"
#include "app/pages/blackout.hpp"
#include <QScreen>
#include <QProcess>

BlackoutPage::BlackoutPage(Arbiter &arbiter, QWidget *parent)
    : QStackedWidget(parent)
    , Page(arbiter, "Blackout", "screen_off", true, this)
{
}

void BlackoutPage::init()
{
    this->addWidget(this->load_msg());
}

QWidget *BlackoutPage::load_msg()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel("Load Launcher Plugin", widget);
    label->setAlignment(Qt::AlignCenter);

    QPushButton *screen_on = new QPushButton(widget);
    connect(screen_on, &QPushButton::clicked, [this]() {
        DASH_LOG(debug) << "BLACKOUT BUTTON PRESSED";
        this->screenOn();
    });
    QSize size = this->size();
    screen_on->setFixedSize((size*2));
    layout->addStretch();
    layout->addWidget(label);
    layout->addStretch();

    return widget;
}

void BlackoutPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QProcess process(this);
    process.startDetached(QString("sudo ddcutil setvcp D6 04"));
    process.waitForFinished();
}
void BlackoutPage::screenOn()
{
    this->arbiter.set_curr_page(0);
    QProcess process(this);
    process.startDetached(QString("sudo ddcutil setvcp D6 01"));
    process.waitForFinished();
}