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

    // main curves init
    this->_curves = EAQtData::getInstance().getCurves();

    // actual curve init
    this->_act = EAQtData::getInstance().Act();

    // data size init
    if(_act == SELECT::all)
    {
        this->_m = _curves->count();
    }
    else
    {
        this->_m = 1;
    }
    this->_n = _curves->get(0)->getNrOfDataPoints();

    // default settings init
    // Lambda
    this->_valueLambda = this->defaultLambda;
    ui->spinBoxLambda->setRange(this->minLambda,this->maxLambda);
    ui->spinBoxLambda->setSingleStep(10);
    ui->spinBoxLambda->setValue(this->defaultLambda);
    ui->horizontalSliderLambda->setRange(this->minLambda,this->maxLambda);
    ui->horizontalSliderLambda->setSingleStep(10);
    ui->horizontalSliderLambda->setValue(this->defaultLambda);
    connect(ui->horizontalSliderLambda,SIGNAL(valueChanged(int)),ui->spinBoxLambda,SLOT(setValue(int)));
    connect(ui->horizontalSliderLambda,SIGNAL(valueChanged(int)),this,SLOT(setLambdaValue(int)));
    connect(ui->spinBoxLambda,SIGNAL(valueChanged(int)),ui->horizontalSliderLambda,SLOT(setValue(int)));
    connect(ui->spinBoxLambda,SIGNAL(valueChanged(int)),this,SLOT(setLambdaValue(int)));
    // Ratio
    this->_valueRatio = this->defaultRatio;
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
    // maxIter
    this->_maxIter = this->defaultMaxIter;
    ui->spinBoxMaxIter->setRange(this->minMaxIter,this->maxMaxIter);
    ui->spinBoxMaxIter->setValue(this->defaultMaxIter);
    connect(ui->spinBoxMaxIter,SIGNAL(valueChanged(int)),this,SLOT(setMaxIterValue(int)));
    // endPoints
    this->_endPoints = this->defaultEndPoints;
    this->maxEndPoints = (_n-10)/2;
    ui->spinBoxEndPoints->setRange(this->minEndPoints,this->maxEndPoints);
    ui->spinBoxEndPoints->setValue(this->defaultEndPoints);
    connect(ui->spinBoxEndPoints,SIGNAL(valueChanged(int)),this,SLOT(setEndPointsValue(int)));
    // refineW
    this->_refineW = this->defaultRefineW;
    ui->doubleSpinBoxRefineW->setRange(this->minRefineW,this->maxRefineW);
    ui->doubleSpinBoxRefineW->setSingleStep(this->refineWStep);
    ui->doubleSpinBoxRefineW->setValue(this->defaultRefineW);
    connect(ui->doubleSpinBoxRefineW,SIGNAL(valueChanged(double)),this,SLOT(setRefineWValue(double)));
    // Widget Plot
    this->_plotBkg = ui->widgetPlot;
    _plotBkg->xAxis->setLabel("E / mV");
    _plotBkg->yAxis->setLabel("i / µA");
    // button Fit background
    connect(ui->pushButtonFitBkg,SIGNAL(clicked()),this,SLOT(calculateBkg()));
    // other buttons
    ui->pushButtonShowWithouBkg->setEnabled(false);
    ui->pushButtonExportBkg->setEnabled(false);

    // main vectors init
    this->_bkg = new QVector<QVector<double>>(this->_m, QVector<double>(this->_n));
    this->_weights = new QVector<QVector<double>>(this->_m, QVector<double>(this->_n));
    this->_iter = new QVector<int>(this->_m);

    // plot curves
    this->plotSignals();
    //this->tests();
}

EAQTArplsBackgroundCorrectionDialog::~EAQTArplsBackgroundCorrectionDialog()
{
    delete ui;
    delete this->_bkg;
    delete this->_weights;
    delete this->_iter;
}

void EAQTArplsBackgroundCorrectionDialog::tests()
{
    for (int current=0; current < this->_m; current++) {

        qDebug() << "****************";
        qDebug() << "_bkg: ";
        qDebug() << "size: " << _bkg[0].size() << " x " << _bkg[0][current].size();
        qDebug() << "first element: " << _bkg[0][current][0];

        qDebug() << "***";
        qDebug() << "_weights: ";
        qDebug() << "size: " << _weights[0].size() << " x " << _weights[0][current].size();
        qDebug() << "first element: " << _weights[0][current][0];

        qDebug() << "***";
        qDebug() << "_iter: ";
        qDebug() << "size: " << _iter->size();
        qDebug() << "first element: " << _iter[0][current];
    }

    qDebug() << "First curve background:";
    for (int i=0; i < _n; i++) {
        qDebug() << _bkg[0][0][i];
    }

    qDebug() << "First curve weights:";
    for (int i=0; i < _n; i++) {
        qDebug() << _weights[0][0][i];
    }

}

void EAQTArplsBackgroundCorrectionDialog::plotSignals()
{
    _plotBkg->clearGraphs();

    if(_act == SELECT::all)
    {
        Curve* curve;
        int i = 0;
        while((curve = _curves->get(i)) != nullptr )
        {
            _plotBkg->addGraph();
            _plotBkg->graph(i);
            _plotBkg->graph(i)->setData(curve->getXVector(),curve->getYVector());
            _plotBkg->graph(i)->setPen(QPen(COLOR::regular));
            i++;
        }
    }
    else
    {
        Curve* curve = _curves->get(_act);
        _plotBkg->addGraph();
        _plotBkg->graph(0);
        _plotBkg->graph(0)->setData(curve->getXVector(),curve->getYVector());
        _plotBkg->graph(0)->setPen(QPen(COLOR::regular));
    }

    _plotBkg->rescaleAxes();
    _plotBkg->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    _plotBkg->replot();
}

