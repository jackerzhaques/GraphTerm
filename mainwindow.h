#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QTextCursor>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QTimer>
#include <QTabWidget>
#include <QLayout>
#include <QVBoxLayout>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "chart.h"
#include "messageprocessor.h"

using namespace QtCharts;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //Chart creation and management
    void AddTab(QString TabName);
    QLineSeries* AddSeries(QString SeriesName);
    void AddChart(QString ChartName);
    int GetChartIndex(Chart* ChartPointer);


public slots:
    void DeleteChart(Chart* ChartPointer);
    void AddReceivedSeries(int SeriesID, QString SeriesName);
    void AddDataPoints(int SeriesID, QList<float> DataPoints);

private slots:

    void on_TerminalEntry_returnPressed();

    void on_AddChartButton_released();

    void on_ConnectButton_released();

    void on_RefreshButton_released();

    void SerialDataAvailable();

private:
    Ui::MainWindow *ui;
    QString OldTerminalText;
    QList<Chart*> Charts;
    QList<QLineSeries*> DataSeries;
    QList<int> SeriesIDs;
    MessageProcessor *MessageHandler = nullptr;
    int GetIndexFromID(int SeriesID);
    QSerialPort *ComPort;

    QString GetPortName(bool &Ok);
    int32_t GetBaudRate(bool &Ok);
    QSerialPort::DataBits GetDataBits(bool &Ok);
    QSerialPort::Parity GetParity(bool &Ok);
    QSerialPort::StopBits GetStopBits(bool &Ok);

    QByteArray SerialBuffer;
};

#endif // MAINWINDOW_H
