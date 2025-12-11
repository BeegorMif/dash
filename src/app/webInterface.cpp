#include "app/webInterface.hpp"
#include <QDebug>

WebInterface::WebInterface(QObject *parent)
    : QObject(parent)
{
}

void WebInterface::tabChanged(const QString &tab)
{
    emit tabChangedSignal(tab);
    qDebug() << "[Dash] WebChannel received tabChanged:" << tab;

}
