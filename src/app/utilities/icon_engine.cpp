#include <QSvgRenderer>

#include "app/arbiter.hpp"

#include "app/utilities/icon_engine.hpp"


IconEngine::IconEngine(Arbiter &arbiter, QString icon, bool colorize)
    : QIconEngine()
    , arbiter(arbiter)
    , icon(icon)
    , alt_icon()
    , colorize(colorize)
{
}

void IconEngine::init_painter(QPainter *painter, QIcon::Mode mode, QIcon::State state) const
{

}

void IconEngine::addFile(const QString &file, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    if (mode == QIcon::Normal)
        this->alt_icon = file;
}


void IconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    QSvgRenderer r(((state == QIcon::On) && !this->alt_icon.isNull()) ? this->alt_icon : this->icon);

    QPixmap output(rect.size());

    r.render(painter);
    painter->setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter->setPen(Qt::NoPen);
    this->init_painter(painter, mode, state);
    painter->drawRect(output.rect());
}

QIconEngine *IconEngine::clone() const
{
    return new IconEngine(*this);
}

QPixmap IconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pm(size);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    QRect r(QPoint(), size);
    this->paint(&painter, r, mode, state);
    return pm;
}

StylizedIconEngine::StylizedIconEngine(Arbiter &arbiter, QString icon, bool colorize)
    : IconEngine(arbiter, icon, colorize)
    , stylized(false)
{
}

void StylizedIconEngine::init_painter(QPainter *painter, QIcon::Mode mode, QIcon::State state) const
{
    if (!this->stylized)
        IconEngine::init_painter(painter, mode, state);
}

void StylizedIconEngine::addFile(const QString &file, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    if (mode == QIcon::Active) {
        this->icon = file;
        this->stylized = (state == QIcon::On);
    }
}

QIconEngine *StylizedIconEngine::clone() const
{
    return new StylizedIconEngine(*this);
}
