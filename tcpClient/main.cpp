#include <QCoreApplication>
#include "tcpclient.h"
#include <QTimer>

int main(int argc, char *argv[])
{


    QCoreApplication a(argc, argv);
        TcpClient client;

//        client.sendNumber();
    return a.exec();
}
