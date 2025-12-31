#pragma once

#include <QtWidgets>
#include <QWebEngineView>
#include <QQueue>

#include "app/config.hpp"
#include "app/pages/page.hpp"

struct MediaEvent {
    QString track;
    QString artist;
    int length; // seconds
};

class Arbiter;

class WebviewPage : public QStackedWidget, public Page {
    Q_OBJECT
public:
    WebviewPage(Arbiter &arbiter, QWidget *parent = nullptr);
    void init() override;

private:
    QWidget* loadWebview();
    void showEvent(QShowEvent *event) override;

    void startNodeReconnect();
    void startMediaMetadataListener();

    void queueOrProcessMediaEvent(const MediaEvent &event);
    void processMediaEvent(const MediaEvent &event);

    QWebEngineView* view_;
    bool nodeOnline_;
    bool pageLoaded_;
    QQueue<MediaEvent> mediaEventQueue_;

    Arbiter &arbiter_;
};
