#pragma once

#include <QObject>
#include <QString>
#include <QThread>

class UsbMonitor : public QThread
{
    Q_OBJECT
public:
    explicit UsbMonitor(QObject *parent = nullptr);
    ~UsbMonitor();

signals:
    void phoneConnected(const QString &device);
    void phoneDisconnected(const QString &device);

protected:
    void run() override;

private:
    void handleDeviceEvent(const QString &device, bool connected);
};
