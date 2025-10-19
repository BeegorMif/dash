#include "app/pages/shutdown_page.hpp"
#include "DashLog.hpp"

ShutdownPage::ShutdownPage(QWidget *parent)
    : QWidget(parent), remaining(0)
{
    setStyleSheet("background-color: black; color: white;");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 48px;");

    cancelButton = new QPushButton("Cancel", this);
    cancelButton->setStyleSheet("font-size: 24px; padding: 10px;");
    connect(cancelButton, &QPushButton::clicked, this, &ShutdownPage::cancel);

    layout->addWidget(label);
    layout->addWidget(cancelButton);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ShutdownPage::updateCountdown);
}

void ShutdownPage::startCountdown(int seconds)
{
    remaining = seconds;
    label->setText(QString("Shutting down in %1 seconds...").arg(remaining));
    timer->start(1000);
    DASH_LOG(info) << "Shutdown countdown started";
}

void ShutdownPage::updateCountdown()
{
    remaining--;
    label->setText(QString("Shutting down in %1 seconds...").arg(remaining));

    if (remaining <= 0) {
        timer->stop();
        emit countdownFinished();
    }
}

void ShutdownPage::cancel()
{
    DASH_LOG(info) << "Shutdown countdown cancelled";
    timer->stop();
    emit cancelled();
}
