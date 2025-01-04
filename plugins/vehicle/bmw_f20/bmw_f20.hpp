#pragma once

#include <QObject>
#include "plugins/vehicle_plugin.hpp"
#include "app/widgets/climate.hpp"
#include "app/widgets/vehicle.hpp"
#include "app/arbiter.hpp"
#include "canbus/socketcanbus.hpp"

class DebugWindow : public QWidget {
    Q_OBJECT

    public:
        DebugWindow(Arbiter &arbiter, QWidget *parent = nullptr);
        QLabel * lightState;
        QLabel * lightState_readable;
        QLabel * reverseState;
        QLabel * reverseState_readable;

};
class Test : public QObject, VehiclePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID VehiclePlugin_iid FILE "f20.json")
    Q_INTERFACES(VehiclePlugin)

   public:
    Test() {};
    ~Test();
    QList<QWidget *> widgets() override;
    bool init(ICANBus*) override;
    bool previous_value = false;

   private:
    Climate *climate;
    Vehicle *vehicle;
    DebugWindow *debug;
    void headlightUpdate(QByteArray payload);
    void reverseUpdate(QByteArray Payload);
};