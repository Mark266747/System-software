#pragma once
#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMutex>

struct PingResult;

class IpcServer : public QObject
{
    Q_OBJECT

public:
    explicit IpcServer(QObject *parent = nullptr);
    ~IpcServer();

    bool start(const QString &pipeName);
    void stop();

    void sendPingEvent(const PingResult &result);
    void sendAlert(const QString &code, double value);
    void sendHealingResult(const QString &action, const QString &result);
    void sendDrop(qint64 timestamp);

signals:
    void profileChangeRequested(const QString &profile);
    void healingRequested(const QString &action);
    void dnsBenchRequested();
    void statusRequested();

private slots:
    void onNewConnection();
    void onClientData();
    void onClientDisconnected();

private:
    void processCommand(const QJsonObject &cmd);
    void broadcast(const QJsonObject &msg);
    bool validateCommand(const QJsonObject &cmd);

    QLocalServer          m_server;
    QList<QLocalSocket*>  m_clients;
    QMutex                m_mutex;
};