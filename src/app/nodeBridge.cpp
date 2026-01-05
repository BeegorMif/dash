#include "app/nodeBridge.hpp"
#include "app/window.hpp"
#include "DashLog.hpp"

NodeBridge::NodeBridge(QObject *parent, MainWindow* window)
    : QObject(parent),
      mainWindow(window)
{
    socket_ = new QWebSocket(QString(),
                            QWebSocketProtocol::VersionLatest,
                            this);

    connect(socket_, &QWebSocket::connected, this, &NodeBridge::onConnected);
    connect(socket_, &QWebSocket::disconnected, this, &NodeBridge::onDisconnected);
    connect(socket_, &QWebSocket::textMessageReceived, this, &NodeBridge::onTextMessageReceived);
}

NodeBridge::~NodeBridge()
{
    if (socket_) {
        socket_->disconnect(this);
        socket_->close();
        socket_ = nullptr;
    }
}   

void NodeBridge::setMainWindow(MainWindow* window)
{
    mainWindow = window;
}

void NodeBridge::connectToServer(const QUrl &url)
{
    if (!socket_)
        return;

    socket_->open(url);
}

void NodeBridge::sendCustomMessage(const QJsonObject &payload)
{
    sendCustomMessage(
        QJsonDocument(payload).toJson(QJsonDocument::Compact));
}

void NodeBridge::sendCustomMessage(const QString &jsonString)
{
    QPointer<QWebSocket> socketPtr(socket_);

    QMetaObject::invokeMethod(
        this,
        [socketPtr, jsonString]() {
            if (!socketPtr)
                return;

            if (socketPtr->state() != QAbstractSocket::ConnectedState)
                return;

            socketPtr->sendTextMessage(jsonString);
        },
        Qt::QueuedConnection
    );
}

void NodeBridge::sendMetadata(const QJsonObject &metadata)
{
    QJsonObject payload = metadata;
    payload["type"] = "metadata";
    sendCustomMessage(payload);
}

void NodeBridge::sendPlaybackStatus(const QString &status)
{
    QJsonObject payload;
    payload["type"] = "playback";
    payload["playbackStatus"] = status;
    sendCustomMessage(payload);
}

void NodeBridge::onConnected()
{
    DASH_LOG(info) << "[NodeBridge] Connected to Node.js server";
        QJsonObject hello;
    hello["type"] = "hello";
    hello["role"] = "dash_cpp";

    sendCustomMessage(hello);
}

void NodeBridge::onDisconnected()
{
    DASH_LOG(info) << "[NodeBridge] Node.js WebSocket disconnected";
}

void NodeBridge::onTextMessageReceived(const QString &message)
{
    DASH_LOG(debug) << "[NodeBridge] Raw message:" << message.toStdString();

    const QJsonDocument doc =
        QJsonDocument::fromJson(message.toUtf8());

    if (!doc.isObject()) {
        DASH_LOG(debug)     << "[Node Bridge] Invalid JSON Message";
        return;
    }

    const QJsonObject obj = doc.object();
    const QString type = obj.value("type").toString();

    if (type == "blackout") {
        const bool enabled = obj.value("enabled").toBool(false);
        if (mainWindow)
            mainWindow->setBlackout(enabled);
    } else if (type == "vehicle.lights") {
        const bool headlightsOn = obj.value("headlights").toBool(false);
        DASH_LOG(info) << "[NodeBridge] Headlights are"
                       << (headlightsOn ? "ON" : "OFF");
                       emit darkMode(headlightsOn);
    } else if (type == "darkMode") {
        const bool enabled = obj.value("enabled").toBool(false);
        emit darkMode(enabled);
    } else if (type =="tabChange") {
        mainWindow->onTabChanged(obj.value("tab").toString(), obj.value("aaConnected").toBool());
    }
}
