#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isOk = false;

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection_Slot()));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_btnSearch_clicked()));

    on_btnListen_clicked();
}

MainWindow::~MainWindow()
{
    tcpServer->close();
    delete ui;
}

/// @brief 异或校验
/// @param[in]  buf[]   参与异或校验的全部buf
/// @param[in]  len     buf[]的长度
/// @return     Xor     异或校验值
uchar MainWindow::Xor(uchar buf[], int len)
{
    uchar CRC = 0;
    for ( int i = 0 ; i < len ; i ++ ) {
       CRC = CRC ^ buf[i];
    }
    return CRC;
}

int MainWindow::strHexToDecimal(const QString &strHex)
{
    bool ok;
    return strHex.toInt(&ok, 16);
}

/// @brief 按照用户点击的按钮来处理即将发送到P500+的数据包
/// @param[in]  Function    用户选择的功能
/// @param[out] messageSend 要发送给P500+的数据包
void MainWindow::DataProess(int Function)
{
    messageSend = 0;
    switch (Function) {
    case Search:{
        uchar buf[] = {0xff, 0x11, 0x05, 0x01, 0xea};
        for (uint i=0;i < sizeof (buf);i++) {
            messageSend.append(buf[i]);
        }
    }break;
    case Control:{
        uchar buf[13] = {0xff, 0x11, 0x0d, 0x02};
        uint FlowRate = ui->txtFlowRate->text().toInt() *10;
        buf[4] = (FlowRate >> 8) & 0xff;
        buf[5] = (FlowRate >> 0) & 0xff;
        uint MaxPress = ui->txtMaxPress->text().toInt() *10;
        buf[6] = (MaxPress >> 8) & 0xff;
        buf[7] = (MaxPress >> 0) & 0xff;
        uint MinPress = ui->txtMinPress->text().toInt() *10;
        buf[8] = (MinPress >> 8) & 0xff;
        buf[9] = (MinPress >> 0) & 0xff;
        buf[10] = 0x01;
        buf[11] = 0x00;
        buf[12] = Xor(buf,sizeof (buf));
        for (uint i=0;i < sizeof (buf);i++) {
            messageSend.append(buf[i]);
        }
    }break;
    case Stop:{
        uchar buf[13] = {0xff, 0x11, 0x0d, 0x02};
        uint FlowRate = ui->txtFlowRate->text().toInt() *10;
        buf[4] = (FlowRate >> 8) & 0xff;
        buf[5] = (FlowRate >> 0) & 0xff;
        uint MaxPress = ui->txtMaxPress->text().toInt() *10;
        buf[6] = (MaxPress >> 8) & 0xff;
        buf[7] = (MaxPress >> 0) & 0xff;
        uint MinPress = ui->txtMinPress->text().toInt() *10;
        buf[8] = (MinPress >> 8) & 0xff;
        buf[9] = (MinPress >> 0) & 0xff;
        buf[10] = 0x00;
        buf[11] = 0x00;
        buf[12] = Xor(buf,sizeof (buf));
        for (uint i=0;i < sizeof (buf);i++) {
            messageSend.append(buf[i]);
        }
    }break;
    default:break;
    }
}

void MainWindow::readData(QByteArray data)
{
    QByteArray temp;
    QString strHex;
    data.resize(15);
    if(data.at(0) == '\xff' && data.at(1) == '\x01')
    {
        if(data.at(4) == '\x00')
            ui->labStatus->setText("停止");
        else
            ui->labStatus->setText("运行");
        uint Pressure = strHexToDecimal(data.toHex().left(10+4).right(4)) / 10;
        uint Flowrate = strHexToDecimal(data.toHex().left(10+4+4).right(4)) / 10;
        uint MaxPressure = strHexToDecimal(data.toHex().left(10+4+4+4).right(4)) / 10;
        uint MinPressure = strHexToDecimal(data.toHex().left(10+4+4+4+4).right(4)) / 10;
        ui->labPress->setText(QString::number(Pressure) + " MPa");
        ui->labFlowrate->setText(QString::number(Flowrate) + " ml/min");
        ui->labMaxpress->setText(QString::number(MaxPressure) + " MPa");
        ui->labMinpress->setText(QString::number(MinPressure) + " MPa");
    }
}

void MainWindow::newConnection_Slot()
{
    tcpSocket = tcpServer->nextPendingConnection();

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead_Slot()));
}


void MainWindow::readyRead_Slot()
{
    QString buf;
    QString port = ui->txtPort->text();
    messageRecv = tcpSocket->readAll();
}

void MainWindow::on_btnListen_clicked()
{
    if (ui->btnListen->text() == "监听") {
        isOk = tcpServer->listen(QHostAddress::Any, ui->txtPort->text().toUInt());
        if (isOk) {
            ui->btnListen->setText("关闭");
            qDebug()<<"监听成功";
        } else {
            qDebug()<<"监听失败";
        }
    } else {
        isOk = false;
        tcpServer->close();
        ui->btnListen->setText("监听");
    }
}

void MainWindow::on_btnSearch_clicked()
{
    readData(messageRecv);
    DataProess(Search);
    tcpSocket->write(messageSend);
    timer->start(1000);
    ui->btnSearch->setText("查询中…");
}

void MainWindow::on_btnRun_clicked()
{
    if (ui->btnRun->text() == "运行")
    {
        DataProess(Control);
        tcpSocket->write(messageSend);
        ui->btnRun->setText("停止");
    }
    else
    {
        DataProess(Stop);
        tcpSocket->write(messageSend);
        ui->btnRun->setText("运行");
    }
}
