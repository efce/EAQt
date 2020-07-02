#ifndef EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H
#define EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H

#include <QDialog>
#include <QWidget>
#include "./Qcustomplot/qcustomplot.h"
#include "eaqtuiinterface.h"
#include "eaqtdata.h"
#include "curve.h"
#include "arPLS/arPLS2EigenF/arPLSEigenF.h"
#include "arPLS/arPLS2EigenD/arPLSEigenD.h"

namespace Ui {
class EAQTArplsBackgroundCorrectionDialog;
}

class EAQTArplsBackgroundCorrectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EAQTArplsBackgroundCorrectionDialog(QWidget *parent = nullptr, EAQtUIInterface *pui = nullptr);
    ~EAQTArplsBackgroundCorrectionDialog();


private:
    Ui::EAQTArplsBackgroundCorrectionDialog *ui;

    void plotSignals();
    void plotSignalsAndBkg();
    void plotSignalsWithoutBkg();
    void addGraphs(QCustomPlot *plt, int iter, QVector<double> x, QVector<double> y, QPen col);
    void rescaleAndReplot(QCustomPlot *plt);
    void applyArPLS();
    //void tryArPLS(Curve *c, int current);
    void tryArPLS(QVector<double> y, int current);
//    emxArray_real_T *argInit_Unboundedx1_real_T(QVector<double> *y);
    void main_arPLS2(QVector<double> *y, int32_t lambda, double ratio, int32_t maxIter, int32_t includeEndsNb,
                     double threshold, int current);
    void subtractBkg();

    EAQtUIInterface *_pUI;

    // data size
    int32_t _m;
    int32_t _n;

    // main curve collection
    CurveCollection *_curves;
    // actual (selected) curve nb
    int32_t _act;
    // main vectors
    QVector<QVector<double>> *_curvesCopyXVector;
    QVector<QVector<double>> *_curvesCopyYVector;
    QVector<QVector<double>> *_bkg;
    QVector<QVector<double>> *_weights;
    QVector<QVector<double>> *_sig_without_bkg;
    QVector<int> *_iter;

    // qCustomPlot
    QCustomPlot *_plotBkg;
    QCustomPlot *_plotW;

    // lambda and ratio param
    int32_t _valueLambda;
    double _valueRatio;
    // other params
    int32_t _maxIter;
    int32_t _endPoints;
    double _refineW;

    // default param values
    int32_t minLambda = 10;
    int32_t maxLambda = 10000;
    int32_t defaultLambda = 1000;
    double minRatio = 0.0001;
    double maxRatio = 0.1;
    double defaultRatio = 0.01;
    int32_t minMaxIter = 0;
    int32_t maxMaxIter = 1000;
    int32_t defaultMaxIter = 100;
    int32_t minEndPoints = 0;
    int32_t maxEndPoints;
    int32_t defaultEndPoints = 0;
    double minRefineW = 0.0;
    double maxRefineW = 0.5;
    double defaultRefineW = 0.0;
    double refineWStep = 0.1;
    // other
    bool plotInticator = false;
    bool cancelCalcul = false;

private slots:
    void calculateBkg();
    void cancelCalculation();
    void showWithoutBkg();
    void exportCurvesWithoutBkg();
    void setValueDoubleSpinBoxRatio(int);
    void setValueHorizontalSliderRatio(double);
    void setLambdaValue(int);
    void setMaxIterValue(int);
    void setEndPointsValue(int);
    void setRefineWValue(double);

};




#endif // EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H
