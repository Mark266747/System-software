#pragma once
#include <QObject>
#include <QString>
#include <QTimer>

class IpcServer;

class Failover : public QObject
{
    Q_OBJECT
public:
    explicit Failover(QObject *parent = nullptr);

    void setPrimaryAdapter(const QString &name);
    void setBackupAdapter(const QString &name);
    void setIpcServer(IpcServer *ipc);

public slots:
    void onConnectionDropped();
    void onPingRestored();
    void onHighLossDetected(double lossPercent);

signals:
    void switchedToBackup(const QString &adapterName);
    void restoredPrimary(const QString &adapterName);

private:
    bool getAdapterList();
    bool setAdapterMetric(const QString &name, int metric);
    bool checkAdapterAvailable(const QString &name);
    void tryRestorePrimary();

    QString     m_primary;
    QString     m_backup;
    bool        m_usingBackup = false;
    IpcServer  *m_ipc = nullptr;
    QTimer      m_restoreTimer;
    int         m_stableCount = 0;
};