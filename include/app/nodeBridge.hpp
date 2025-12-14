#pragma once

#include <QObject>
#include <QPointer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtWebSockets/QWebSocket>

class MainWindow;

class NodeBridge : public QObject
{
    Q_OBJECT

public:
    explicit NodeBridge(QObject *parent = nullptr,
                        MainWindow* window = nullptr);
    ~NodeBridge() override;

    void setMainWindow(MainWindow* window);
    void connectToServer(const QUrl &url);

    Q_INVOKABLE void sendCustomMessage(const QJsonObject &payload);
    Q_INVOKABLE void sendCustomMessage(const QString &jsonString);

    void sendMetadata(const QJsonObject &metadata);
    void sendPlaybackStatus(const QString &status);
    
signals:
    void darkMode(bool enabled);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);

private:
    MainWindow* mainWindow = nullptr;
    QPointer<QWebSocket> socket_;
};
