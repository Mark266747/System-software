#include <QCoreApplication>
#include <QDebug>
#include "monitor.h"
#include "dnsbench.h"
#include "errorhandler.h"
#include "failover.h"
#include "qosscheduler.h"
#include "selfhealing.h"
#include "trafficinspect.h"
#include "database.h"
#include "ipcserver.h"

int main(int argc, char *argv[])
{
    

    Database db;
    if (db.init("netguard.db"))
        qDebug() << "[OK] Database";
    else
        qDebug() << "[FAIL] Database";

    IpcServer ipcServer;
    if (ipcServer.start("NetGuardPipe"))
        qDebug() << "[OK] IpcServer";
    else
        qDebug() << "[FAIL] IpcServer";


    SelfHealing selfHealing;
    qDebug() << "[OK] SelfHealing";

    QosScheduler scheduler;
    scheduler.start();
    qDebug() << "[OK] QosScheduler";

    DnsBench dnsBench;
    qDebug() << "[OK] DnsBench";

    Monitor monitor;
    monitor.setTarget("8.8.8.8");

    QObject::connect(&monitor, &Monitor::pingCompleted, [](const PingResult &result)
    {
     if (result.isTimeout)
           qDebug() << "[Monitor] TIMEOUT";
     else
        qDebug() << "[Monitor] Ping:" << result.latencyMs << "ms";
        });

    QObject::connect(&monitor, &Monitor::pingCompleted, [&db](const PingResult &result){
     db.insertPingResult(result);
    });

    QObject::connect(&monitor, &Monitor::pingCompleted, [&ipcServer](const PingResult &r){
        ipcServer.sendPingEvent(r);
    });

    QObject::connect(&monitor, &Monitor::pingCompleted, [&selfHealing](const PingResult &result){
        if (result.isTimeout)
         selfHealing.onHighLossDetected(100.0);
    });

    QObject::connect(&monitor, &Monitor::pingCompleted, [&scheduler](const PingResult &result){
        if (result.isTimeout)
         scheduler.onHighLoss(100.0);
    });

    QObject::connect(&ipcServer, &IpcServer::dnsBenchRequested, [&dnsBench](){
     dnsBench.runBenchmark();
    });

    QObject::connect(&dnsBench, &DnsBench::benchmarkCompleted, [&ipcServer](const QList<DnsServer> &results){
     for (const auto &s : results) {
      ipcServer.sendAlert( QString("DNS_%1").arg(s.ip), s.latencyMs);
     }
    });

    monitor.start();
    qDebug() << "[OK] Monitor";

    Failover failover;
    failover.setPrimaryAdapter("Ethernet");
    failover.setBackupAdapter("Wi-Fi");
    failover.setIpcServer(&ipcServer);


    QObject::connect(&monitor, &Monitor::connectionDropped, &failover, &Failover::onConnectionDropped);

    QObject::connect(&monitor, &Monitor::pingCompleted, [&failover](const PingResult &r){
        if (!r.isTimeout)
         failover.onPingRestored();
        });

    QObject::connect(&monitor, &Monitor::highLossDetected, &failover, &Failover::onHighLossDetected);

    TrafficInspect trafficInspect;
    trafficInspect.start();
    qDebug() << "[OK] TrafficInspect";

    qDebug() << "=== All modules initialized ===";
    return app.exec();
}