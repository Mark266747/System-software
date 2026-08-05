#include "errorhandler.h"
#include "EventLogger.h"
#include <QCoreApplication>
#include <QProcess>
#include <cstdlib>

ErrorHandler::ErrorHandler(QObject *parent) : QObject(parent)
{
    install();
}

void ErrorHandler::install()
{
    qInstallMessageHandler(onQtMessage);
    std::set_terminate(onTerminate);
}

void ErrorHandler::onQtMessage(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    switch (type) {
    case QtCriticalMsg:
    case QtFatalMsg:
        EventLogger::instance().error("[Qt] " + msg.toStdString());
        if (type == QtFatalMsg) {
            rollbackAll();
            std::abort();
        }
        break;
    case QtWarningMsg:
        EventLogger::instance().warn("[Qt] " + msg.toStdString());
        break;
    default:
        EventLogger::instance().info("[Qt] " + msg.toStdString());
    }
}

void ErrorHandler::onTerminate()
{
    EventLogger::instance().error("std::terminate called — rolling back");
    rollbackAll();
    std::abort();
}

void ErrorHandler::rollbackAll()
{
    restoreDefaultDns();
    restoreDefaultMtu();
    EventLogger::instance().info("Rollback completed");
}

bool ErrorHandler::restoreDefaultDns()
{
    EventLogger::instance().info("[Rollback] Restoring default DNS (DHCP)");
    QProcess p;
    p.start("netsh", {"interface", "ipv4", "set", "dnsservers", "name=\"Ethernet\"", "source=dhcp"});
    p.waitForFinished(5000);
    return p.exitCode() == 0;
}

bool ErrorHandler::restoreDefaultMtu()
{
    EventLogger::instance().info("[Rollback] Restoring default MTU (1500)");
    QProcess p;

    p.start("netsh", {"interface", "ipv4", "set", "subinterface", "\"Ethernet\"", "mtu=1500", "store=persistent"});
    p.waitForFinished(5000);
    return p.exitCode() == 0;
}