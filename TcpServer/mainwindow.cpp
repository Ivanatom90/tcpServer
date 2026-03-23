#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(TcpServer* tcp, QMap<QTcpSocket*, uint>* id, std::list<ClientInfo>* lclient, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      clientsId_(id),
      clientsInfoList_(lclient),
      startClients_(&tcp->startRecieve_),
      metricsRestruction(&tcp->metricsRestruction)
{
    ui->setupUi(this);
    debug_ = &tcp->debug_;
    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_,SIGNAL(timeout()),this,SLOT(doFillinfo()), Qt::AutoConnection);
    connect(this, SIGNAL(startClients()), tcp, SLOT(startClients()), Qt::AutoConnection);
    connect(tcp,SIGNAL(displayConnectionStatus()),this,SLOT(fillTableStatus()),Qt::AutoConnection);
    timer_->start(100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fillTableStatus(){
    timer_->start();
    if(clientsId_->empty()) return;

    ui->tableWidgetStat->setColumnCount(4);
    ui->tableWidgetStat->setRowCount(clientsId_->count()+1);


    if(!ui->tableWidgetStat->item(0, 0)){
        ui->tableWidgetStat->setItem(0, 0, new QTableWidgetItem("Id"));
        ui->tableWidgetStat->setItem(0, 1, new QTableWidgetItem("Ip"));
        ui->tableWidgetStat->setItem(0, 2, new QTableWidgetItem("Port"));
        ui->tableWidgetStat->setItem(0, 3, new QTableWidgetItem("Status"));
    }
    try {


    int i = 1;
    for(const std::pair<QTcpSocket*, uint>& client:clientsId_->toStdMap()){
        int j = 0;
        if(ui->tableWidgetStat->item(i,j)){
            ui->tableWidgetStat->item(i,j++)->setText(QString::number( client.second));
            ui->tableWidgetStat->item(i,j++)->setText(client.first->peerAddress().toString());
            ui->tableWidgetStat->item(i,j++)->setText(QString::number(client.first->peerPort()));
            ui->tableWidgetStat->item(i,j++)->setText(QString((client.first->ConnectedState) ? "Connected" : "Disconnected"));

        } else {
            ui->tableWidgetStat->setItem(i,j++, new QTableWidgetItem(QString::number( client.second)));
            ui->tableWidgetStat->setItem(i,j++, new QTableWidgetItem(client.first->peerAddress().toString()));
            ui->tableWidgetStat->setItem(i,j++, new QTableWidgetItem(QString::number(client.first->peerPort())));
            ui->tableWidgetStat->setItem(i,j++, new QTableWidgetItem((client.first->ConnectingState) ? "Connected" : "Disconnected"));
        }
        i++;

    }
    }  catch (...) {
        debug_->append("Client info error");
    }

}

void MainWindow::fillCommonTable(){
    if(!clientsInfoList_->size()){
        return;
    }
    static bool firstRow = true;

    try {

    if(firstRow){
        ui->tableWidgetMessege->setRowCount(2);
        ui->tableWidgetMessege->setColumnCount(6);
        ui->tableWidgetMessege->setItem(0,0, new QTableWidgetItem("Id"));
        ui->tableWidgetMessege->setItem(0,1, new QTableWidgetItem("Data Type"));
        firstRow = false;
    }
    int row = ui->tableWidgetMessege->rowCount();
    int i =  row-1;

    QMutexLocker locker(&mutex);
    for(std::list<ClientInfo>::reverse_iterator client = clientsInfoList_->rbegin(); client != clientsInfoList_->rend(); client++){
        int j = 0;
        ui->tableWidgetMessege->setRowCount(++row);

            for(int c = 0; c< ui->tableWidgetMessege->columnCount(); c++){
                 ui->tableWidgetMessege->setItem(i, c, new QTableWidgetItem());
            }

            ui->tableWidgetMessege->item(i,j++)->setText(QString::number(client->id));
            if(client->jsonData.isObject()){
                QJsonObject jobj = client->jsonData.object();
                if(jobj["type"].toString() == "NetworkMetrics"){
                    ui->tableWidgetMessege->item(i,j++)->setText(jobj["type"].toString());
                    ui->tableWidgetMessege->item(i,j++)->setText(QString::number(jobj["bandwidth"].toDouble()));
                    ui->tableWidgetMessege->item(i,j++)->setText(QString::number(jobj["latency"].toDouble()));
                    ui->tableWidgetMessege->item(i,j++)->setText(QString::number(jobj["packet_loss"].toDouble()));
                } else if(jobj["type"].toString() == "DeviceStatus") {
                    ui->tableWidgetMessege->item(i,j++)->setText(jobj["type"].toString());
                    ui->tableWidgetMessege->item(i,j++)->setText(QString::number(jobj["uptime"].toDouble()));
                    ui->tableWidgetMessege->item(i,j++)->setText(QString::number(jobj["cpu_usage"].toDouble()));
                    ui->tableWidgetMessege->item(i,j++)->setText(QString::number(jobj["memory_usage"].toDouble()));
                } else {
                    ui->tableWidgetMessege->item(i,j++)->setText(jobj["type"].toString());
                    ui->tableWidgetMessege->item(i,j++)->setText((jobj["message"].toString()));
                    ui->tableWidgetMessege->item(i,j++)->setText((jobj["severity"].toString()));
                }
                QTime time = client->timeReceive;
                if(time.isValid()){
                     ui->tableWidgetMessege->item(i,5)->setText(client->timeReceive.toString("hh:mm:ss:zz"));
                }

            }             
        i++;
    }
    clientsInfoList_->clear();
    }  catch (...) {
        qDebug()<<"fillTableError";
        return;
    }
}

void MainWindow::doFillinfo(){
        fillTableStatus();
        fillCommonTable();
        fillDebug();
}

void MainWindow::fillDebug(){
    if(debug_->empty()){
        return;
    }

    QMutexLocker locker(&mutexD);
    for(const QString& info:(*debug_)){
        ui->textEdit4Logs->append(info);
    }
    debug_->clear();
}


void MainWindow::on_startClient_clicked()
{
    *startClients_ = !(*startClients_);
    *metricsRestruction = ui->leMetricsRestruct->text().toDouble();
    (*startClients_) ? ui->startClient->setText("Stop") : ui->startClient->setText("Start");

    emit startClients();
}

void MainWindow::on_pbMetrics_clicked()
{
    *metricsRestruction = ui->leMetricsRestruct->text().toDouble();
    emit startClients();
}
