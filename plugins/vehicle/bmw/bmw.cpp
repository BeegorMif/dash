#include "bmw.hpp"



bool bmw::init(ICANBus* canbus){
    this->duelClimate=false;
    if (this->arbiter) {
        this->climate = new Climate(*this->arbiter);
        this->climate->max_fan_speed(7);
        this->climate->setObjectName("Climate");

        this->debug = new DebugWindow(*this->arbiter);

        canbus->registerFrameHandler(0x60D, [this](QByteArray payload){this->monitorHeadlightStatus(payload);});
        canbus->registerFrameHandler(0x54B, [this](QByteArray payload){this->updateClimateDisplay(payload);});
        canbus->registerFrameHandler(0x542, [this](QByteArray payload){this->updateTemperatureDisplay(payload);});
        canbus->registerFrameHandler(0x551, [this](QByteArray payload){this->engineUpdate(payload);});
        canbus->registerFrameHandler(0x1A0, [this](QByteArray payload){this->tpmsUpdate(payload);});
        canbus->registerFrameHandler(0x1D6, [this](QByteArray payload){this->updateSteeringButtons(payload);});
        canbus->registerFrameHandler(0x1F6, [this](QByteArray payload){this->updateIndicators(payload);});
        canbus->registerFrameHandler(0x200, [this](QByteArray payload){this->updateCruiseControl(payload);});
        canbus->registerFrameHandler(0x252, [this](QByteArray payload){this->updateWipers(payload);});
        canbus->registerFrameHandler(0x200, [this](QByteArray payload){this->updateCruiseControl(payload);});
        canbus->registerFrameHandler(0x2CA, [this](QByteArray payload){this->updateExtTemp(payload);});
        canbus->registerFrameHandler(0x34F, [this](QByteArray payload){this->updateHandbrake(payload);});
        canbus->registerFrameHandler(0x3B0, [this](QByteArray payload){this->updateReverse(payload);});
        

        G37_LOG(info)<<"loaded successfully";
        return true;
    }
    else{
        G37_LOG(error)<<"Failed to get arbiter";
        return false;
    }
    

}

QList<QWidget *> bmw::widgets()
{
    QList<QWidget *> tabs;
    tabs.append(this->climate);
    tabs.append(this->debug);
    return tabs;
}


// TPMS
// 385
// THIRD BYTE:
//  Tire Pressure (PSI) * 4
// FOURTH BYTE:
//  Tire Pressure (PSI) * 4
// FIFTH BYTE:
//  Tire Pressure (PSI) * 4
// SIXTH BYTE:
//  Tire Pressure (PSI) * 4
// SEVENTH BYTE:
// |Tire 1 Pressure Valid|Tire 2 Pressure Valid|Tire 3 Pressure Valid|Tire 4 Pressure Valid|unknown|unknown|unknown|unknown

// OTHERS UNKNOWN

void bmw::tpmsUpdate(QByteArray payload){
    this->debug->tpmsOne->setText(QString::number((uint8_t)payload.at(0)*0.103));
    // this->debug->tpmsTwo->setText(QString::number((uint8_t)payload.at(3)/4));
    // this->debug->tpmsThree->setText(QString::number((uint8_t)payload.at(4)/4));
    // this->debug->tpmsFour->setText(QString::number((uint8_t)payload.at(5)/4));
}
void bmw::updateSteeringButtons(QByteArray payload){
    this->debug->tpmsTwo->setText(QString::number((uint8_t)payload.at(0)>>3));
    this->debug->tpmsThree->setText(QString::number((uint8_t)payload.at(0)>>2));
    // this->debug->tpmsFour->setText(QString::number((uint8_t)payload.at(0)));
    // this->debug->tpmsFive->setText(QString::number((uint8_t)payload.at(0)));
    // this->debug->tpmsSix->setText(QString::number((uint8_t)payload.at(0)>>5));
    // this->debug->tpmsSeven->setText(QString::number((uint8_t)payload.at(0)>>6));
}
void bmw::updateIndicators(QByteArray payload){
    this->debug->tpmsEight->setText(QString::number((uint8_t)payload.at(0)>>4));
    this->debug->tpmsNine->setText(QString::number((uint8_t)payload.at(0)>>5));
    this->debug->tpmsTen->setText(QString::number((uint8_t)payload.at(0)));
    bool leftInd = payload.at(0)>>4 & 1;
    bool rightInd = payload.at(0)>>5 & 1;
    bool holdInd = payload.at(0) & 1;
    if (leftInd)
        vehicle->indicators(LEFT, 1);
    else
        vehicle->indicators(LEFT, 0);
    if (rightInd)
        vehicle->indicators(RIGHT, 1);
    else
        vehicle->indicators(RIGHT, 0);
}
void bmw::updateCruiseControl(QByteArray payload){
    this->debug->tpmsEleven->setText(QString::number((uint8_t)payload.at(1)>>4));
    this->debug->tpmsTwelve->setText(QString::number((uint8_t)payload.at(1)>>5));
    this->debug->tpmsThirteen->setText(QString::number((uint8_t)payload.at(0)));
}
void bmw::updateWipers(QByteArray payload){
    this->debug->tpmsFourteen->setText(QString::number((uint8_t)payload.at(0)));
}
void bmw::updateExtTemp(QByteArray payload){
    this->debug->tpmsFifteen->setText(QString::number(((uint8_t)payload.at(0)-48)*0.555536));
}
void bmw::updateHandbrake(QByteArray payload){
    this->debug->tpmsSixteen->setText(QString::number((uint8_t)payload.at(0)));
}
void bmw::updateReverse(QByteArray payload){
    this->debug->tpmsSeventeen->setText(QString::number((uint8_t)payload.at(0)));
}
//551
//(A, B, C, D, E, F, G, H)
// D - Bitfield.

