#pragma once
#include <QObject>

class ErrorHandler : public QObject {
    Q_OBJECT
public:
    explicit ErrorHandler(QObject *parent = nullptr);

    static void install();
    static void rollbackAll();

private:
    static void onQtMessage(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
    static void onTerminate();

    static bool restoreDefaultDns();
    static bool restoreDefaultMtu();
};
