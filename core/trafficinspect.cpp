#include "TrafficInspect.h"
#include "EventLogger.h"
#include <QProcess>

TrafficInspect::TrafficInspect(QObject *parent) : QObject(parent) {}

void TrafficInspect::start()
{
    EventLogger::instance().info("[TrafficInspect] Starting L3/L4 packet capture");

    m_process.start("netsh", {"trace", "start", "capture=yes", "IPv4.Address=0.0.0.0", "maxSize=50"});
    m_process.waitForFinished(5000);
}

void TrafficInspect::stop()
{
    EventLogger::instance().info("[TrafficInspect] Stopping packet capture");

    if (m_process.state() != QProcess::NotRunning) {
        m_process.kill();
        m_process.waitForFinished(1000);
    }

    QProcess p;
    p.start("netsh", {"trace", "stop"});
    p.waitForFinished(15000);
}