//     0xA0 / 160 - Engine off, "On"
//     0x20 / 32 - Engine turning on (500ms)
//     0x00 / 0 - Engine turning on (2500ms)
//     0x80 / 128 - Engine running
//     0x00 / 0 - Engine shutting down (2500ms)
//     0x20 / 32 - Engine off


// HEADLIGHTS AND DOORS
// 60D
// FIRST BYTE:
// |unknown|RR_DOOR|RL_DOOR|FR_DOOR|FL_DOOR|SIDE_LIGHTS|HEADLIGHTS|unknown|
// SECOND BYTE:
// |unknown|unknown|unknown|unknown|unknown|unknown|unknown|FOGLIGHTS|
// OTHERS UNKNOWN

void bmw::monitorHeadlightStatus(QByteArray payload){
    if((payload.at(0)>>1) & 1){
        //headlights are ON - turn to dark mode
        if(this->arbiter->theme().mode == Session::Theme::Light){
            this->arbiter->set_mode(Session::Theme::Dark);
        }
    }
    else{
        //headlights are off or not fully on (i.e. sidelights only) - make sure is light mode
        if(this->arbiter->theme().mode == Session::Theme::Dark){
            this->arbiter->set_mode(Session::Theme::Light);
        }
    }
}

// HVAC
// 54B

// FIRST BYTE 
// |unknown|unknown|unknown|unknown|unknown|unknown|unknown|HVAC_OFF|
// SECOND BYTE
// |unknown|unknown|unknown|unknown|unknown|unknown|unknown|unknown|
// THIRD BYTE - MODE
// |unknown|unknown|MODE|MODE|MODE|unknown|unknown|unknown|
//   mode:
//    defrost+leg
//       1 0 0
//    head
//       0 0 1
//    head+feet
//       0 1 0
//    feet
//       0 1 1
//    defrost
//       1 0 1
// FOURTH BYTE
// |unknown|DUEL_CLIMATE_ON|DUEL_CLIMATE_ON|unknown|unknown|unknown|RECIRCULATE_OFF|RECIRCULATE_ON|
// Note both duel climate on bytes toggle to 1 when duel climate is on
// FIFTH BYTE - FAN LEVEL
// |unknown|unknown|FAN_1|FAN_2|FAN_3|unknown|unknown|unknown|
// FAN_1, FAN_2, FAN_3 scale linearly fan 0 (off) -> 7
//
// ALL OTHERS UNKNOWN

bool oldStatus = true;

void bmw::updateClimateDisplay(QByteArray payload){
    duelClimate = (payload.at(3)>>5) & 1;
    bool hvacOff = payload.at(0) & 1;
    if(hvacOff != oldStatus){
        oldStatus = hvacOff;
        if(hvacOff){
            climate->airflow(Airflow::OFF);
            climate->fan_speed(0);
            G37_LOG(info)<<"Climate is off";
            return;
        }
    }
    uint8_t airflow = (payload.at(2) >> 3) & 0b111;
    uint8_t dash_airflow = 0;
    switch(airflow){
        case(1):
            dash_airflow = Airflow::BODY;
            break;
        case(2):
            dash_airflow = Airflow::BODY | Airflow::FEET;
            break;
        case(3):
            dash_airflow = Airflow::FEET;
            break;
        case(4):
            dash_airflow = Airflow::DEFROST | Airflow::FEET;
            break;
        case(5):
            dash_airflow = Airflow::DEFROST;
            break;
    }
    if(climate->airflow()!=dash_airflow)
        climate->airflow(dash_airflow);
    uint8_t fanLevel = (payload.at(4)>>3) & 0b111;
    if(climate->fan_speed()!=fanLevel)
        climate->fan_speed(fanLevel);
}

// Climate
// 542

