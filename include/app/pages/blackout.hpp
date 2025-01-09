#pragma once

#include <QtWidgets>

#include "app/config.hpp"
#include "app/pages/page.hpp"

class Arbiter;
class BlackoutPage;

class BlackoutPage : public QStackedWidget, public Page {
    Q_OBJECT

   public:
    BlackoutPage(Arbiter &arbiter, QWidget *parent = nullptr);

    void init() override;

    private:
        QWidget *load_msg();
        void showEvent(QShowEvent *event);
        void screenOn();
};