#include "mainwindow.h"
#include "tcpserver.h"

#include <QApplication>
#include <QThread>

QMutex mutex;
QMutex mutexD;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug()<<"MainWindow Thread = "<<QThread::currentThreadId();
    QThread* tr = new QThread;

    TcpServer* server = new TcpServer();
    QMap<QTcpSocket*, uint>* clInfo = server->getClientInfo();
    std::list<ClientInfo>* clInfoList = server->getClientInfoList();
    server->moveToThread(tr);
    QObject::connect(tr, SIGNAL(started()), server, SLOT(startServer()), Qt::AutoConnection);
    tr->start();

    MainWindow w(server, clInfo, clInfoList);
    w.show();
    return a.exec();
}
