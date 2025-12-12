#pragma once

#include <QObject>
#include <QString>

class WebInterface : public QObject {
    Q_OBJECT

public:
    explicit WebInterface(QObject *parent = nullptr);

    Q_INVOKABLE void tabChanged(const QString &tab);
    Q_INVOKABLE void darkModeChanged(bool state);

signals:
    void tabChangedSignal(const QString &tab);
    void darkModeChangedSignal(bool state);
};
