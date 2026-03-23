#include "tcpclient.h"

TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
    tcpClient_ = new QTcpSocket(this);

    timer_ = new QTimer(this); //таймер для отправки JSON данных
    makeInterval4Send(); //устанавливаем интервал таймера для отпавки JSON данных
    reconnectTimer_ = new QTimer(this); //таймер для переподключения к серверу
    reconnectTimer_->setInterval(5000);

    connect(timer_, SIGNAL(timeout()), this, SLOT(sendAllJSON()), Qt::AutoConnection); //отправка данных JSON по таймеру
    connect(reconnectTimer_, SIGNAL(timeout()), this, SLOT(connect2server()), Qt::AutoConnection); // попытки соедениться с сервевром по таймеру
    connect(tcpClient_, SIGNAL(readyRead()),this, SLOT(onReadyRead()), Qt::AutoConnection); //когда в сокете появляются данные считываем данные

    connect2server();
}

bool TcpClient::checkConnection(){
    return QTcpSocket::ConnectedState == tcpClient_->state();
}

void TcpClient::makeInterval4Send(){
    QRandomGenerator* rand = QRandomGenerator::global();
    time4send_ = rand->bounded(10, 100);
    timer_->setInterval(time4send_);
}


void TcpClient::connect2server(){
    if(checkConnection()){
        reconnectTimer_->stop();
        makeInterval4Send();
        if(serverPermition4send_){
            timer_->start();
        }
        return;
    }

    tcpClient_->connectToHost(QHostAddress::LocalHost, 12345);

    if(checkConnection()){

        reconnectTimer_->stop();
        makeInterval4Send();
        if(serverPermition4send_){
            timer_->start();
        }
        qDebug()<<"Connecting Established!!!";
    } else {
        timer_->stop();
        reconnectTimer_->start();
    }
}

void TcpClient::reconnect2Server(){
    timer_->stop();
    connect2server();
}

void TcpClient::sendAllJSON(){
    if(!checkConnection()) {
        timer_->stop();
        connect2server();
        return;
    }

    sendJSON(makeNetworkMetricksOnJSON());
    sendJSON(makeDeviceStatusOnJSON());
    sendJSON(makeLogOnJSON());

    makeInterval4Send();
    timer_->start();
}

void TcpClient::sendJSON(const QJsonDocument Js) {
    QByteArray data = Js.toJson();
    QByteArray message;
    int sizeMessage = static_cast<int>(data.size());
    qDebug()<<"message size = "<<sizeMessage;

    message.append(reinterpret_cast<char*>(&sizeMessage), sizeof (int));
    message.append(data);
    tcpClient_->write(message);
}

QJsonDocument TcpClient::makeNetworkMetricksOnJSON() {
    QRandomGenerator* rand = QRandomGenerator::global();
    QJsonObject networkMetrics;
        bandwidth_ = rand->bounded(150.0); //100.5;
        networkMetrics["type"] = "NetworkMetrics";
        networkMetrics["bandwidth"] = bandwidth_;
        networkMetrics["latency"] = rand->bounded(20); // 12.3;
        networkMetrics["packet_loss"] = rand->generateDouble(); // 0.1;
    QJsonDocument jsDoc(networkMetrics);
    return jsDoc;
}

QJsonDocument TcpClient::makeDeviceStatusOnJSON() const{
    QRandomGenerator* rand = QRandomGenerator::global();
    QJsonObject deviceStatus;
        deviceStatus["type"] = "DeviceStatus";
        deviceStatus["uptime"] = rand->bounded(5000.0); //3600;
        deviceStatus["cpu_usage"] = rand->bounded(101.0); //25;
        deviceStatus["memory_usage"] = rand->bounded(100.0); //60;
    QJsonDocument jsDoc(deviceStatus);
    return jsDoc;
}

QJsonDocument TcpClient::makeLogOnJSON() const{
    QJsonObject log;
        log["type"]= "log";
        if(bandwidth_> metricsRestrucrion_){
           log["message"]= "Interface eth0 restarted";
        } else {
            log["message"]="attantion bandwidth low";
        }

        log["severity"] = "INFO";
    QJsonDocument jsDoc(log);
    return jsDoc;
}

void TcpClient::onReadyRead(){
    QByteArray data = tcpClient_->readAll();
    QByteArray buf = data.left(sizeof(int));
    data.remove(0, sizeof(int));
    QJsonDocument jbuf;
    jbuf = QJsonDocument::fromJson(data);
    if(jbuf.isObject()){
         QJsonObject jobj = jbuf.object();
         serverPermition4send_ = jobj["serverSendPermition"].toBool();
         metricsRestrucrion_ = jobj["metricsRestruction"].toDouble();
         qDebug()<<"Server Report = "<<jobj["serverReport"].toString()<<metricsRestrucrion_;
         if(serverPermition4send_){
             connect2server();
         } else {
             timer_->stop();
         }
    }


}
