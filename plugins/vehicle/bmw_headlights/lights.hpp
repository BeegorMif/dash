#include <QString>
#include <iostream>
#include <stdlib.h>
#include <QByteArray>
#include <boost/log/trivial.hpp>


#include "plugins/vehicle_plugin.hpp"
#include "app/arbiter.hpp"
#include "openauto/Service/InputService.hpp"


#define F20_LOG(severity) BOOST_LOG_TRIVIAL(severity) << "[F20LightsVehiclePlugin] "


class DebugWindow : public QWidget {
    Q_OBJECT

    public:
        DebugWindow(Arbiter &arbiter, QWidget *parent = nullptr);
        QLabel* headlightStatus;
        QLabel * headlightState;
};

class BMWF20 : public QObject, VehiclePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID VehiclePlugin_iid)
    Q_INTERFACES(VehiclePlugin)

    public:
        bool init(ICANBus* canbus) override;

    private:
        QList<QWidget *> widgets() override;

        void monitorHeadlightStatus(QByteArray payload);
        void headlightUpdate(QByteArray payload);

        DebugWindow *debug;
};