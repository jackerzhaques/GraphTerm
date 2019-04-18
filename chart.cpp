#include "chart.h"
#include "ui_chart.h"

Chart::Chart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chart)
{
    ui->setupUi(this);

    this->ChartPointer = new QChart();
    this->ChartPointer->createDefaultAxes();
    this->ChartView = new QChartView(this->ChartPointer);
    this->ChartView->setRenderHint(QPainter::Antialiasing);
    this->ui->ChartContainer->layout()->addWidget(this->ChartView);
    this->ChartPointer->legend()->setAlignment(Qt::AlignRight);
}

Chart::~Chart()
{
    delete ui;
}

void Chart::AddDataSeries(QString SeriesName, QLineSeries *Series)
{
    //Name the data series, in case the user didn't.
    Series->setName(SeriesName);

    //Create the checkbox and add it to the ui
    QCheckBox *cb = new QCheckBox(this);
    cb->setText(SeriesName);
    QVBoxLayout *Layout = dynamic_cast<QVBoxLayout*>(this->ui->DataSeriesContainer->layout());
    Layout->insertWidget(1,cb);

    //Save the input data
    this->DataSeriesLabels.append(SeriesName);
    this->DataSeries.append(Series);
    this->DataSeriesCheckState.append(false);

    //Connect the checkbox slot last so no events fire while data is still being added
    connect(cb, SIGNAL(stateChanged(int)), this, SLOT(ShowSeriesCheckToggled()));
}

void Chart::ShowSeriesCheckToggled()
{
    //Check each checkbox, add the series if it is checked, remove if not
    for(int i = 0; i < this->DataSeries.size(); i++){
        bool State = this->GetCheckboxState(this->DataSeriesLabels[i]);

        //Only update if the state changed
        if(State != this->DataSeriesCheckState[i]){
            if(State){
                this->ChartPointer->addSeries(this->DataSeries[i]);
            }
            else{
                this->ChartPointer->removeSeries(this->DataSeries[i]);
            }
        }

        //Save the new state
        this->DataSeriesCheckState[i] = State;
    }


    this->ChartPointer->createDefaultAxes();
}

bool Chart::GetCheckboxState(QString SeriesName)
{
    bool State = false;

    //Iterate starting past the title label, and stopping before the vertical spacer.
    //NOTE: If these widgets are deleted this function will most likely crash the program.
    for(int i = 1; i < this->ui->DataSeriesContainer->layout()->count() - 2; i++){
        QCheckBox *Box = dynamic_cast<QCheckBox*>(this->ui->DataSeriesContainer->layout()->itemAt(i)->widget());
        if(Box){
            if(Box->text() == SeriesName){
                State = Box->isChecked();
                break;
            }
        }
        else{
            qDebug() << "Error grabbing QCheckBox. Invalid index range. Chart::GetCheckBoxState";
        }
    }
    return State;
}

void Chart::SetChartName(QString Name)
{
    this->ChartPointer->setTitle(Name);
}

void Chart::on_DeleteButton_released()
{
    emit(Delete(this));
}
