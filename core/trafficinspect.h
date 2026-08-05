#pragma once
#include <QObject>
#include <QProcess>

class TrafficInspect : public QObject {
    Q_OBJECT
public:
    explicit TrafficInspect(QObject *parent = nullptr);
    void start();
    void stop();
signals:
    void highTrafficDetected(const QString &processName, quint64 bytesPerSec);
private:
    QProcess m_process;
};
