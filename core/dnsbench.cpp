#include "dnsbench.h"
#include <QDebug>
#include <QProcess>
#include <QDateTime>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <QSslSocket>
#include <QTimer>
#include <QtNetwork>

DnsBench::DnsBench(QObject *parent) : QObject(parent) {}

void DnsBench::runBenchmark()
{
    qDebug() << "[DnsBench] Starting benchmark...";
    QList<DnsServer> results;

    struct Target {
        QString name;
        QString ip;
        QString dohUrl;
    };

    QList<Target> targets = {
                             {"Google DNS",    "8.8.8.8",  "https://dns.google/resolve"},
                             {"Cloudflare",    "1.1.1.1",  "https://cloudflare-dns.com/dns-query"},
                             {"Quad9",         "9.9.9.9",  "https://dns.quad9.net/dns-query"},
                             {"Google DNS 2",  "8.8.4.4",  "https://dns.google/resolve"},
                             };

    for (const auto &t : targets) {
        DnsServer server;
        server.name = t.name;
        server.ip   = t.ip;

        double udp = testUdp(t.ip);

        double doh = testDoh(t.dohUrl);

        double dot = testDot(t.ip);

        QList<double> valid;
        if (udp > 0) valid.append(udp);
        if (doh > 0) valid.append(doh);
        if (dot > 0) valid.append(dot);

        if (!valid.isEmpty()) {
            server.latencyMs = *std::min_element(
                valid.begin(), valid.end());
        } else {
            server.latencyMs = -1.0;
        }

        qDebug() << "[DnsBench]" << t.name
                 << "UDP:" << udp
                 << "DoH:" << doh
                 << "DoT:" << dot
                 << "Best:" << server.latencyMs << "ms";

        results.append(server);
    }

    std::sort(results.begin(), results.end(),
              [](const DnsServer &a, const DnsServer &b) {
                  if (a.latencyMs < 0) return false;
                  if (b.latencyMs < 0) return true;
                  return a.latencyMs < b.latencyMs;
              });

    emit benchmarkCompleted(results);

    if (!results.isEmpty() && results.first().latencyMs > 0)
        emit fastestChanged(results.first());
}

double DnsBench::testUdp(const QString &ip)
{
    QProcess p;
    qint64 start = QDateTime::currentMSecsSinceEpoch();
    p.start("ping", {"-n", "3", "-w", "1000", ip});
    p.waitForFinished(5000);

    if (p.exitCode() != 0)
        return -1.0;

    QString output = QString::fromLocal8Bit(
        p.readAllStandardOutput());

    QRegularExpression rx(
        "(?:Average)\\s*=\\s*(\\d+)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m = rx.match(output);
    if (m.hasMatch())
        return m.captured(1).toDouble();

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - start;
    return elapsed / 3.0;
}

double DnsBench::testDoh(const QString &url)
{
    QNetworkAccessManager manager;
    QEventLoop loop;

    QString fullUrl = url + "?name=google.com&type=A";
    QNetworkRequest request{QUrl(fullUrl)};
    request.setRawHeader("Accept", "application/dns-json");

    qint64 start = QDateTime::currentMSecsSinceEpoch();
    QNetworkReply *reply = manager.get(request);

    QTimer timer;
    timer.setSingleShot(true);
    timer.start(3000);

    QObject::connect(reply,   &QNetworkReply::finished,
                     &loop,   &QEventLoop::quit);
    QObject::connect(&timer,  &QTimer::timeout,
                     &loop,   &QEventLoop::quit);

    loop.exec();

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - start;

    if (timer.isActive() &&
        reply->error() == QNetworkReply::NoError) {
        timer.stop();
        reply->deleteLater();
        qDebug() << "[DnsBench] DoH" << url << elapsed << "ms";
        return static_cast<double>(elapsed);
    }

    reply->abort();
    reply->deleteLater();
    qDebug() << "[DnsBench] DoH" << url << "failed:"
             << reply->errorString();
    return -1.0;
}

double DnsBench::testDot(const QString &ip)
{
    if (!QSslSocket::supportsSsl()) {
        qDebug() << "[DnsBench] DoT: SSL not supported";
        return -1.0;
    }

    QSslSocket socket;
    QEventLoop loop;

    socket.setPeerVerifyMode(QSslSocket::VerifyNone);

    qint64 start = QDateTime::currentMSecsSinceEpoch();

    socket.connectToHostEncrypted(ip, 853);

    QTimer timer;
    timer.setSingleShot(true);
    timer.start(3000);

    QObject::connect(&socket, &QSslSocket::encrypted,
                     &loop,   &QEventLoop::quit);
    QObject::connect(&socket,
                     QOverload<const QList<QSslError>&>::of(
                         &QSslSocket::sslErrors),
                     &loop, &QEventLoop::quit);
    QObject::connect(&timer,  &QTimer::timeout,
                     &loop,   &QEventLoop::quit);

    loop.exec();

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - start;

    bool connected = socket.isEncrypted();
    socket.disconnectFromHost();
    timer.stop();

    if (connected) {
        qDebug() << "[DnsBench] DoT" << ip << elapsed << "ms";
        return static_cast<double>(elapsed);
    }

    qDebug() << "[DnsBench] DoT" << ip << "failed";
    return -1.0;
}