void EAQTArplsBackgroundCorrectionDialog::plotSignalsAndBkg()
{
    this->plotSignals();

    int gCount = _plotBkg->graphCount();
    for(int i = 0; i < _m; i++)
    {
        QVector<double> x = _curves->get(i)->getXVector();
        QVector<double> y = _bkg[0][i];
        _plotBkg->addGraph();
        _plotBkg->graph(gCount);
        _plotBkg->graph(gCount)->setData(x,y);
        _plotBkg->graph(gCount)->setPen(QPen(COLOR::background));
        gCount++;
    }

    _plotBkg->rescaleAxes();
    _plotBkg->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    _plotBkg->replot();
}

void EAQTArplsBackgroundCorrectionDialog::applyArPLS()
{
    arPLS2Ver2_initialize();
    ui->pushButtonFitBkg->setEnabled(false);
    ui->pushButtonShowWithouBkg->setEnabled(false);
    ui->pushButtonExportBkg->setEnabled(false);

    if(_act == SELECT::all)
    {
        for(int32_t i=0; i<_curves->count(); i++)
        {
            if(_curves->get(i) != nullptr)
            {
                qDebug() << "----------------";
                qDebug() << "Curve: " << i;
                //wywołanie funkcji
                tryArPLS(_curves->get(i),i);
            }
            else
            {
                throw 1;
            }
        }
    }
    else
    {
        Curve *c = _curves->get(_act);
        if(c != nullptr)
        {
            qDebug() << "----------------";
            qDebug() << "Curve: " << 0;
            //wywołanie funkcji
            tryArPLS(c,0);
        }
        else
        {
            throw 1;
        }
    }

    arPLS2Ver2_terminate();
    this->plotSignalsAndBkg();
    ui->pushButtonFitBkg->setEnabled(true);
    ui->pushButtonShowWithouBkg->setEnabled(true);
    ui->pushButtonExportBkg->setEnabled(true);
}

void EAQTArplsBackgroundCorrectionDialog::tryArPLS(Curve *c, int current)
{
    QVector<double> y = c->getYVector();
    main_arPLS2(&y,_valueLambda,_valueRatio,_maxIter,_endPoints,_refineW,current);
}

emxArray_real_T *EAQTArplsBackgroundCorrectionDialog::argInit_Unboundedx1_real_T(QVector<double> *y)
{
  emxArray_real_T *result;
  //static int iv0[1] = { y->size() };
  int iv = y->size();
  int *iv0 = &iv;

  int idx0;

  // Set the size of the array.
  // Change this size to the value that the application requires.
  result = emxCreateND_real_T(1, iv0);

  // Loop over the array to initialize each element.
  for (idx0 = 0; idx0 < result->size[0U]; idx0++) {
    // Set the value of the array element.
    // Change this value to the value that the application requires.
      result->data[idx0] = y->at(idx0);
  }

  return result;
}

void EAQTArplsBackgroundCorrectionDialog::main_arPLS2(QVector<double> *yqVect, int32_t lambda, double ratio,
                                                      int32_t maxIter, int32_t includeEndsNb, double threshold, int current)
{
    emxArray_real_T *bkg;
    emxArray_real_T *weights;
    emxArray_real_T *y;
    double iter;
    emxInitArray_real_T(&bkg, 1);
    emxInitArray_real_T(&weights, 1);

    // Initialize function 'arPLS2' input arguments.
    // Initialize function input argument 'y'.
    //y = argInit_Unboundedx1_real_T(yqVect);
    y = argInit_Unboundedx1_real_T(yqVect);

    // Call the entry-point 'arPLS2'.
    // arPLS(y, lambda, ratio, maxIter, includeEndsNb, threshold, bkg, weights)
    //arPLS2(y, lambda, ratio, maxIter, includeEndsNb, threshold, bkg, weights);
    arPLS2Ver2(y, lambda, ratio, maxIter, includeEndsNb, threshold, bkg, weights, &iter);

    qDebug() << "iterations nb: " << int32_t(iter);    

    for(int i = 0; i < yqVect->size(); i++) {
        this->_bkg[0][current][i] = bkg->data[i];
        this->_weights[0][current][i] = weights->data[i];
        this->_iter[0][current] = iter;
    }

    emxDestroyArray_real_T(weights);
    emxDestroyArray_real_T(bkg);
    emxDestroyArray_real_T(y);
}

void EAQTArplsBackgroundCorrectionDialog::calculateBkg()
{
    this->applyArPLS();
}


void EAQTArplsBackgroundCorrectionDialog::setValueDoubleSpinBoxRatio(int val)
{
    ui->doubleSpinBoxRatio->setValue(qPow(10,-log10(val)));
    _valueRatio = qPow(10,-log10(val));
}

void EAQTArplsBackgroundCorrectionDialog::setValueHorizontalSliderRatio(double val)
{
    ui->horizontalSliderRatio->setValue(qPow(10,abs(log10(val))));
    _valueRatio = val;
}

void EAQTArplsBackgroundCorrectionDialog::setLambdaValue(int val)
{
    _valueLambda = val;
}

void EAQTArplsBackgroundCorrectionDialog::setMaxIterValue(int val)
{
    _maxIter = val;
}

void EAQTArplsBackgroundCorrectionDialog::setEndPointsValue(int val)
{
    _endPoints = val;
}

void EAQTArplsBackgroundCorrectionDialog::setRefineWValue(double val)
{
    _refineW = val;
}



