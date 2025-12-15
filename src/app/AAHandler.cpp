#include "AAHandler.hpp"


AAHandler::AAHandler() :
QObject()
{

}

AAHandler::~AAHandler()
{

}

void AAHandler::mediaPlaybackUpdate(const aasdk::proto::messages::MediaInfoChannelPlaybackData& playback)
{
    emit aa_media_playback_update(playback);
}

void AAHandler::mediaMetadataUpdate(const aasdk::proto::messages::MediaInfoChannelMetadataData& metadata)
{
    emit aa_media_metadata_update(metadata);
}


void AAHandler::navigationStatusUpdate(const aasdk::proto::messages::NavigationStatus& navStatus)
{
    emit aa_navigation_status_update(navStatus);
}
void AAHandler::navigationTurnEvent(const aasdk::proto::messages::NavigationTurnEvent& turnEvent)
{
    emit aa_navigation_turn_event(turnEvent);
}
void AAHandler::navigationDistanceEvent(const aasdk::proto::messages::NavigationDistanceEvent& distanceEvent)
{
    emit aa_navigation_distance_event(distanceEvent);
}
