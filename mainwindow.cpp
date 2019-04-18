#include "mainwindow.h"
#include "ui_mainwindow.h"

static QByteArray test(QByteArray test1, uint32_t data){
    test1.append((data & 0xFF000000) >> 24);
    test1.append((data & 0x00FF0000) >> 16);
    test1.append((data & 0x0000FF00) >> 8);
    test1.append((data & 0x000000FF) >> 0);

    return test1;
}

static uint32_t FloatToBytes(float n){
    return static_cast<uint32_t>(*(uint32_t*)&n);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->MessageHandler = new MessageProcessor(this);

    connect(this->MessageHandler, SIGNAL(NewSeriesReceived(int,QString)), this, SLOT(AddReceivedSeries(int,QString)));
    connect(this->MessageHandler, SIGNAL(DataPointsReceived(int,QList<float>)), this, SLOT(AddDataPoints(int, QList<float>)));

    QByteArray Message;
    Message.append(0x01);
    Message = test(Message, 0xFF);
    Message = test(Message, 'T');
    Message = test(Message, 'e');
    Message = test(Message, 's');
    Message = test(Message, 't');
    Message.append(0x0A);

    this->MessageHandler->AddBytesToBuffer(Message);

    Message.clear();
    Message.append(0x03);
    Message = test(Message, 0xFF);
    Message = test(Message, FloatToBytes(1.0));
    Message = test(Message, FloatToBytes(1.5));
    Message = test(Message, FloatToBytes(2.0));
    Message.append(0x11);
    Message.append(0x13);
    Message.append(0x12);

    this->MessageHandler->AddBytesToBuffer(Message);

    Message.clear();
    Message.append(0x03);
    Message = test(Message, 0xFF);
    Message = test(Message, FloatToBytes(1.0));
    Message = test(Message, FloatToBytes(1.5));
    Message = test(Message, FloatToBytes(2.0));
    this->MessageHandler->AddBytesToBuffer(Message);

    //Add a blank chart to keep the view from being empty
    //This is not required, but makes the interface look nicer
    this->AddChart("Chart1");

    this->ComPort = new QSerialPort(this);
    //Unsure why, but this needs to be connected like so
    connect(this->ComPort, &QSerialPort::readyRead, this, &MainWindow::SerialDataAvailable);
    //Refresh list of available com ports
    this->on_RefreshButton_released();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::AddTab(QString TabName)
{
    Chart *ChartWidget = new Chart();
    this->ui->GraphTabs->addTab(ChartWidget, TabName);
    this->Charts.append(ChartWidget);
    ChartWidget->SetChartName(TabName);

    //Add all of the current data series to the chart
    for(int i = 0; i < this->DataSeries.size(); i++){
        ChartWidget->AddDataSeries(this->DataSeries[i]->name(), this->DataSeries[i]);
    }

    //Connect the chart's signals to the appropriate functions
    connect(ChartWidget, SIGNAL(Delete(Chart*)), this, SLOT(DeleteChart(Chart*)));
}

QLineSeries* MainWindow::AddSeries(QString SeriesName)
{
    //Create the data series and name it
    QLineSeries *Series = new QLineSeries();
    Series->setName(SeriesName);

    //Add the series to every chart
    for(int i = 0; i < this->Charts.size(); i++){
        this->Charts[i]->AddDataSeries(SeriesName, Series);
    }

    //Add the series to this class
    this->DataSeries.append(Series);

    return Series;
}

void MainWindow::AddChart(QString ChartName)
{
    this->AddTab(ChartName);
}

//Returns the index of the chart found at the pointer
//  Or returns -1 if the chart was not found
int MainWindow::GetChartIndex(Chart *ChartPointer)
{
    int Index = -1;

    for(int i = 0; i < this->Charts.size(); i++){
        if(this->Charts[i] == ChartPointer){
            Index = i;
            break;
        }
    }

    return Index;
}

void MainWindow::DeleteChart(Chart *ChartPointer)
{
    //Get the index of the chart being deleted
    int Index = GetChartIndex(ChartPointer);

    //Remove the tab from the UI
    this->ui->GraphTabs->removeTab(Index);

    //Remove the data from this object
    this->Charts.removeAt(Index);

    //Delete the chart
    delete ChartPointer;
}

void MainWindow::AddReceivedSeries(int SeriesID, QString SeriesName)
{
    this->SeriesIDs.append(SeriesID);
    this->AddSeries(SeriesName);
}

void MainWindow::AddDataPoints(int SeriesID, QList<float> DataPoints)
{
    int Index = GetIndexFromID(SeriesID);

    if(Index != -1){
        for(int i = 0; i < DataPoints.size(); i++){
            double Data = static_cast<double>(DataPoints[i]);
            int x = DataSeries[Index]->count() + 1;
            DataSeries[Index]->append(x,Data);
        }
    }
    else{
        qDebug() << "Data received for " << SeriesID << " which does not exist";
    }
}

void MainWindow::on_TerminalEntry_returnPressed()
{
    QString Input = this->ui->TerminalEntry->text();
    this->ui->Terminal->addItem(Input);
    this->ui->TerminalEntry->clear();
    this->ui->Terminal->scrollToBottom();
}

void MainWindow::on_AddChartButton_released()
{
    this->AddChart(this->ui->ChartNameInput->text());
}

int MainWindow::GetIndexFromID(int SeriesID)
{
    int Index = -1;

    for(int i = 0; i < this->SeriesIDs.size(); i++){
        if(this->SeriesIDs[i] == SeriesID){
            return i;
        }
    }

    return Index;
}

QString MainWindow::GetPortName(bool &Ok)
{
    QString PortName = this->ui->ComPort->currentText();

    if(PortName.isEmpty()){
        Ok = false;
    }

    return PortName;
}

int32_t MainWindow::GetBaudRate(bool &Ok)
{
    bool ConversionGood = false;
    int32_t BaudRate = this->ui->BaudRate->text().toInt(&ConversionGood);

    //If the conversion failed, or if Ok was previously false, set Ok to false.
    Ok = Ok & ConversionGood;

    return BaudRate;
}

QSerialPort::DataBits MainWindow::GetDataBits(bool &Ok)
{
    bool ConversionGood = false;
    QSerialPort::DataBits databits;
    uint32_t DataBitsAsInt = this->ui->DataBits->text().toUInt(&ConversionGood);
    switch(DataBitsAsInt){
        case 5:
            databits = QSerialPort::Data5;
            break;
        case 6:
            databits = QSerialPort::Data6;
            break;
        case 7:
            databits = QSerialPort::Data7;
            break;
        case 8:
            databits = QSerialPort::Data5;
            break;
        default:
            //Default to 8 bits on error
            databits = QSerialPort::Data8;
            ConversionGood = false;
            break;
    }

    Ok = Ok & ConversionGood;
    return databits;
}

QSerialPort::Parity MainWindow::GetParity(bool &Ok)
{
    bool ConversionGood = true;
    QSerialPort::Parity parity;
    int ParityComboIndex = this->ui->Parity->currentIndex();
    switch(ParityComboIndex){
        case 0:
        //none
            parity = QSerialPort::NoParity;
            break;
        case 1:
        //Odd
            parity = QSerialPort::OddParity;
            break;
        case 2:
        //Even
            parity = QSerialPort::EvenParity;
            break;
        case 3:
        //Mark
            parity = QSerialPort::MarkParity;
            break;
        case 4:
            parity = QSerialPort::SpaceParity;
            //Space
            break;
        default:
            //Default to no parity on error
            parity = QSerialPort::NoParity;
            ConversionGood = false;
            break;
    }

    Ok = Ok & ConversionGood;
    return parity;
}

QSerialPort::StopBits MainWindow::GetStopBits(bool &Ok)
{
    bool ConversionGood = false;
    QSerialPort::StopBits stopbits;
    uint32_t StopBitsInt = this->ui->StopBits->text().toUInt(&ConversionGood);
    switch(StopBitsInt){
        case 1:
        //none
            stopbits = QSerialPort::OneStop;
            break;
        case 2:
        //Even
            stopbits = QSerialPort::TwoStop;
            break;
        default:
            //Default to one stop on error
            stopbits = QSerialPort::OneStop;
            ConversionGood = false;
            break;
    }

    Ok = Ok & ConversionGood;
    return stopbits;
}

void MainWindow::on_ConnectButton_released()
{
    if(this->ComPort->isOpen()){
        //Disconnect
        this->ComPort->close();
        this->ui->ConnectButton->setText("Connect");
        this->ui->StatusLabel->setText("Disconnected");
    }
    else{
        bool Ok = true;
        QString PortName = this->GetPortName(Ok);
        int32_t BaudRate = this->GetBaudRate(Ok);
        QSerialPort::DataBits bits = this->GetDataBits(Ok);
        QSerialPort::Parity parity = this->GetParity(Ok);
        QSerialPort::StopBits stopbits = this->GetStopBits(Ok);
        QSerialPort::FlowControl flowcontrol = QSerialPort::NoFlowControl;
        if(Ok){
            this->ComPort->setPortName(PortName);
            this->ComPort->setBaudRate(BaudRate);
            this->ComPort->setDataBits(bits);
            this->ComPort->setParity(parity);
            this->ComPort->setStopBits(stopbits);
            this->ComPort->setFlowControl(flowcontrol);

            bool ConnectSuccessfull = this->ComPort->open(QIODevice::ReadWrite);

            if(ConnectSuccessfull){
                this->ui->ConnectButton->setText("Disconnect");
                this->ui->StatusLabel->setText("Connected to " + PortName);
            }
            else{
                this->ui->StatusLabel->setText("Failed to connect to " + PortName);
            }
        }
        else{
            qDebug() << "Failed to convert serial settings.";
            this->ui->StatusLabel->setText("Invalid COM Settings");
        }
    }
}

void MainWindow::on_RefreshButton_released()
{
    this->ui->ComPort->clear();
    //Refresh the list of valid com ports
    for(QSerialPortInfo PortInfo : QSerialPortInfo::availablePorts()){
        this->ui->ComPort->addItem(PortInfo.portName());
    }
}

void MainWindow::SerialDataAvailable()
{
    while(this->ComPort->bytesAvailable()){
        char Data[1];
        this->ComPort->read(Data, 1);
        qDebug() << static_cast<int>(Data[0]);
    }
}
