#include "app/usb_monitor.hpp"
#include "DashLog.hpp"
#include <libudev.h>

UsbMonitor::UsbMonitor(QObject *parent)
    : QThread(parent)
{
    start(); // Launch the thread
}

UsbMonitor::~UsbMonitor()
{
    requestInterruption();
    wait();
}

void UsbMonitor::run()
{
    struct udev *udev = udev_new();
    if (!udev) {
        DASH_LOG(error) << "UsbMonitor: Failed to create udev context";
        return;
    }

    struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");
    if (!mon) {
        DASH_LOG(error) << "UsbMonitor: Failed to create udev monitor";
        udev_unref(udev);
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", nullptr);
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);
    DASH_LOG(info) << "USB monitor thread started";

    while (!isInterruptionRequested()) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int ret = select(fd + 1, &fds, nullptr, nullptr, nullptr);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            struct udev_device *dev = udev_monitor_receive_device(mon);
            if (dev) {
                const char *action = udev_device_get_action(dev);
                const char *devnode = udev_device_get_devnode(dev);

                QString devStr = devnode ? QString(devnode) : QString("unknown device");

                if (action && devnode) {
                    if (QString(action) == "add") {
                        DASH_LOG(info) << "Phone connected: " << devStr.toStdString();
                        emit phoneConnected(devStr);
                    } else if (QString(action) == "remove") {
                        DASH_LOG(info) << "Phone disconnected: " << devStr.toStdString();
                        emit phoneDisconnected(devStr);
                    }
                }

                udev_device_unref(dev);
            }
        }
    }

    udev_monitor_unref(mon);
    udev_unref(udev);
}

void UsbMonitor::handleDeviceEvent(const QString &device, bool connected)
{
    if (connected)
        emit phoneConnected(device);
    else
        emit phoneDisconnected(device);
}
