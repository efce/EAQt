#include "eaqtarplsbackgroundcorrectiondialog.h"
#include "ui_eaqtarplsbackgroundcorrectiondialog.h"
#include <QIntValidator>
#include <QDoubleValidator>
#include <math.h>

EAQTArplsBackgroundCorrectionDialog::EAQTArplsBackgroundCorrectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EAQTArplsBackgroundCorrectionDialog)
{ 
    ui->setupUi(this);

    // load current signals
    this->_signals = EAQtData::getInstance().getCurves();

    // default settings
    // Lambda
    ui->spinBoxLambda->setRange(this->minLambda,this->maxLambda);
    ui->spinBoxLambda->setSingleStep(10);
    ui->spinBoxLambda->setValue(this->defaultLambda);
    ui->horizontalSliderLambda->setRange(this->minLambda,this->maxLambda);
    ui->horizontalSliderLambda->setSingleStep(10);
    ui->horizontalSliderLambda->setValue(this->defaultLambda);
    connect(ui->horizontalSliderLambda,SIGNAL(valueChanged(int)),ui->spinBoxLambda,SLOT(setValue(int)));
    connect(ui->spinBoxLambda,SIGNAL(valueChanged(int)),ui->horizontalSliderLambda,SLOT(setValue(int)));
    // Ratio
    ui->doubleSpinBoxRatio->setRange(this->minRatio,this->maxRatio);
    ui->doubleSpinBoxRatio->setDecimals(abs(log10(this->minRatio)));
    ui->doubleSpinBoxRatio->setSingleStep(this->minRatio);
    ui->doubleSpinBoxRatio->setValue(this->defaultRatio);
    ui->horizontalSliderRatio->setRange(qPow(10,abs(log10(this->maxRatio))),qPow(10,abs(log10(this->minRatio))));
    ui->horizontalSliderRatio->setSingleStep(qPow(10,abs(log10(this->maxRatio))));
    ui->horizontalSliderRatio->setValue(qPow(10,abs(log10(this->defaultRatio))));
    //qDebug() << qPow(10,abs(log10(this->maxRatio))) << " " << qPow(10,abs(log10(this->minRatio)));
    connect(ui->horizontalSliderRatio,SIGNAL(valueChanged(int)),this,SLOT(setValueDoubleSpinBoxRatio(int)));
    connect(ui->doubleSpinBoxRatio,SIGNAL(valueChanged(double)),this,SLOT(setValueHorizontalSliderRatio(double)));
    // Widget Plot
    ui->widgetPlot->xAxis->setLabel("E / mV");
    ui->widgetPlot->yAxis->setLabel("i / µA");


    //this->plotSignals();
    this->showCurves();
}

EAQTArplsBackgroundCorrectionDialog::~EAQTArplsBackgroundCorrectionDialog()
{
    delete ui;
}

void EAQTArplsBackgroundCorrectionDialog::showCurves()
{
    //CurveCollection *cc = EAQtData::getInstance().getCurves();
    //qDebug() << cc->count() << "   " << EAQtData::getInstance().Act();

    cc3 = &cc1;
    qDebug() << "Start: " << cc1.count() << "  " << cc2.count() << "  " << cc3->count();
    cc2.unset(0);
    cc1.unset(0);
    cc1.unset(1);
    qDebug() << "Remove 1: " << cc1.count() << "  " << cc2.count() << "  " << cc3->count();
    cc2.unset(0);
    cc1.unset(0);
    qDebug() << "Remove 2: " << cc1.count() << "  " << cc2.count() << "  " << cc3->count();
    cc3->unset(1);
    qDebug() << "Remove 3: "  << cc1.count() << "  " << cc2.count() << "  " << cc3->count();
    //cc3->append(cc2.get(2));
    cc3->addNew(300);
    qDebug() << "Add 3: "  << cc1.count() << "  " << cc2.count() << "  " << cc3->count();

//    CurveCollection *cc1 = EAQtData::getInstance().getCurves();
//    CurveCollection *cc2 = EAQtData::getInstance().getCurves();
//    qDebug() << cc1->count() << "  " << cc2->count();
//    cc2->remove(0);
//    cc1->remove(0);
//    cc1->remove(1);
//    qDebug() << cc1->count() << "  " << cc2->count();
//    cc2->remove(0);
//    cc1->remove(0);
//    qDebug() << cc1->count() << "  " << cc2->count();

//    qDebug() << "test 1";
//    TYPES::vectorindex_t indx = this->_bkg.append(this->_signals->get(0));
//    qDebug() << indx;
    //this->plotSignals(this->_bkg);
}

void EAQTArplsBackgroundCorrectionDialog::plotSignals()
{
    Curve* curve;
    int i = 0;
    while((curve = _signals->get(i)) != nullptr )
    {
        ui->widgetPlot->addGraph();
        ui->widgetPlot->graph(i);
        ui->widgetPlot->graph(i)->setData(curve->getXVector(),curve->getYVector());
        ui->widgetPlot->graph(i)->setPen(QPen(COLOR::regular));
        ui->widgetPlot->graph(i)->rescaleAxes(true);
        i++;
    }
    ui->widgetPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->widgetPlot->replot();
}

void EAQTArplsBackgroundCorrectionDialog::plotSignals(CurveCollection *cc)
{
    Curve* curve;
    int i = 0;
    while((curve = cc->get(i)) != nullptr )
    {
        ui->widgetPlot->addGraph();
        ui->widgetPlot->graph(i);
        ui->widgetPlot->graph(i)->setData(curve->getXVector(),curve->getYVector());
        ui->widgetPlot->graph(i)->setPen(QPen(COLOR::regular));
        ui->widgetPlot->graph(i)->rescaleAxes(true);
        i++;
    }
    ui->widgetPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->widgetPlot->replot();
}

void EAQTArplsBackgroundCorrectionDialog::plotSignals(CurveCollection cc)
{
    Curve* curve;
    int i = 0;
    while((curve = cc.get(i)) != nullptr )
    {
        ui->widgetPlot->addGraph();
        ui->widgetPlot->graph(i);
        ui->widgetPlot->graph(i)->setData(curve->getXVector(),curve->getYVector());
        ui->widgetPlot->graph(i)->setPen(QPen(COLOR::regular));
        ui->widgetPlot->graph(i)->rescaleAxes(true);
        i++;
    }
    ui->widgetPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->widgetPlot->replot();
}

void EAQTArplsBackgroundCorrectionDialog::setValueDoubleSpinBoxRatio(int val)
{
    ui->doubleSpinBoxRatio->setValue(qPow(10,-log10(val)));
}

void EAQTArplsBackgroundCorrectionDialog::setValueHorizontalSliderRatio(double val)
{
    ui->horizontalSliderRatio->setValue(qPow(10,abs(log10(val))));
}



