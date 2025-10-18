#include "app/usb_monitor.hpp"
#include "DashLog.hpp"
#include <libudev.h>
// #include <QDebug>

UsbMonitor::UsbMonitor(QObject *parent) : QThread(parent) {}

void UsbMonitor::run() {
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

    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int ret = select(fd + 1, &fds, nullptr, nullptr, nullptr);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            struct udev_device *dev = udev_monitor_receive_device(mon);
            if (dev) {
                const char *action = udev_device_get_action(dev);
                const char *devnode = udev_device_get_devnode(dev);
                const char *vendor = udev_device_get_sysattr_value(dev, "idVendor");
                const char *product = udev_device_get_sysattr_value(dev, "idProduct");

                QString devStr = devnode ? QString(devnode) : QString("unknown device");
                QString vendorStr = vendor ? QString(vendor) : QString("unknown vendor");
                QString productStr = product ? QString(product) : QString("unknown product");

                // Filter for phones (common vendor IDs, optional)
                // Example: Android phones (Google/Samsung/Huawei etc.)
                bool isPhone = true; // For now, log all USB devices
                if (isPhone && action && devnode) {
                    if (QString(action) == "add") {
                        DASH_LOG(info) << "Phone connected: " << devStr.toStdString() << " Vendor: " << vendorStr.toStdString() << " Product: " << productStr.toStdString();
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
