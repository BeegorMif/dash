#include <QDebug>
#include <QTimer>

#include "bmw_f20.hpp"

Test::~Test()
{
    if (this->climate)
        delete this->climate;
    if (this->vehicle)
        delete this->vehicle;
    if (this->debug)
        delete this->debug;
}

QList<QWidget *> Test::widgets()
{
    QList<QWidget *> tabs;
    if (this->vehicle)
        tabs.append(this->vehicle);
    if (this->debug)
        tabs.append(this->debug);
    return tabs;
}

bool Test::init(ICANBus* canbus)
{
    if (this->arbiter) {
        this->debug = new DebugWindow(*this->arbiter);
        this->vehicle = new Vehicle(*this->arbiter);
        this->vehicle->pressure_init("psi", 35);
        this->vehicle->disable_sensors();
        this->vehicle->rotate(270);

        this->climate = new Climate(*this->arbiter);
        this->climate->max_fan_speed(4);

        canbus->registerFrameHandler(0x21A, [this](QByteArray payload){this->headlightUpdate(payload);});
        canbus->registerFrameHandler(0x3B0, [this](QByteArray payload){this->reverseUpdate(payload);});

        auto timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [this]{
            static bool toggle = false;

            switch(rand() % 50) {
                case 0:
                    this->climate->airflow(Airflow::OFF);
                    break;
                case 1:
                    this->climate->airflow(Airflow::DEFROST);
                    break;
                case 2:
                    this->climate->airflow(Airflow::BODY);
                    break;
                case 3:
                    this->climate->airflow(Airflow::FEET);
                    break;
                case 4:
                    this->climate->airflow(Airflow::DEFROST | Airflow::BODY);
                    break;
                case 5:
                    this->climate->airflow(Airflow::DEFROST | Airflow::FEET);
                    break;
                case 6:
                    this->climate->airflow(Airflow::BODY | Airflow::FEET);
                    break;
                case 7:
                    this->climate->airflow(Airflow::DEFROST | Airflow::BODY | Airflow::FEET);
                    break;
                case 8:
                    this->climate->fan_speed((rand() % 4) + 1);
                    break;
                case 9:
                    this->vehicle->sensor(Position::FRONT_LEFT, rand() % 5);
                    break;
                case 10:
                    this->vehicle->sensor(Position::FRONT_MIDDLE_LEFT, rand() % 5);
                    break;
                case 11:
                    this->vehicle->sensor(Position::FRONT_MIDDLE_RIGHT, rand() % 5);
                    break;
                case 12:
                    this->vehicle->sensor(Position::FRONT_RIGHT, rand() % 5);
                    break;
                case 13:
                    this->vehicle->sensor(Position::BACK_LEFT, rand() % 5);
                    break;
                case 14:
                    this->vehicle->sensor(Position::BACK_MIDDLE_LEFT, rand() % 5);
                    break;
                case 15:
                    this->vehicle->sensor(Position::BACK_MIDDLE_RIGHT, rand() % 5);
                    break;
                case 16:
                    this->vehicle->sensor(Position::BACK_RIGHT, rand() % 5);
                    break;
                case 17:
                    this->vehicle->door(Position::FRONT_LEFT, toggle);
                    break;
                case 18:
                    this->vehicle->door(Position::BACK_LEFT, toggle);
                    break;
                case 19:
                    this->vehicle->door(Position::FRONT_RIGHT, toggle);
                    break;
                case 20:
                    this->vehicle->door(Position::BACK_RIGHT, toggle);
                    break;
                case 21:
                    this->vehicle->headlights(toggle);
                    break;
                case 22:
                    this->vehicle->taillights(toggle);
                    break;
                case 23:
                    this->vehicle->pressure(Position::BACK_RIGHT, (rand() % 21) + 30);
                    break;
                case 24:
                    this->vehicle->pressure(Position::BACK_LEFT, (rand() % 21) + 30);
                    break;
                case 25:
                    this->vehicle->pressure(Position::FRONT_RIGHT, (rand() % 21) + 30);
                    break;
                case 26:
                    this->vehicle->pressure(Position::FRONT_LEFT, (rand() % 21) + 30);
                    break;
                case 27:
                    this->vehicle->wheel_steer((rand() % 10) * ((rand() % 2) ? 1 : -1));
                    break;
                case 28:
                    this->vehicle->indicators(Position::LEFT, toggle);
                    break;
                case 29:
                    this->vehicle->indicators(Position::RIGHT, toggle);
                    break;
                case 30:
                    this->vehicle->hazards(toggle);
                    break;
                default:
                    toggle = !toggle;
                    break;
            }
        });
        // timer->start(1000);

        auto timer2 = new QTimer(this);
        connect(timer2, &QTimer::timeout, [this]{
            if (rand() % 10 == 1) {
                this->climate->left_temp((rand() % 20) + 60);
                this->climate->right_temp((rand() % 20) + 60);
            }
        });
        // timer2->start(1000);

        return true;
    }

    return false;
}

void Test::headlightUpdate(QByteArray payload)
{
    this->debug->lightState->setText(QString::number((uint8_t)payload.at(0)));
    if((payload.at(0)>>0) & 1){
        this->vehicle->headlights(true);
        //headlights are ON - turn to dark mode
        if(this->arbiter->theme().mode == Session::Theme::Light){
            this->arbiter->set_mode(Session::Theme::Dark);
        }
    }
    else{
        this->vehicle->headlights(false);
        //headlights are off or not fully on (i.e. sidelights only) - make sure is light mode
        if(this->arbiter->theme().mode == Session::Theme::Dark){
            this->arbiter->set_mode(Session::Theme::Light);
        }
    }
}

void Test::reverseUpdate(QByteArray payload)
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
    this->setObjectName("Debug");


    QLabel* lights = new QLabel("Light Status", this);
    QLabel* reverse = new QLabel("Reverse Gear", this);

    lightState = new QLabel("--", this);
    reverseState = new QLabel("--", this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(lights);
    layout->addWidget(lightState);
    layout->addWidget(Session::Forge::br(false));

    layout->addWidget(reverse); 
    layout->addWidget(reverseState);
    layout->addWidget(Session::Forge::br(false));
}