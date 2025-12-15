#pragma once

#include <QMap>
#include <QtWidgets>
#include "openauto/Configuration/Configuration.hpp"

#include "app/config.hpp"

#include "app/pages/page.hpp"

class Arbiter;

class SettingsPage : public QTabWidget, public Page {
    Q_OBJECT

   public:
    SettingsPage(Arbiter &arbiter, QWidget *parent = nullptr);

    void init() override;
};

class BluetoothSettingsTab : public QWidget {
    Q_OBJECT

   public:
    BluetoothSettingsTab(Arbiter &arbiter, QWidget *parent = nullptr);

   private:
    QWidget *controls_widget();
    QWidget *scanner_widget();
    QWidget *devices_widget();

    Arbiter &arbiter;
    Config *config;
    QMap<BluezQt::DevicePtr, QPushButton *> devices;
};
