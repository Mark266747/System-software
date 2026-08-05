#include "failover.h"
#include "ipcserver.h"
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

Failover::Failover(QObject *parent) : QObject(parent)
{
    connect(&m_restoreTimer, &QTimer::timeout, this, &Failover::tryRestorePrimary);
}

void Failover::setPrimaryAdapter(const QString &n) { m_primary = n; }
void Failover::setBackupAdapter(const QString &n)  { m_backup  = n; }
void Failover::setIpcServer(IpcServer *ipc)        { m_ipc = ipc;   }

void Failover::onConnectionDropped()
{
    if (m_usingBackup) return;

    qDebug() << "[Failover] Connection dropped detected";

    if (!checkAdapterAvailable(m_backup)) {
        qDebug() << "[Failover] Backup adapter not available:" << m_backup;
        return;
    }

    qDebug() << "[Failover] Switching to backup:" << m_backup;

    bool ok1 = setAdapterMetric(m_primary, 9000);
    bool ok2 = setAdapterMetric(m_backup,  1);

    if (ok1 && ok2) {
        m_usingBackup = true;
        m_stableCount = 0;
        qDebug() << "[Failover] Switched to backup successfully";

        if (m_ipc)
            m_ipc->sendHealingResult("failover", "switched to " + m_backup);

        emit switchedToBackup(m_backup);

        m_restoreTimer.start(30000);
    } else {
        qDebug() << "[Failover] Failed to switch adapter";
    }
}

void Failover::onPingRestored()
{
    if (!m_usingBackup) return;

    m_stableCount++;
    qDebug() << "[Failover] Stable ping count:" << m_stableCount;

    if (m_stableCount >= 5)
        tryRestorePrimary();
}

void Failover::tryRestorePrimary()
{
    if (!m_usingBackup) {
        m_restoreTimer.stop();
        return;
    }

    qDebug() << "[Failover] Trying to restore primary:" << m_primary;

    if (!checkAdapterAvailable(m_primary)) {
        qDebug() << "[Failover] Primary still unavailable";
        m_stableCount = 0;
        return;
    }

    bool ok1 = setAdapterMetric(m_primary, 1);
    bool ok2 = setAdapterMetric(m_backup,  9000);

    if (ok1 && ok2) {
        m_usingBackup = false;
        m_stableCount = 0;
        m_restoreTimer.stop();
        qDebug() << "[Failover] Restored primary:" << m_primary;

        if (m_ipc)
            m_ipc->sendHealingResult("failover", "restored to " + m_primary);

        emit restoredPrimary(m_primary);
    }
}

bool Failover::checkAdapterAvailable(const QString &name)
{
    QProcess p;
    p.start("netsh", {"interface", "show", "interface", name});
    p.waitForFinished(3000);
    QString out = QString::fromLocal8Bit(p.readAllStandardOutput());

    return out.contains("Connected",   Qt::CaseInsensitive) ||
           out.contains("Підключено",  Qt::CaseInsensitive);
}

bool Failover::setAdapterMetric(const QString &name, int metric)
{
    QProcess p;
    p.start("netsh", {"interface", "ip", "set", "interface", name, "metric=" + QString::number(metric)
     });
    p.waitForFinished(5000);

    bool ok = p.exitCode() == 0;
    qDebug() << "[Failover] Set metric" << metric << "for" << name << ":" << (ok ? "ok" : "failed");
    return ok;
}

void Failover::onHighLossDetected(double lossPercent)
{
    if (lossPercent > 40.0 && !m_usingBackup) {
        qDebug() << "[Failover] Critical packet loss detected:" << lossPercent << "%";
        onConnectionDropped();
    }
}

#include "failover.moc"