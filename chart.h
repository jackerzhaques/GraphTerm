#ifndef CHART_H
#define CHART_H

#include <QWidget>
#include <QLineSeries>
#include <QChart>
#include <QChartView>
#include <QList>
#include <QString>
#include <QCheckBox>

#include <QDebug>

using namespace QtCharts;

namespace Ui {
class Chart;
}

class Chart : public QWidget
{
    Q_OBJECT

public:
    explicit Chart(QWidget *parent = nullptr);
    ~Chart();

public slots:
    void AddDataSeries(QString SeriesName, QLineSeries* Series);
    void ShowSeriesCheckToggled();
    bool GetCheckboxState(QString SeriesName);
    void SetChartName(QString Name);

private slots:
    void on_DeleteButton_released();

signals:
    void Delete(Chart* ChartPointer);

private:
    Ui::Chart *ui;
    QChartView         *ChartView = nullptr;
    QChart             *ChartPointer = nullptr;
    QList<QLineSeries*> DataSeries;
    QList<QString>      DataSeriesLabels;
    QList<QCheckBox*>   DataSeriesCheckbox;
    QList<bool>         DataSeriesCheckState;
};

#endif // CHART_H
