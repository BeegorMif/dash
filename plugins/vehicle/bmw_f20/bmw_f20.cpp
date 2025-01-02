#include "bmw_f20.hpp"

bool BMWF20::init(ICANBus* canbus){
    if (this->arbiter) {
        this->debug = new DebugWindow(*this->arbiter);

        canbus->registerFrameHandler(0x21A, [this](QByteArray payload){
            this->monitorHeadlightStatus(payload);
            this->headlightUpdate(payload);
            });


        DASH_LOG(info)<<"loaded successfully";
        return true;
    }
    else{
        DASH_LOG(error)<<"Failed to get arbiter";
        return false;
    }

}

QList<QWidget *> BMWF20::widgets()
{
    QList<QWidget *> tabs;
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

void BMWF20::headlightUpdate(QByteArray payload){
    this->debug->headlightState->setText(QString::number((uint8_t)payload.at(0)));
}

// HEADLIGHTS AND DOORS
// 21A
// FIRST BYTE:
// |unknown|unknown|unknown|unknown|unknown|unknown|unknown|MAIN BEAM|
// SECOND BYTE:
// |unknown|unknown|AUTO|unknown|unknown|unknown|unknown|unknown|
// OTHERS UNKNOWN

void BMWF20::monitorHeadlightStatus(QByteArray payload){
    if((payload.at(0)>>0) & 1){
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

DebugWindow::DebugWindow(Arbiter &arbiter, QWidget *parent) : QWidget(parent)
{
    this->setObjectName("Headlight Debug");


    QLabel* textOne = new QLabel("Payload Lights", this);

    headlightState = new QLabel("--", this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(textOne);
    layout->addWidget(headlightState);
    layout->addWidget(Session::Forge::br(false));

}