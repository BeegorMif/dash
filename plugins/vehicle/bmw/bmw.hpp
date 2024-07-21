#include <QString>
#include <iostream>
#include <stdlib.h>
#include <QByteArray>
#include <boost/log/trivial.hpp>


#include "plugins/vehicle_plugin.hpp"
#include "app/widgets/climate.hpp"
#include "app/widgets/vehicle.hpp"
#include "app/arbiter.hpp"
#include "openauto/Service/InputService.hpp"


#define G37_LOG(severity) BOOST_LOG_TRIVIAL(severity) << "[F20VehiclePlugin] "


class DebugWindow : public QWidget {
    Q_OBJECT

    public:
        DebugWindow(Arbiter &arbiter, QWidget *parent = nullptr);
        QLabel* tpmsOne;
        QLabel* tpmsTwo;
        QLabel* tpmsThree;
        QLabel* tpmsFour;
        QLabel* tpmsFive;
        QLabel* tpmsSix;
        QLabel* tpmsSeven;
        QLabel* tpmsEight;
        QLabel* tpmsNine;
        QLabel* tpmsTen;
        QLabel* tpmsEleven;
        QLabel* tpmsTwelve;
        QLabel* tpmsThirteen;
        QLabel* tpmsFourteen;
        QLabel* tpmsFifteen;
        QLabel* tpmsSixteen;
        QLabel* tpmsSeventeen;
};

class bmw : public QObject, VehiclePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID VehiclePlugin_iid)
    Q_INTERFACES(VehiclePlugin)

    public:
        bool init(ICANBus* canbus) override;

    private:
        QList<QWidget *> widgets() override;

        bool duelClimate;

        void monitorHeadlightStatus(QByteArray payload);
        void updateClimateDisplay(QByteArray payload);
        void updateTemperatureDisplay(QByteArray payload);
        void engineUpdate(QByteArray payload);
        void tpmsUpdate(QByteArray payload);
        void updateSteeringButtons(QByteArray payload);
        void updateIndicators(QByteArray payload);
        void updateWipers(QByteArray payload);
        void updateExtTemp(QByteArray payload);
        void updateCruiseControl(QByteArray payload);
        void updateHandbrake(QByteArray payload);
        void updateReverse(QByteArray payload);


        Vehicle *vehicle;
        Climate *climate;
        DebugWindow *debug;
        bool engineRunning = false;
};