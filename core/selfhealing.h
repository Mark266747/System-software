#pragma once
#include <QObject>
#include <QString>

class SelfHealing : public QObject
{
    Q_OBJECT
public:
    explicit SelfHealing(QObject *parent = nullptr);

    enum class Mode { Full, MonitorOnly };
    void setMode(Mode mode);

public slots:
    void onHighLossDetected(double lossPercent);
    void runAction(const QString &action);

signals:
    void actionCompleted(const QString &action, const QString &result);

private:
    bool flushDns();
    bool renewDhcp();
    bool resetWinsock();
    bool resetMtu();

    Mode m_mode = Mode::Full;
};
