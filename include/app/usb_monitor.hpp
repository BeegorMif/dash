#pragma once
#include <QThread>
#include <QString>

class UsbMonitor : public QThread {
    Q_OBJECT
public:
    explicit UsbMonitor(QObject *parent = nullptr);
    void run() override;
signals:
    void phoneConnected(const QString &device);
    void phoneDisconnected(const QString &device);
};
