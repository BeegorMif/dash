#include "app/nodeBridge.hpp"
#include <QDebug>

NodeBridge::NodeBridge(QObject *parent)
    : QObject(parent)
{
    ws = new QWebSocket();
    connect(ws, &QWebSocket::connected, this, &NodeBridge::onConnected);
    connect(ws, &QWebSocket::disconnected, this, &NodeBridge::onDisconnected);
    connect(ws, &QWebSocket::textMessageReceived, this, &NodeBridge::onTextMessageReceived);
}

NodeBridge::~NodeBridge() {
    ws->close();
    delete ws;
}

void NodeBridge::connectToServer(const QUrl &url) {
    ws->open(url);
}

void NodeBridge::sendCustomMessage(const QJsonObject &payload) {
    if(ws->isValid()) {
        ws->sendTextMessage(QJsonDocument(payload).toJson(QJsonDocument::Compact));
    }
}

void NodeBridge::sendAAStatus(bool connected) {
{
    if(!ws) {
        qDebug() << "[NodeBridge] WebSocket not initialized!";
        return;
    }

    if(ws->isValid()) {
        QJsonObject payload;
        payload["type"] = "aa_status";
        payload["connected"] = connected;
        ws->sendTextMessage(QJsonDocument(payload).toJson(QJsonDocument::Compact));
        qDebug() << "[NodeBridge] Sent AA status:" << connected;
    } else {
        qDebug() << "[NodeBridge] WebSocket not valid yet. Cannot send AA status";
    }
}
}

void NodeBridge::sendMetadata(const QJsonObject &metadata) {
    QJsonObject payload = metadata;
    payload["type"] = "metadata";
    sendCustomMessage(payload);
}

void NodeBridge::sendPlaybackStatus(const QString &status) {
    QJsonObject payload;
    payload["type"] = "playback";
    payload["playbackStatus"] = status;
    sendCustomMessage(payload);
}

// Slots
void NodeBridge::onConnected() {
    qDebug() << "[NodeBridge] Connected to Node.js server";
    emit connected();
}

void NodeBridge::onDisconnected() {
    qDebug() << "[NodeBridge] Node.js WebSocket disconnected";
    emit disconnected();
}

void NodeBridge::onTextMessageReceived(const QString &message) {
    qDebug() << "[NodeBridge] Message from Node.js:" << message;
}
