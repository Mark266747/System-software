#include "monitor.h"
#include "database.h"
#include "ipcserver.h"
#include "eventlogger.h"
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

Monitor::Monitor(QObject *parent) : QObject(parent)
{
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &Monitor::onPingOutput);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Monitor::onPingFinished);
}

Monitor::~Monitor() { stop(); }

void Monitor::setDatabase(Database *db)   { m_db  = db;  }
void Monitor::setIpcServer(IpcServer *ipc){ m_ipc = ipc; }
void Monitor::setTarget(const QString &h) { m_target = h;}

void Monitor::start()
{

    m_process.start("ping", { "-n", "9999", "-w", "2000", m_target
     });
    qDebug() << "[Monitor] Continuous ping started →" << m_target;
}

void Monitor::stop()
{
    if (m_process.state() != QProcess::NotRunning) {
        m_process.kill();
        m_process.waitForFinished(1000);
    }
}

void Monitor::onPingOutput()
{
    m_buffer += QString::fromLocal8Bit(
        m_process.readAllStandardOutput());

    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QString line = m_buffer.left(idx).trimmed();
        m_buffer = m_buffer.mid(idx + 1);
        if (!line.isEmpty())
            processLine(line);
    }
}

void Monitor::processLine(const QString &line)
{
    QRegularExpression rxOk(
        "(?:time)[<=](\\d+)(?:ms|ms)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpression rxTimeout(
        "(?:timed out|timeout)",
        QRegularExpression::CaseInsensitiveOption);

    PingResult result;
    result.timestamp = QDateTime::currentMSecsSinceEpoch();
    result.target    = m_target;

    QRegularExpressionMatch m = rxOk.match(line);
    if (m.hasMatch()) {
        result.latencyMs = m.captured(1).toDouble();
        result.isTimeout = false;
    } else if (rxTimeout.match(line).hasMatch()) {
        result.latencyMs = -1.0;
        result.isTimeout = true;
    } else {
        return;
    }

    if (m_history.size() >= m_lossWindow)
        m_history.removeFirst();

    m_history.append(result.isTimeout ? -1.0 : result.latencyMs);

    result.jitterMs   = calculateJitter(result.latencyMs);
    result.packetLoss = calculatePacketLoss();
    m_lastLatency     = result.latencyMs;

    if (m_db)  m_db->insertPingResult(result);
    if (m_ipc) m_ipc->sendPingEvent(result);
    emit pingCompleted(result);

    if (m_history.size() >= m_lossWindow) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();

        if (result.packetLoss > m_lossThreshold) {
            if (now - m_lastLossAlert >= ALERT_COOLDOWN_MS) {
                emit highLossDetected(result.packetLoss);
                m_lastLossAlert = now;
            }
        }

        if (result.isTimeout) {
            if (now - m_lastDropAlert >= ALERT_COOLDOWN_MS) {
                emit connectionDropped();
                m_lastDropAlert = now;
            }
        }
    }
}

void Monitor::onPingFinished()
{
    qDebug() << "[Monitor] Ping process finished, restarting...";
    QTimer::singleShot(1000, this, &Monitor::start);
}

double Monitor::calculateJitter(double currentMs)
{
    if (m_lastLatency < 0 || currentMs < 0) return 0.0;
    return qAbs(currentMs - m_lastLatency);
}

double Monitor::calculatePacketLoss()
{
    if (m_history.isEmpty()) return 0.0;
    int lost = 0;
    for (double v : qAsConst(m_history))
        if (v < 0) lost++;
    return (double)lost / m_history.size() * 100.0;
}

#include "monitor.moc"