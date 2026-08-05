#include "selfhealing.h"
#include <QDebug>
#include <QProcess>

SelfHealing::SelfHealing(QObject *parent) : QObject(parent) {}

void SelfHealing::setMode(Mode mode) { m_mode = mode; }

void SelfHealing::onHighLossDetected(double lossPercent)
{
    if (m_mode == Mode::MonitorOnly) return;
    qDebug() << "[SelfHealing] High loss detected:" << lossPercent << "%";
    if (flushDns())
        emit actionCompleted("flushdns", "ok");
    else
        emit actionCompleted("flushdns", "failed");
}

void SelfHealing::runAction(const QString &action)
{
    if (m_mode == Mode::MonitorOnly) return;
    bool ok = false;
    if      (action == "flushdns") ok = flushDns();
    else if (action == "renew")    ok = renewDhcp();
    else if (action == "winsock")  ok = resetWinsock();
    emit actionCompleted(action, ok ? "ok" : "failed");
}

bool SelfHealing::flushDns()
{
    qDebug() << "[SelfHealing] Flushing DNS...";
    QProcess p;
    p.start("ipconfig", {"/flushdns"});
    p.waitForFinished(5000);
    bool ok = p.exitCode() == 0;
    qDebug() << "[SelfHealing] FlushDNS:" << (ok ? "ok" : "failed");
    return ok;
}

bool SelfHealing::renewDhcp()
{
    qDebug() << "[SelfHealing] Renewing DHCP...";
    QProcess p;
    p.start("ipconfig", {"/renew"});
    p.waitForFinished(15000);
    bool ok = p.exitCode() == 0;
    qDebug() << "[SelfHealing] RenewDHCP:" << (ok ? "ok" : "failed");
    return ok;
}

bool SelfHealing::resetWinsock()
{
    qDebug() << "[SelfHealing] Resetting Winsock...";
    QProcess p;
    p.start("netsh", {"winsock", "reset"});
    p.waitForFinished(10000);
    bool ok = p.exitCode() == 0;
    qDebug() << "[SelfHealing] Winsock reset:" << (ok ? "ok" : "failed");
    return ok;
}

bool SelfHealing::resetMtu()
{
    qDebug() << "[SelfHealing] Resetting MTU...";
    QProcess p;
    p.start("netsh", {"interface", "ipv4", "set", "subinterface", "Ethernet", "mtu=1500", "store=persistent"});
    p.waitForFinished(5000);
    return p.exitCode() == 0;
}
