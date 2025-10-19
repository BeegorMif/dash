#include "app/pages/shutdown_page.hpp"
#include <QFontDatabase>
#include <QPalette>
#include <QPixmap>
#include <QGuiApplication>
#include <QScreen>

ShutdownPage::ShutdownPage(QWidget *parent)
    : QWidget(parent)
    , labelStatus(new QLabel("Phone Disconnected"))
    , labelCountdown(new QLabel(""))
    , buttonCancel(new QPushButton("Cancel"))
    , timer(new QTimer(this))
    , timeLeft(0)
{
    // ---- Window setup ----
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setObjectName("ShutdownPage");
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->setStyleSheet(R"(
        QLabel {
            color: lightgray;
            font-size: 28px;
        }
        QPushButton {
            background-color: #4fa8d2;
            border: none;
            color: white;
            padding: 15px 32px;
            text-align: center;
            font-size: 28px;
            border-radius: 8px;
        }
        QPushButton:hover {
            background-color: #62b6de;
        }
        QPushButton:pressed {
            background-color: #3d97c2;
        }
    )");

    // ---- Layout ----
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QFont font("Montserrat", 18);
    labelStatus->setFont(font);
    labelCountdown->setFont(font);
    labelStatus->setAlignment(Qt::AlignCenter);
    labelCountdown->setAlignment(Qt::AlignCenter);

    buttonCancel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonCancel->setFixedHeight(100); // keep nice big touch size

    layout->addWidget(labelStatus);
    layout->addWidget(labelCountdown);

    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonCancel);
    buttonLayout->addStretch();

    // Stretch ratios: make the button take up 3/4 of the screen width
    buttonLayout->setStretch(0, 1);
    buttonLayout->setStretch(1, 3);
    buttonLayout->setStretch(2, 1);
    layout->addLayout(buttonLayout);

    connect(buttonCancel, &QPushButton::clicked, this, &ShutdownPage::cancelled);
    connect(timer, &QTimer::timeout, this, &ShutdownPage::updateCountdown);
}

void ShutdownPage::startCountdown(int seconds)
{
    timeLeft = seconds;
    labelCountdown->setText(QString("Shutting down in %1").arg(timeLeft));
    timer->start(1000);
}

void ShutdownPage::updateCountdown()
{
    if (--timeLeft <= 0) {
        timer->stop();
        emit countdownFinished();
    } else {
        labelCountdown->setText(QString("Shutting down in %1").arg(timeLeft));
    }
}
