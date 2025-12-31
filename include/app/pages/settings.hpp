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
