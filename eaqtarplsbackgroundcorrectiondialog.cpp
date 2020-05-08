#include "eaqtarplsbackgroundcorrectiondialog.h"
#include "ui_eaqtarplsbackgroundcorrectiondialog.h"
#include <QIntValidator>
#include <QDoubleValidator>
#include <math.h>

EAQTArplsBackgroundCorrectionDialog::EAQTArplsBackgroundCorrectionDialog(QWidget *parent, EAQtUIInterface *pui) :
    QDialog(parent),
    ui(new Ui::EAQTArplsBackgroundCorrectionDialog)
{ 
    ui->setupUi(this);
    this->_pUI = pui;

    // main window title
    this->setWindowTitle(tr("arPLS background correction - EAQt"));

    // main curves init
    this->_curves = EAQtData::getInstance().getCurves();

    // actual curve init
    this->_act = EAQtData::getInstance().Act();

    // data size init
    if(_act == SELECT::all)
    {
        this->_m = _curves->count();
        this->_n = _curves->get(0)->getNrOfDataPoints();
    }
    else
    {
        this->_m = 1;
        this->_n = _curves->get(_act)->getNrOfDataPoints();
    }

    // main vectors init
    this->_curvesCopyXVector = new QVector<QVector<double>>(this->_m, QVector<double>(this->_n));
    this->_curvesCopyYVector = new QVector<QVector<double>>(this->_m, QVector<double>(this->_n));
    this->_bkg = new QVector<QVector<double>>(this->_m, QVector<double>(this->_n));
    this->_weights = new QVector<QVector<double>>(this->_m, QVector<double>(this->_n));
    this->_sig_without_bkg = new QVector<QVector<double>>(this->_m, QVector<double>(this->_n));
    this->_iter = new QVector<int>(this->_m);

    // copy Collection Curves
    if(_act == SELECT::all)
    {
        for (int i=0; i < _m; i++)
        {
            _curvesCopyXVector[0][i] = _curves->get(i)->getXVector();
            _curvesCopyYVector[0][i] = _curves->get(i)->getYVector();
            _weights[0][i] = QVector<double>(_n,0.0);
        }
    }
    else
    {
        _curvesCopyXVector[0][0] = _curves->get(_act)->getXVector();
        _curvesCopyYVector[0][0] = _curves->get(_act)->getYVector();
        _weights[0][0] = QVector<double>(_n,0.0);
    }

    // "status bar"
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->hide();
    ui->labelIterations->setText("");
    ui->labelIterations->hide();
    ui->labelStatus->setText("");
    ui->labelStatus->hide();
    ui->labelSeparator->hide();

    // grupBox
    ui->groupBoxArplsParams->setTitle(tr("arPLS parameters"));
    ui->groupBoxOtherParams->setTitle(tr("Other parameters"));

    // default settings init
    // Lambda
    ui->labelLambda->setText(tr("Lambda"));
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
    ui->labelRatio->setText(tr("Ratio"));
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
    ui->labelMaxIter->setText(tr("Max. number of iterations"));
    this->_maxIter = this->defaultMaxIter;
    ui->spinBoxMaxIter->setRange(this->minMaxIter,this->maxMaxIter);
    ui->spinBoxMaxIter->setValue(this->defaultMaxIter);
    connect(ui->spinBoxMaxIter,SIGNAL(valueChanged(int)),this,SLOT(setMaxIterValue(int)));
    // endPoints
    ui->labelEndPoints->setText(tr("Set 'w=1' to marginal points"));
    this->_endPoints = this->defaultEndPoints;
    this->maxEndPoints = (_n-10)/2;
    ui->spinBoxEndPoints->setRange(this->minEndPoints,this->maxEndPoints);
    ui->spinBoxEndPoints->setValue(this->defaultEndPoints);
    connect(ui->spinBoxEndPoints,SIGNAL(valueChanged(int)),this,SLOT(setEndPointsValue(int)));
    // refineW
    ui->labelRefineW->setText(tr("Refine 'w' threshold"));
    this->_refineW = this->defaultRefineW;
    ui->doubleSpinBoxRefineW->setRange(this->minRefineW,this->maxRefineW);
    ui->doubleSpinBoxRefineW->setSingleStep(this->refineWStep);
    ui->doubleSpinBoxRefineW->setValue(this->defaultRefineW);
    connect(ui->doubleSpinBoxRefineW,SIGNAL(valueChanged(double)),this,SLOT(setRefineWValue(double)));
    // Widget Plots
    this->_plotBkg = ui->widgetPlot;
    _plotBkg->xAxis->setLabel("E / mV");
    _plotBkg->yAxis->setLabel("i / µA");
    _plotBkg->plotLayout()->insertRow(0);
    _plotBkg->plotLayout()->addElement(0, 0, new QCPTextElement(_plotBkg, tr("Signals and fitted background"), QFont("sans", 8, QFont::Normal)));
    this->_plotW = ui->widgetPlotW;
    _plotW->xAxis->setLabel("E / mV");
    _plotW->yAxis->setLabel("w");
    _plotW->plotLayout()->insertRow(0);
    _plotW->plotLayout()->addElement(0, 0, new QCPTextElement(_plotW, tr("Weights"), QFont("sans", 8, QFont::Normal)));
    // button Fit background
    connect(ui->pushButtonFitBkg,SIGNAL(clicked()),this,SLOT(calculateBkg()));
    // other buttons
    ui->pushButtonShowWithouBkg->setEnabled(false);
    ui->pushButtonExportBkg->setEnabled(false);
    connect(ui->pushButtonShowWithouBkg,SIGNAL(clicked()),this,SLOT(showWithoutBkg()));
    connect(ui->pushButtonExportBkg,SIGNAL(clicked()),this,SLOT(exportCurvesWithoutBkg()));

    // plot curves
    this->plotSignals();
}

