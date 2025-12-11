#pragma once

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>

class NodeBridge : public QObject
{
    Q_OBJECT

public:
    explicit NodeBridge(QObject *parent = nullptr);
    ~NodeBridge();

    // Connect to Node server
    void connectToServer(const QUrl &url);

    // Send AA status, playback metadata, or arbitrary payload
    void sendAAStatus(bool connected);
    void sendMetadata(const QJsonObject &metadata);
    void sendPlaybackStatus(const QString &status);
    void sendCustomMessage(const QJsonObject &payload);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);

private:
    QWebSocket *ws;
};
