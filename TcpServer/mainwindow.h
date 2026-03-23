#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tcpserver.h"
#include <QThread>
#include <QPair>
#include <QTableWidgetItem>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

extern QMutex mutex;
extern QMutex mutexD;

struct clientSettings{

};


class  MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(TcpServer* tcp, QMap<QTcpSocket*, uint>* id, std::list<ClientInfo>* lclientsInfo_, QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void fillTableStatus();
    void fillCommonTable();
    void doFillinfo();

signals:

    void startClients();


private slots:
    void on_startClient_clicked();

    void on_pbMetrics_clicked();

private:
    QTimer* timer_;

    QMap<QTcpSocket*, uint>* clientsId_;
    std::list<ClientInfo>* clientsInfoList_;

    void fillDebug();

    bool* startClients_;
    double* metricsRestruction;

    QVector<QString>* debug_;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