EAQTArplsBackgroundCorrectionDialog::~EAQTArplsBackgroundCorrectionDialog()
{
    delete ui;
    delete this->_curvesCopyXVector;
    delete this->_curvesCopyYVector;
    delete this->_bkg;
    delete this->_weights;
    delete this->_sig_without_bkg;
    delete this->_iter;
}

void EAQTArplsBackgroundCorrectionDialog::plotSignals()
{
    _plotBkg->clearGraphs();
    _plotW->clearGraphs();

    for (int i=0; i < _m; i++)
    {
        addGraphs(_plotBkg, i, _curvesCopyXVector[0][i], _curvesCopyYVector[0][i], QPen(COLOR::regular));
        addGraphs(_plotW, i, _curvesCopyXVector[0][i], _weights[0][i], QPen(COLOR::regular));
    }

    rescaleAndReplot(_plotBkg);
    rescaleAndReplot(_plotW);
}

void EAQTArplsBackgroundCorrectionDialog::plotSignalsAndBkg()
{
    _plotBkg->clearGraphs();
    _plotW->clearGraphs();
    this->plotSignals();

    int gCount = _plotBkg->graphCount();
    for(int i = 0; i < _m; i++)
    {
        addGraphs(_plotBkg, gCount, _curvesCopyXVector[0][i], _bkg[0][i], QPen(COLOR::background));
        gCount++;
        addGraphs(_plotW, i, _curvesCopyXVector[0][i], _weights[0][i], QPen(COLOR::regular));
    }

    rescaleAndReplot(_plotBkg);
    rescaleAndReplot(_plotW);
}

void EAQTArplsBackgroundCorrectionDialog::plotSignalsWithoutBkg()
{
    _plotBkg->clearGraphs();
    if(!plotInticator)
    {
        for(int i = 0; i < _m; i++)
        {
            addGraphs(_plotBkg, i, _curvesCopyXVector[0][i], _sig_without_bkg[0][i], QPen(COLOR::regular));
        }

        rescaleAndReplot(_plotBkg);
        ui->pushButtonShowWithouBkg->setText(tr("BACK"));
        ui->pushButtonExportBkg->setEnabled(false);
        ui->pushButtonFitBkg->setEnabled(false);
        plotInticator = true;
    }
    else
    {
        plotSignalsAndBkg();
        ui->pushButtonShowWithouBkg->setText(tr("Show signals without background"));
        ui->pushButtonExportBkg->setEnabled(true);
        ui->pushButtonFitBkg->setEnabled(true);
        plotInticator = false;
    }

}

