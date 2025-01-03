#include "bmw_f20.hpp"

bool BMWF20::init(ICANBus* canbus){
    if (this->arbiter) {
        this->debug = new DebugWindow(*this->arbiter);

        canbus->registerFrameHandler(0x21A, [this](QByteArray payload){this->headlightUpdate(payload);});
        canbus->registerFrameHandler(0x3B0, [this](QByteArray payload){this->reverseUpdate(payload);});


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

// HEADLIGHTS AND DOORS
// 21A
// FIRST BYTE:
// |unknown|unknown|unknown|unknown|unknown|unknown|unknown|MAIN BEAM|
// SECOND BYTE:
// |unknown|unknown|AUTO|unknown|unknown|unknown|unknown|unknown|
// OTHERS UNKNOWN

void BMWF20::headlightUpdate(QByteArray payload){
    this->debug->headlightState->setText(QString::number((uint8_t)payload.at(0)));
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

void BMWF20::reverseUpdate(QByteArray payload)
{
    this->debug->reverseState->setText(QString::number((uint8_t)payload.at(0)));
    if((payload.at(0)>>0) & 1){
        DASH_LOG(debug) << "[FUNCTION] Reverse gear selected.";
        //reverse gear selected, switch to camera page
        this->arbiter->set_curr_page(3);
    }
    else{
        DASH_LOG(debug) << "[FUNCTION] Reverse gear deselected.";
        //not in reverse, switch to Android Auto
        this->arbiter->set_curr_page(0);
        }
}

DebugWindow::DebugWindow(Arbiter &arbiter, QWidget *parent) : QWidget(parent)
{
    this->setObjectName("F20 Debug");


    QLabel* lights = new QLabel("Light Status", this);
    QLabel* reverse = new QLabel("Reverse Gear", this);

    headlightState = new QLabel("--", this);
    reverseState = new QLabel("--", this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(lights);
    layout->addWidget(headlightState);
    layout->addWidget(Session::Forge::br(false));
    layout->addWidget(reverse);
    layout->addWidget(reverseState);
    layout->addWidget(Session::Forge::br(false));

}