#include "QosScheduler.h"
#include "EventLogger.h"
#include <QProcess>

QosScheduler::QosScheduler(QObject *p) : QObject(p) {
    connect(&m_hourlyTimer, &QTimer::timeout, this, &QosScheduler::onHourlyTimer);
}

void QosScheduler::start() { m_hourlyTimer.start(3600 * 1000); }

void QosScheduler::onHighLoss(double loss)
{

    if (loss < 5.0) return;

    EventLogger::instance().warn("[QoS] High loss detected. Throttling BITS transfer rates.");
    QProcess p;
    p.start("powershell", { "-Command", "Import-Module BitsTransfer; Set-BitsManager -MaxDownloadRate 10 -MaxUploadRate 10"
     });
    p.waitForFinished(5000);
}

void QosScheduler::onHourlyTimer() { emit hourlyAggregationNeeded(); }