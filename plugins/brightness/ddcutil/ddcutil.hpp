#pragma once

#include <QObject>
#include "plugins/brightness_plugin.hpp"

class Ddcutil : public QObject, BrightnessPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID BrightnessPlugin_iid FILE "ddcutil.json")
    Q_INTERFACES(BrightnessPlugin)

   public:
    Ddcutil();
    bool supported() override;
    uint8_t priority() override;
    void set(int brightness) override;

   private:
    QScreen *screen;
};
