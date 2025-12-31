#include <BluezQt/Device>
#include <BluezQt/PendingCall>
#include <QLabel>
#include <QLayoutItem>
#include <QScrollArea>

#include <aasdk_proto/ButtonCodeEnum.pb.h>
#include <aasdk_proto/VideoFPSEnum.pb.h>
#include <aasdk_proto/VideoResolutionEnum.pb.h>

#include "openauto/Configuration/AudioOutputBackendType.hpp"
#include "openauto/Configuration/BluetootAdapterType.hpp"
#include "openauto/Configuration/HandednessOfTrafficType.hpp"

#include "app/config.hpp"
#include "app/session.hpp"
#include "app/window.hpp"
#include "app/pages/settings.hpp"
#include "gitversion.h"

SettingsPage::SettingsPage(Arbiter &arbiter, QWidget *parent)
    : QTabWidget(parent)
    , Page(arbiter, "Settings", false, this)
{
}

void SettingsPage::init()
{
}


