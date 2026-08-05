#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVector>

struct PingResult;

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool init(const QString &path);

    void insertPingResult(const PingResult &result);
    void flushBuffer();

    bool insertDrop(qint64 ts, qint64 durationMs, const QString &reason);
    bool insertHourlyStats(qint64 hourTs, double avgLatency, double maxLatency, double avgJitter, double packetLoss, int dropCount);

private:
    bool createTables();
    QSqlDatabase        m_db;
    QVector<PingResult> m_buffer;
    int m_bufferLimit = 10;
};