#include <QGuiApplication>
#include <QProcess>
#include <QScreen>

#include "ddcutil.hpp"

Ddcutil::Ddcutil()
{
    this->screen = QGuiApplication::primaryScreen();
}

bool Ddcutil::supported()
{
    if (this->screen != nullptr) {
        // Check that we can execute ddcutil
        QProcess process(this);
        process.start(QString("ddcutil --version"));
        process.waitForFinished();
        return process.exitCode() == 0;
    }
    return false;
}

uint8_t Ddcutil::priority()
{
    return 4;
}

void Ddcutil::set(int brightness)
{
    if (this->screen != nullptr) {
        QProcess process(this);
        process.start(QString("ddcutil --output %1 setvcp 10 %2").arg(this->screen->name()).arg(brightness / 2.55));
        process.waitForFinished();
    }
}
