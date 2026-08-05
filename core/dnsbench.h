#pragma once
#include <QObject>
#include <QStringList>
#include <algorithm>

struct DnsServer {
    QString name;
    QString ip;
    double  latencyMs = -1.0;
};

class DnsBench : public QObject {
    Q_OBJECT
public:
    explicit DnsBench(QObject *parent = nullptr);
    void runBenchmark();

signals:
    void benchmarkCompleted(const QList<DnsServer> &results);
    void fastestChanged(const DnsServer &server);

private:
    double testUdp(const QString &ip);
    double testDoh(const QString &url);
    double testDot(const QString &ip);

    QStringList m_targets = { "8.8.8.8", "1.1.1.1", "9.9.9.9", "8.8.4.4" };
};