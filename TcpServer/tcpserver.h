#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QThread>
#include <QMutex>

extern QMutex mutex;
extern QMutex mutexD;

struct Message{
  uint idClient = 0;
  int size = 0;
  QByteArray data;
  QJsonDocument jMessage;
};

struct ClientInfo{
  uint id;
  QTime timeReceive;
  QJsonDocument jsonData;
  bool status =true;
};


class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

    QMap<QTcpSocket*, uint>* getClientInfo() {return &clientId_;}
    std::list<ClientInfo>* getClientInfoList() {return &lClientInfo_;}

    QVector<QString> debug_;

    bool startRecieve_ = false;
    double metricsRestruction;

public slots:

    void newSocketConnection();
    void onReadyRead();
    void onDisconnected();

    void startServer();

    void startClients();

signals:
        void displayConnectionStatus();

private:

    void startServer(int port);
    void sendStartStop2Client(QTcpSocket* client);
    QTcpServer* tcpServer_;
    std::list<ClientInfo> lClientInfo_;
    QMap<QTcpSocket*, Message> socketBuffer_;
    QMap<QTcpSocket*, uint> clientId_;

    void makeJSONanswer2client(QTcpSocket* client, QString message) const;

    uint idCounter = 0;


};

#endif // TCPSERVER_H
