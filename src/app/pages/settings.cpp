#include <BluezQt/Device>
#include <BluezQt/PendingCall>
#include <QLabel>
#include <QLayoutItem>
#include <QScrollArea>

#include <aasdk_proto/ButtonCodeEnum.pb.h>
#include <aasdk_proto/VideoFPSEnum.pb.h>
#include <aasdk_proto/VideoResolutionEnum.pb.h>

#include "openauto/Configuration/AudioOutputBackendType.hpp"
#include "openauto/Configuration/BluetootAdapterType.hpp"
#include "openauto/Configuration/HandednessOfTrafficType.hpp"

#include "app/config.hpp"
#include "app/session.hpp"
#include "app/window.hpp"
#include "app/pages/settings.hpp"
#include "app/widgets/selector.hpp"
#include "app/widgets/switch.hpp"
#include "gitversion.h"

SettingsPage::SettingsPage(Arbiter &arbiter, QWidget *parent)
    : QTabWidget(parent)
    , Page(arbiter, "Settings", false, this)
{
}

void SettingsPage::init()
{
    this->addTab(new BluetoothSettingsTab(this->arbiter, this), "Bluetooth");
}

BluetoothSettingsTab::BluetoothSettingsTab(Arbiter &arbiter, QWidget *parent)
    : QWidget(parent)
    , arbiter(arbiter)
{
    this->config = Config::get_instance();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 0, 6, 0);

    layout->addWidget(this->controls_widget(), 1);
    layout->addWidget(this->devices_widget(), 1);
}

QWidget *BluetoothSettingsTab::controls_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QLabel *label = new QLabel("Media Player", widget);
    layout->addStretch();
    layout->addWidget(label);

    QLabel *connected_device = new QLabel(this->arbiter.system().bluetooth.get_media_player().first, widget);
    connected_device->setIndent(16);
    connect(&this->arbiter.system().bluetooth, &Bluetooth::media_player_changed, [connected_device](QString name, BluezQt::MediaPlayerPtr){
        connected_device->setText(name);
    });
    layout->addWidget(connected_device);
    layout->addStretch();

    layout->addWidget(this->scanner_widget());

    return widget;
}

QWidget *BluetoothSettingsTab::scanner_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QPushButton *button = new QPushButton("scan", widget);
    button->setFlat(true);
    button->setCheckable(true);
    button->setEnabled(false);
    connect(button, &QPushButton::clicked, [this](bool checked){
        if (checked)
            this->arbiter.system().bluetooth.start_scan();
        else
            this->arbiter.system().bluetooth.stop_scan();
    });
    connect(&this->arbiter.system().bluetooth, &Bluetooth::init, [this, button]{
        button->setEnabled(this->arbiter.system().bluetooth.has_adapter());
    });
    layout->addWidget(button);

    ProgressIndicator *loader = new ProgressIndicator(widget);
    loader->scale(this->arbiter.layout().scale);
    connect(&this->arbiter.system().bluetooth, &Bluetooth::scan_status, [button, loader](bool status){
        if (status)
            loader->start_animation();
        else
            loader->stop_animation();
        button->setChecked(status);
    });
    layout->addWidget(loader);

    return widget;
}

QWidget *BluetoothSettingsTab::devices_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    for (BluezQt::DevicePtr device : this->arbiter.system().bluetooth.get_devices()) {
        QPushButton *button = new QPushButton(device->name(), widget);
        button->setCheckable(true);
        if (device->isConnected())
            button->setChecked(true);
        connect(button, &QPushButton::clicked, [this, button, device](bool checked = false){
            button->setChecked(!checked);
            this->arbiter.system().bluetooth.toggle_device(device);
        });

        this->devices[device] = button;
        layout->addWidget(button);
    }
    connect(&this->arbiter.system().bluetooth, &Bluetooth::device_added, [this, layout, widget](BluezQt::DevicePtr device){
        QPushButton *button = new QPushButton(device->name(), widget);
        button->setCheckable(true);
        if (device->isConnected())
            button->setChecked(true);
        connect(button, &QPushButton::clicked, [this, button, device](bool checked = false){
            button->setChecked(!checked);
            this->arbiter.system().bluetooth.toggle_device(device);
        });
        this->devices[device] = button;
        layout->addWidget(button);
    });
    connect(&this->arbiter.system().bluetooth, &Bluetooth::device_changed, [this](BluezQt::DevicePtr device){
        if(!this->devices.contains(device)) {
            emit this->arbiter.system().bluetooth.device_added(device);
        }
        else {
            this->devices[device]->setText(device->name());
            this->devices[device]->setChecked(device->isConnected());
        }
    });
    connect(&this->arbiter.system().bluetooth, &Bluetooth::device_removed, [this, layout](BluezQt::DevicePtr device){
        layout->removeWidget(devices[device]);
        delete this->devices[device];
        this->devices.remove(device);
    });

    QScrollArea *scroll_area = new QScrollArea(this);
    Session::Forge::to_touch_scroller(scroll_area);
    scroll_area->setWidgetResizable(true);
    scroll_area->setWidget(widget);

    return scroll_area;
}

