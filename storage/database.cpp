#include "database.h"
#include "monitor.h"
#include "eventlogger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

Database::Database(QObject *parent) : QObject(parent) {}

Database::~Database()
{
    flushBuffer();
}

bool Database::init(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qDebug() << "[DB] Cannot open:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode=WAL");
    q.exec("PRAGMA synchronous=NORMAL");

    qDebug() << "[DB] Opened:" << path;
    return createTables();
}

bool Database::createTables()
{
    QSqlQuery q(m_db);

    bool ok = q.exec(R"(
        CREATE TABLE IF NOT EXISTS connection_drops (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            ts          INTEGER NOT NULL,
            duration_ms INTEGER,
            reason      TEXT
        )
    )");

    ok &= q.exec(R"(
        CREATE TABLE IF NOT EXISTS ping_log (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            ts        INTEGER NOT NULL,
            target    TEXT,
            latency   REAL,
            jitter    REAL,
            loss      REAL,
            timeout   INTEGER
        )
    )");

    ok &= q.exec(R"(
        CREATE TABLE IF NOT EXISTS hourly_statistics (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            hour_ts      INTEGER NOT NULL,
            avg_latency  REAL,
            max_latency  REAL,
            avg_jitter   REAL,
            packet_loss  REAL,
            drop_count   INTEGER
        )
    )");

    if (ok)
        qDebug() << "[DB] Tables ready";
    else
        qDebug() << "[DB] Table error:" << q.lastError().text();

    return ok;
}

void Database::insertPingResult(const PingResult &result)
{
    m_buffer.append(result);
    if (m_buffer.size() >= m_bufferLimit)
        flushBuffer();
}

void Database::flushBuffer()
{
    if (m_buffer.isEmpty()) return;

    m_db.transaction();
    QSqlQuery q(m_db);

    for (const auto &r : qAsConst(m_buffer)) {
        q.prepare(R"(INSERT INTO ping_log (ts, target, latency, jitter, loss, timeout) VALUES (?, ?, ?, ?, ?, ?))");
        q.addBindValue(r.timestamp);
        q.addBindValue(r.target);
        q.addBindValue(r.latencyMs);
        q.addBindValue(r.jitterMs);
        q.addBindValue(r.packetLoss);
        q.addBindValue(r.isTimeout ? 1 : 0);
        q.exec();
    }

    m_db.commit();
    qDebug() << "[DB] Flushed" << m_buffer.size() << "records";
    m_buffer.clear();
}

bool Database::insertDrop(qint64 ts, qint64 durationMs, const QString &reason)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO connection_drops (ts, duration_ms, reason) VALUES (?,?,?)");
    q.addBindValue(ts);
    q.addBindValue(durationMs);
    q.addBindValue(reason);
    return q.exec();
}

bool Database::insertHourlyStats(qint64 hourTs, double avgLatency, double maxLatency, double avgJitter, double packetLoss, int dropCount)
{
    QSqlQuery q(m_db);
    q.prepare(R"(INSERT INTO hourly_statistic (hour_ts, avg_latency, max_latency, avg_jitter, packet_loss, drop_count) VALUES (?,?,?,?,?,?))");
    q.addBindValue(hourTs);
    q.addBindValue(avgLatency);
    q.addBindValue(maxLatency);
    q.addBindValue(avgJitter);
    q.addBindValue(packetLoss);
    q.addBindValue(dropCount);
    return q.exec();
}