// FIRST BYTE:
// unknown
// SECOND BYTE:
// entire byte is driver side temperature, 60F->90F
// THIRD BYTE:
// entire byte is passenger side temperature, 60F->90F
// note that this byte is only updated when duel climate is on. When duel climate is off, SECOND BYTE contains accurate passenger temperature.

void bmw::updateTemperatureDisplay(QByteArray payload){
    if(climate->right_temp()!=(unsigned char)payload.at(1))
        climate->right_temp((unsigned char)payload.at(1));
    if(duelClimate){
        if(climate->left_temp()!=(unsigned char)payload.at(2)){
            climate->left_temp((unsigned char)payload.at(2));
        }
    }else{
        if(climate->left_temp()!=(unsigned char)payload.at(1))
            climate->left_temp((unsigned char)payload.at(1));
    }
}




DebugWindow::DebugWindow(Arbiter &arbiter, QWidget *parent) : QWidget(parent)
{
    this->setObjectName("Debug");


    QLabel* textOne = new QLabel("Vehicle Speed", this);
    QLabel* textTwo = new QLabel("Steering Wheel - Vol +", this);
    QLabel* textThree = new QLabel("Steering Wheel - Vol -", this);
    // QLabel* textFour = new QLabel("Steering Wheel - Voice", this);
    // QLabel* textFive = new QLabel("Steering Wheel - Telphone", this);
    // QLabel* textSix = new QLabel("Steering Wheel - Media Up", this);
    // QLabel* textSeven = new QLabel("Steering Wheel - Media Down", this);
    QLabel* textEight = new QLabel("Indicator Left", this);
    QLabel* textNine = new QLabel("Indicator Right", this);
    QLabel* textTen = new QLabel("Indicator Hold", this);
    QLabel* textEleven = new QLabel("Cruise Control Inactive", this);
    QLabel* textTwelve = new QLabel("Cruise Control Active", this);
    QLabel* textThirteen = new QLabel("Cruise Control Set Speed", this);
    QLabel* textFourteen = new QLabel("Automatic Wipers", this);
    QLabel* textFifteen = new QLabel("External Temperature", this);
    QLabel* textSixteen = new QLabel("Handbrake", this);
    QLabel* textSeventeen = new QLabel("Reverse", this);

    tpmsOne = new QLabel("--", this);
    tpmsTwo = new QLabel("--", this);
    tpmsThree = new QLabel("--", this);
    // tpmsFour = new QLabel("--", this);
    // tpmsFive = new QLabel("--", this);
    // tpmsSix = new QLabel("--", this);
    // tpmsSeven = new QLabel("--", this);
    tpmsEight = new QLabel("--", this);
    tpmsNine = new QLabel("--", this);
    tpmsTen = new QLabel("--", this);
    tpmsEleven = new QLabel("--", this);
    tpmsTwelve = new QLabel("--", this);
    tpmsThirteen = new QLabel("--", this);
    tpmsFourteen = new QLabel("--", this);
    tpmsFifteen = new QLabel("--", this);
    tpmsSixteen = new QLabel("--", this);
    tpmsSeventeen = new QLabel("--", this);




    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(textOne);
    layout->addWidget(tpmsOne);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textTwo);
    layout->addWidget(tpmsTwo);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textThree);
    layout->addWidget(tpmsThree);
    layout->addWidget(Session::Forge::br(false));

    // layout->addWidget(textFour); 
    // layout->addWidget(tpmsFour);
    // layout->addWidget(Session::Forge::br(false));

    // layout->addWidget(textFive); 
    // layout->addWidget(tpmsFive);
    // layout->addWidget(Session::Forge::br(false));

    // layout->addWidget(textSix); 
    // layout->addWidget(tpmsSix);
    // layout->addWidget(Session::Forge::br(false));

    // layout->addWidget(textSeven); 
    // layout->addWidget(tpmsSeven);
    // layout->addWidget(Session::Forge::br(false));


    layout->addWidget(textEight); 
    layout->addWidget(tpmsEight);
    layout->addWidget(Session::Forge::br(false));


    layout->addWidget(textNine); 
    layout->addWidget(tpmsNine);
    layout->addWidget(Session::Forge::br(false));


    layout->addWidget(textTen); 
    layout->addWidget(tpmsTen);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textEleven); 
    layout->addWidget(tpmsEleven);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textTwelve); 
    layout->addWidget(tpmsTwelve);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textThirteen); 
    layout->addWidget(tpmsThirteen);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textFourteen); 
    layout->addWidget(tpmsFourteen);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textFifteen); 
    layout->addWidget(tpmsFifteen);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textSixteen); 
    layout->addWidget(tpmsSixteen);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(textSeventeen); 
    layout->addWidget(tpmsSeventeen);
    layout->addWidget(Session::Forge::br(false));


}