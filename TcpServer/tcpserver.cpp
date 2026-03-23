#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QObject(parent)
{
    tcpServer_ = new QTcpServer(this);
    debug_.reserve(150);

}

void TcpServer::startServer(int port){
    connect(tcpServer_, SIGNAL(newConnection()), this, SLOT(newSocketConnection()) );

    if(tcpServer_->listen(QHostAddress::LocalHost, port) ){
        qDebug()<<"server started on port "<<port;
        debug_.push_back("server started on port " + QString::number(port));
    } else {
        qDebug()<<"start server failed";
        debug_<<"start server failed";
    }
}

void TcpServer::startServer(){
    startServer(12345);
    qDebug()<<"Server Thread = "<<QThread::currentThreadId();
}

void TcpServer::newSocketConnection(){
    while(tcpServer_->hasPendingConnections()){
        QTcpSocket* clientSocket = tcpServer_->nextPendingConnection();

        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(clientSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
//        clientsSockets_.push_front(clientSocket);
        clientId_[clientSocket] = idCounter++;
        makeJSONanswer2client(clientSocket,  "client connect to server");
        emit displayConnectionStatus();

        qDebug()<<"New connection esteblisheed";
        debug_<<"New connection esteblisheed"<<" address = " + clientSocket->peerAddress().toString() << " port = " + QString::number(clientSocket->peerPort());
    }
}

void TcpServer::onReadyRead(){
    static bool haveSize = false;    
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());

    QByteArray bufData = clientSocket->readAll();
    QMutexLocker locd(&mutexD);
    debug_<<"Client data recieve! " << "ip = " + clientSocket->peerAddress().toString()
          <<"port = " + QString::number(clientSocket->peerPort())
          <<"data size = " + QString::number(bufData.size());

    if(!socketBuffer_.contains(clientSocket)){
        socketBuffer_[clientSocket].idClient = clientId_[clientSocket];
    }

    socketBuffer_[clientSocket].data.append(bufData);
    bufData.clear();
    auto socBuf = socketBuffer_.find(clientSocket);

    int size = socBuf->data.size();

    if(!haveSize && socBuf->data.size() >= (int)sizeof (int)){
        memcpy(&socBuf->size, socBuf->data.data(), sizeof(int));
        haveSize = true;
    }

    if(!haveSize){
        return;
    }

    while(socBuf->data.size() >=  (int)socBuf->size + (int)sizeof(int) && haveSize){

            socBuf->data.remove(0, sizeof(int));
            bufData = socBuf->data.left(socketBuffer_[clientSocket].size);
            socBuf->data.remove(0, socketBuffer_[clientSocket].size);
            QJsonParseError parseError;
            QJsonDocument bufDoc = QJsonDocument::fromJson(bufData, &parseError);

            if(parseError.error != QJsonParseError::NoError){
                debug_<<"JSON parsing error: " + parseError.errorString();
                return;
            }

            if(bufDoc.isNull()){
                debug_ << "Document is null or empty";
               return;
            }

            if (!bufDoc.isObject()) {
                debug_ << "JSON is not object";
                return;
            }

            ClientInfo info;
            info.id = socBuf->idClient;
            info.jsonData = bufDoc;
            info.timeReceive = QTime::currentTime();

            if(socBuf->data.size() >= (int)sizeof(int)){
                memcpy(&socBuf->size, socBuf->data.data(), sizeof(int));
                haveSize = true;
            } else {
                socBuf->size = 0;
                haveSize = false;
            }
            QMutexLocker locker(&mutex);
            lClientInfo_.push_front(info);

    }

}

void TcpServer::onDisconnected(){
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if(!clientSocket) return;

    auto it = clientId_.find(clientSocket);
    clientId_.erase(it);
    auto it2 = socketBuffer_.find(clientSocket);
    socketBuffer_.erase(it2);
//    clientsSockets_.remove(clientSocket);
    debug_<<"Client disconnected"<<"IP address = " + clientSocket->peerAddress().toString()<<"port = " + QString::number(clientSocket->peerPort());
    clientSocket->deleteLater();
    qDebug()<<"Client disconnected";

}


void TcpServer::makeJSONanswer2client(QTcpSocket* client, QString message) const{
    QJsonObject jobj;
    jobj["serverReport"] = message;
    jobj["serverSendPermition"] = startRecieve_;
    jobj["metricsRestruction"] = metricsRestruction;
    QJsonDocument doc(jobj);
    int sizeDoc = sizeof(doc);
    QByteArray data;
    data.append(reinterpret_cast<char*>(&sizeDoc), sizeof (int));
    data.append(doc.toJson());
    client->write(data);
}

void TcpServer::startClients(){
//    startRecieve_ = !startRecieve_;
    for(std::pair<QTcpSocket*, uint> clients:clientId_.toStdMap()){
        makeJSONanswer2client(clients.first, "client can start sending");
    }
}