void EAQTArplsBackgroundCorrectionDialog::addGraphs(QCustomPlot *plt, int iter, QVector<double> x, QVector<double> y, QPen col)
{
    plt->addGraph();
    plt->graph(iter);
    plt->graph(iter)->setData(x,y);
    plt->graph(iter)->setPen(col);
}

void EAQTArplsBackgroundCorrectionDialog::rescaleAndReplot(QCustomPlot *plt)
{
    plt->rescaleAxes();
    //plt->xAxis->setRangeReversed(true);
    plt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    plt->replot();
}

void EAQTArplsBackgroundCorrectionDialog::applyArPLS()
{
    ui->labelStatus->show();
    ui->labelStatus->setText(tr("Calculating background for curve: -"));
    ui->labelIterations->show();
    ui->labelIterations->setText(tr("iterations: -"));
    ui->progressBar->show();
    ui->progressBar->setValue(0);
    ui->labelSeparator->show();

    arPLS2Ver2_initialize();
    plotInticator = false;

    for(int i=0; i<_m; i++)
    {
        QString s1 = tr("Calculating background for curve: ");
        QString s2 = QString::number(i+1);
        ui->labelStatus->setText(s1.append(s2));

        //call arPLS function
        tryArPLS(_curvesCopyYVector[0][i],i);
        QCoreApplication::processEvents();
        ui->progressBar->setValue(((i+1)*100/_m));
    }

    arPLS2Ver2_terminate();
    this->plotSignalsAndBkg();
    //QCoreApplication::processEvents();
    //ui->progressBar->hide();
    //ui->labelStatus->hide();
    //ui->labelIterations->hide();

}

void EAQTArplsBackgroundCorrectionDialog::tryArPLS(QVector<double> y, int current)
{
    main_arPLS2(&y,_valueLambda,_valueRatio,_maxIter,_endPoints,_refineW,current);
}

emxArray_real_T *EAQTArplsBackgroundCorrectionDialog::argInit_Unboundedx1_real_T(QVector<double> *y)
{
  emxArray_real_T *result;
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
    y = argInit_Unboundedx1_real_T(yqVect);

    // Call the entry-point 'arPLS2'.
    arPLS2Ver2(y, lambda, ratio, maxIter, includeEndsNb, threshold, bkg, weights, &iter);

    QString s3 = tr("iterations: ");
    ui->labelIterations->setText(s3.append(QString::number(iter)));

    for(int i = 0; i < yqVect->size(); i++) {
        this->_bkg[0][current][i] = bkg->data[i];
        this->_weights[0][current][i] = weights->data[i];
        this->_iter[0][current] = iter;
    }

    this->subtractBkg();

    emxDestroyArray_real_T(weights);
    emxDestroyArray_real_T(bkg);
    emxDestroyArray_real_T(y);
}

void EAQTArplsBackgroundCorrectionDialog::subtractBkg()
{
    for(int i = 0; i < _m; i++)
    {
        for (int j = 0; j < _n; j++) {
            _sig_without_bkg[0][i][j] = _curvesCopyYVector[0][i][j] - _bkg[0][i][j];
        }
    }
}

void EAQTArplsBackgroundCorrectionDialog::calculateBkg()
{
    ui->pushButtonShowWithouBkg->setEnabled(false);
    ui->pushButtonExportBkg->setEnabled(false);
    ui->pushButtonFitBkg->setEnabled(false);

    this->applyArPLS();

    ui->pushButtonShowWithouBkg->setEnabled(true);
    ui->pushButtonExportBkg->setEnabled(true);
    ui->pushButtonFitBkg->setEnabled(true);
}

void EAQTArplsBackgroundCorrectionDialog::showWithoutBkg()
{
    this->plotSignalsWithoutBkg();
}

void EAQTArplsBackgroundCorrectionDialog::exportCurvesWithoutBkg()
{
    if(_act == SELECT::all)
    {
        for(int i = 0; i < _m; i++)
        {
            Curve *c = _curves->get(i);
            for(int j = 0; j < _n; j++)
            {
                c->setYValue(j,_sig_without_bkg[0][i][j]);
            }

        }
    }
    else
    {
        Curve *c = _curves->get(_act);
        for (int j = 0; j < _n; j++)
        {
            c->setYValue(j,_sig_without_bkg[0][0][j]);
        }
    }

    _pUI->updateAll(true);
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







