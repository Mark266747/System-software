#pragma once
#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QDateTime>
#include <QVector>
#include <QString>

// Forward declarations — не включаємо заголовки повністю
class Database;
class IpcServer;

struct PingResult {
    qint64  timestamp  = 0;
    QString target;
    double  latencyMs  = -1.0;
    double  jitterMs   = 0.0;
    double  packetLoss = 0.0;
    bool    isTimeout  = true;
};

class Monitor : public QObject
{
    Q_OBJECT

public:
    explicit Monitor(QObject *parent = nullptr);
    ~Monitor();

    void setDatabase(Database *db);
    void setIpcServer(IpcServer *ipc);
    void start();
    void stop();
    void setTarget(const QString &host);

signals:
    void pingCompleted(const PingResult &result);
    void highLossDetected(double lossPercent);
    void connectionDropped();

private slots:
    void onPingOutput();
    void onPingFinished();

private:
    double calculateJitter(double currentMs);
    double calculatePacketLoss();
    void   processLine(const QString &line);

    QProcess        m_process;
    QString         m_target        = "8.8.8.8";
    double          m_lastLatency   = -1.0;
    double          m_lossThreshold = 40.0;
    int             m_lossWindow    = 10;
    QVector<double> m_history;
    QString         m_buffer;
    qint64 m_lastLossAlert = 0;
    qint64 m_lastDropAlert = 0;
    const qint64 ALERT_COOLDOWN_MS = 30000;

    Database  *m_db  = nullptr;
    IpcServer *m_ipc = nullptr;
};