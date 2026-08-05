#include "ipcserver.h"
#include "monitor.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

IpcServer::IpcServer(QObject *parent) : QObject(parent)
{
    connect(&m_server, &QLocalServer::newConnection, this, &IpcServer::onNewConnection);
}

IpcServer::~IpcServer() { stop(); }

bool IpcServer::start(const QString &pipeName)
{
    QLocalServer::removeServer(pipeName);
    if (!m_server.listen(pipeName)) {
        qDebug() << "[IPC] Cannot start:" << m_server.errorString();
        return false;
    }
    qDebug() << "[IPC] Listening on pipe:" << pipeName;
    return true;
}

void IpcServer::stop()
{
    for (auto *client : qAsConst(m_clients)) client->disconnectFromServer();
    m_server.close();
}

void IpcServer::onNewConnection()
{
    QLocalSocket *client = m_server.nextPendingConnection();
    connect(client, &QLocalSocket::readyRead,
            this, &IpcServer::onClientData);
    connect(client, &QLocalSocket::disconnected,
            this, &IpcServer::onClientDisconnected);
    m_clients.append(client);
    qDebug() << "[IPC] Client connected";
}

void IpcServer::onClientDisconnected()
{
    auto *client = qobject_cast<QLocalSocket*>(sender());
    m_clients.removeAll(client);
    client->deleteLater();
    qDebug() << "[IPC] Client disconnected";
}

void IpcServer::onClientData()
{
    auto *client = qobject_cast<QLocalSocket*>(sender());
    while (client->canReadLine()) {
        QByteArray line = client->readLine().trimmed();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError) {
            qDebug() << "[IPC] Invalid JSON";
            continue;
        }
        QJsonObject cmd = doc.object();
        if (validateCommand(cmd))
            processCommand(cmd);
        else
            qDebug() << "[IPC] Rejected invalid command";
    }
}

bool IpcServer::validateCommand(const QJsonObject &cmd)
{
    static const QStringList allowed = {
        "set_profile", "run_healing", "dns_bench", "get_status"
    };
    return cmd.contains("cmd") && allowed.contains(cmd["cmd"].toString());
}

void IpcServer::processCommand(const QJsonObject &cmd)
{
    QString c = cmd["cmd"].toString();
    if      (c == "set_profile")  emit profileChangeRequested(cmd["profile"].toString());
    else if (c == "run_healing")  emit healingRequested(cmd["action"].toString());
    else if (c == "dns_bench")    emit dnsBenchRequested();
    else if (c == "get_status")   emit statusRequested();
}

void IpcServer::broadcast(const QJsonObject &msg)
{
    QMutexLocker lock(&m_mutex);
    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n";
    for (auto *client : qAsConst(m_clients))
        if (client->isOpen())
            client->write(data);
}

void IpcServer::sendPingEvent(const PingResult &result)
{
    QJsonObject msg;
    msg["type"]   = "ping";
    msg["ms"]     = result.latencyMs;
    msg["jitter"] = result.jitterMs;
    msg["loss"]   = result.packetLoss;
    msg["ts"]     = result.timestamp;
    broadcast(msg);
}

void IpcServer::sendAlert(const QString &code, double value)
{
    broadcast({ {"type","alert"}, {"code",code}, {"value",value} });
}

void IpcServer::sendHealingResult(const QString &action, const QString &res)
{
    broadcast({ {"type","healing"}, {"action",action}, {"result",res} });
}

void IpcServer::sendDrop(qint64 timestamp)
{
    broadcast({ {"type","drop"}, {"ts", timestamp} });
}
