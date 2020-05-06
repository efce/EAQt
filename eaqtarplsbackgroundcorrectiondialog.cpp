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
        }
    }
    else
    {
        _curvesCopyXVector[0][0] = _curves->get(_act)->getXVector();
        _curvesCopyYVector[0][0] = _curves->get(_act)->getYVector();
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
    this->_plotW = ui->widgetPlotW;
    _plotW->xAxis->setLabel("E / mV");
    _plotW->yAxis->setLabel("w");
    //_plotW->xAxis->setRange(0,1);
    //_plotW->yAxis->setRange(-0.2,1.2);
    // button Fit background
    connect(ui->pushButtonFitBkg,SIGNAL(clicked()),this,SLOT(calculateBkg()));
    // other buttons
    ui->pushButtonShowWithouBkg->setEnabled(false);
    ui->pushButtonExportBkg->setEnabled(false);
    connect(ui->pushButtonShowWithouBkg,SIGNAL(clicked()),this,SLOT(showWithoutBkg()));
    connect(ui->pushButtonExportBkg,SIGNAL(clicked()),this,SLOT(exportCurvesWithoutBkg()));



    // plot curves
    this->plotSignals();
    //this->tests();
}

EAQTArplsBackgroundCorrectionDialog::~EAQTArplsBackgroundCorrectionDialog()
{
    delete ui;
    delete this->_bkg;
    delete this->_weights;
    delete this->_sig_without_bkg;
    delete this->_iter;
}

void EAQTArplsBackgroundCorrectionDialog::tests()
{

}

void EAQTArplsBackgroundCorrectionDialog::plotSignals()
{
    _plotBkg->clearGraphs();

/*    if(_act == SELECT::all)
    {
//        Curve* curve;
//        int i = 0;
//        while((curve = _curves->get(i)) != nullptr )
//        {
//            addGraphs(_plotBkg, i, curve->getXVector(), curve->getYVector(), QPen(COLOR::regular));
//            i++;
//        }
        for (int i=0; i < _m; i++)
        {
            addGraphs(_plotBkg, i, _curvesCopyXVector[0][i], _curvesCopyYVector[0][i], QPen(COLOR::regular));
        }
    }
    else
    {
//        Curve* curve = _curves->get(_act);
//        addGraphs(_plotBkg, 0, curve->getXVector(), curve->getYVector(), QPen(COLOR::regular));
        addGraphs(_plotBkg, 0, _curvesCopyXVector[0][0], _curvesCopyYVector[0][0], QPen(COLOR::regular));
    }*/

    for (int i=0; i < _m; i++)
    {
        addGraphs(_plotBkg, i, _curvesCopyXVector[0][i], _curvesCopyYVector[0][i], QPen(COLOR::regular));
    }

    rescaleAndReplot(_plotBkg);
}

void EAQTArplsBackgroundCorrectionDialog::plotSignalsAndBkg()
{
    _plotBkg->clearGraphs();
    this->plotSignals();

    int gCount = _plotBkg->graphCount();
    for(int i = 0; i < _m; i++)
    {
        //QVector<double> x = _curves->get(i)->getXVector();
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
//        if(_act == SELECT::all)
//        {
//            for(int i = 0; i < _m; i++)
//            {
//                QVector<double> x = _curves->get(i)->getXVector();
//                addGraphs(_plotBkg, i, x, _sig_without_bkg[0][i], QPen(COLOR::regular));
//            }
//        }
//        else
//        {
//            Curve* curve = _curves->get(_act);
//            QVector<double> x = curve->getXVector();
//            addGraphs(_plotBkg, 0, x, _sig_without_bkg[0][0], QPen(COLOR::regular));
//        }
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
    arPLS2Ver2_initialize();
    plotInticator = false;

//    if(_act == SELECT::all)
//    {
//        for(int i=0; i<_m; i++)
//        {
//            if(_curves->get(i) != nullptr)
//            {
//                QString s1 = tr("Calculating background for curve: ");
//                QString s2 = QString::number(i+1);
//                ui->labelStatus->setText(s1.append(s2).append(" |"));
//                //call arPLS function
//                tryArPLS(_curves->get(i),i);
//                QCoreApplication::processEvents();
//                ui->progressBar->setValue(((i+1)*100/_m));
//            }
//            else
//            {
//                throw 1;
//            }
//        }
//    }
//    else
//    {
//        Curve *c = _curves->get(_act);
//        if(c != nullptr)
//        {
//            QString s1 = tr("Calculating background for curve: ");
//            ui->labelStatus->setText(s1.append("1").append(" |"));
//            //call arPLS function
//            tryArPLS(c,0);
//            QCoreApplication::processEvents();
//            ui->progressBar->setValue(100);
//        }
//        else
//        {
//            throw 1;
//        }
//    }

    for(int i=0; i<_m; i++)
    {
        QString s1 = tr("Calculating background for curve: ");
        QString s2 = QString::number(i+1);
        ui->labelStatus->setText(s1.append(s2).append(" |"));
        //call arPLS function
        tryArPLS(_curvesCopyYVector[0][i],i);
        QCoreApplication::processEvents();
        ui->progressBar->setValue(((i+1)*100/_m));
    }

    arPLS2Ver2_terminate();
    this->plotSignalsAndBkg();
    QCoreApplication::processEvents();
    ui->progressBar->hide();
    ui->labelStatus->hide();
    ui->labelIterations->hide();
    ui->pushButtonShowWithouBkg->setEnabled(true);
    ui->pushButtonExportBkg->setEnabled(true);
}

void EAQTArplsBackgroundCorrectionDialog::tryArPLS(QVector<double> y, int current)
{
    //QVector<double> y = c->getYVector();
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
    y = argInit_Unboundedx1_real_T(yqVect);

    // Call the entry-point 'arPLS2'.
    arPLS2Ver2(y, lambda, ratio, maxIter, includeEndsNb, threshold, bkg, weights, &iter);

    QString s3 = tr("algorithm iterations number: ");
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
//    if(_act == SELECT::all)
//    {
//        for(int i = 0; i < _m; i++)
//        {
//            QVector<double> y = _curves->get(i)->getYVector();
//            QVector<double> b = _bkg[0][i];
//            QVector<double> swb(_n);
//            for (int j = 0; j < _n; j++) {
//                swb[j] = y[j] - b[j];
//            }
//            this->_sig_without_bkg[0][i] = swb;
//        }
//    }
//    else
//    {
//        Curve* curve = _curves->get(_act);
//        QVector<double> y = curve->getYVector();
//        QVector<double> b = _bkg[0][0];
//        QVector<double> swb(_n);
//        for (int j = 0; j < _n; j++) {
//            swb[j] = y[j] - b[j];
//        }
//        _sig_without_bkg[0][0] = swb;
//    }

    for(int i = 0; i < _m; i++)
    {
        for (int j = 0; j < _n; j++) {
            _sig_without_bkg[0][i][j] = _curvesCopyYVector[0][i][j] - _bkg[0][i][j];
        }
    }
}

void EAQTArplsBackgroundCorrectionDialog::calculateBkg()
{
    ui->labelStatus->show();
    ui->labelIterations->show();
    ui->progressBar->show();
    ui->progressBar->setValue(0);
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







