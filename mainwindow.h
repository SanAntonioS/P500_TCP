#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))
#define Search 1
#define Control 2
#define Stop 3

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void        DataProess(int Function);
    void        readData(QByteArray data);
    uchar       Xor(uchar *buf, int len);
    static int  strHexToDecimal(const QString &strHex);

    QTcpServer  *tcpServer;
    QTcpSocket  *tcpSocket;
    QTimer      *timer;
    QByteArray  messageSend;
    QByteArray  messageRecv;
private:
    bool isOk;

private slots:

    void newConnection_Slot();

    void readyRead_Slot();

    void on_btnListen_clicked();

    void on_btnSearch_clicked();

    void on_btnRun_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
