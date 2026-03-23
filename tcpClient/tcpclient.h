#ifndef TCPCLIENT_H
#define TCPCLIENT_H



#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>
#include <QRandomGenerator>
#include <QHostAddress>

#include <sstream>


class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    QTimer* timer_;
public slots:
    void sendJSON(const QJsonDocument Js);
    void sendAllJSON();

    void connect2server();
    void onReadyRead();

private:
    QTcpSocket * tcpClient_;

    bool checkConnection();
    void reconnect2Server();

    QJsonDocument makeNetworkMetricksOnJSON();
    QJsonDocument makeDeviceStatusOnJSON() const;
    QJsonDocument makeLogOnJSON() const;

    void makeInterval4Send();
    QTimer* reconnectTimer_;

    int time4send_ = 10;
    bool serverPermition4send_ = false;
    double metricsRestrucrion_;
    double bandwidth_;

};

#endif // TCPCLIENT_H
