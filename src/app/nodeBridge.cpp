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
    if (!socket_) return;
    serverUrl_ = url;  // save it for reconnects
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
    payload["action"] = status;
    sendCustomMessage(payload);
}

void NodeBridge::onConnected()
{
    DASH_LOG(info) << "[NodeBridge] Connected to Node.js server";
}

void NodeBridge::onDisconnected()
{
    DASH_LOG(info) << "[NodeBridge] Node.js WebSocket disconnected, retrying in 3s";

    if (!reconnectTimer_) {
        reconnectTimer_ = new QTimer(this);
        reconnectTimer_->setSingleShot(true);
        connect(reconnectTimer_, &QTimer::timeout, this, [this]() {
            if (socket_->state() == QAbstractSocket::UnconnectedState)
                socket_->open(serverUrl_);
        });
    }

    reconnectTimer_->start(3000);
}

void NodeBridge::onTextMessageReceived(const QString &message)
{
    // DASH_LOG(debug) << "[NodeBridge] Raw message:" << message.toStdString();

    const QJsonDocument doc =
        QJsonDocument::fromJson(message.toUtf8());

    if (!doc.isObject()) {
        DASH_LOG(debug)     << "[Node Bridge] Invalid JSON Message";
        return;
    }

    const QJsonObject msg = doc.object();
    const QString type = msg.value("type").toString();
    const QString action = msg.value("action").toString();
    const QString source = msg.value("source").toString();
    const QJsonObject payload =
        msg.value("payload").toObject();

    if (type == "system") {
        if (action == "extra_dim") {
            const bool enabled = payload.value("value").toBool(false);
            if (mainWindow)
                mainWindow->setDim(enabled);
        } else if (action == "blackout") {
            const bool enabled = payload.value("value").toBool(false);
            if (mainWindow)
                mainWindow->setBlackout(enabled);
        } else if (action == "darkMode") {
            const bool enabled = payload.value("enabled").toBool(false);
            emit darkMode(enabled);
        }
    } else if (type == "vehicle" && action == "lights") {
        const bool headlightsOn = payload.value("headlights").toBool(false);
        emit darkMode(headlightsOn);
    } else if (type =="browser" && action == "tabChange") {
        if (mainWindow) {
            mainWindow->onTabChanged(
                payload.value("tab").toString(),
                payload.value("aaConnected").toBool()
            );
        }
    } else if (type == "media") {
        if (mainWindow && mainWindow->openAutoPage())
            mainWindow->openAutoPage()->sendMediaKey(action);
    } else if (type == "canbus") {
        return;
    }
    else {
        DASH_LOG(debug)
            << "[NodeBridge] Unhandled message:"
            << type.toStdString()
            << ":" << action.toStdString();
    }
}
