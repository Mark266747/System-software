#pragma once
#include <QObject>
#include <QTimer>

class QosScheduler : public QObject {
    Q_OBJECT
public:
    explicit QosScheduler(QObject *parent = nullptr);
    void start();
public slots:
    void onHighLoss(double lossPercent);   // High priority task
signals:
    void hourlyAggregationNeeded();        // Low priority task → Database
    void priorityTrafficNeeded(const QString &processName);
private slots:
    void onHourlyTimer();
private:
    QTimer m_hourlyTimer;
};
