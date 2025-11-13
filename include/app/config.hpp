#pragma once

#include "openauto/Configuration/Configuration.hpp"

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVideoFrame>
#include <QWidget>

/*
    NOTE: to only be used for page-specific config

    this is temporary, will soon be migrated to new cool ways
*/

class Config : public QObject {
    Q_OBJECT

   public:
    std::shared_ptr<openauto::configuration::Configuration> openauto_config;
    openauto::configuration::Configuration::ButtonCodes openauto_button_codes;

    Config();

    inline QString get_media_home() { return this->media_home; }
    inline void set_media_home(QString media_home)
    {
        this->media_home = media_home;
        this->settings.setValue("Pages/Media/Local/home", this->media_home);
    }

    inline bool get_si_units() { return this->si_units; }
    inline void set_si_units(bool si_units)
    {
        this->si_units = si_units;
        this->settings.setValue("Pages/Vehicle/si_units", this->si_units);
        emit si_units_changed(this->si_units);
    }

    inline bool get_show_aa_connected() { return this->show_aa_connected; }
    inline void set_show_aa_connected(bool enabled)
    {
        this->show_aa_connected = enabled;
        this->settings.setValue("Pages/OpenAuto/show_aa_connected", this->show_aa_connected);
    }


    static Config *get_instance();

   private:
    QSettings settings;
    QString media_home;
    bool si_units;
    QString cam_network_url;
    QString cam_local_device;
    bool cam_is_network;
    QVideoFrame::PixelFormat cam_local_format_override;
    bool cam_autoconnect;
    int cam_autoconnect_time_secs;
    bool cam_overlay;
    int cam_overlay_width;
    int cam_overlay_height;
    bool show_aa_connected;

   signals:
    void si_units_changed(bool si_units);
    void cam_autoconnect_changed(bool enabled);
    void cam_overlay_changed(bool enabled);
};
