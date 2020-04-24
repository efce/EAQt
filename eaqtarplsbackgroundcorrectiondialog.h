#ifndef EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H
#define EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H

#include <QDialog>
#include <QWidget>
#include "./Qcustomplot/qcustomplot.h"
#include "./arPLS2Ver2/rt_nonfinite.h"
#include "./arPLS2Ver2/arPLS2Ver2.h"
#include "./arPLS2Ver2/arPLS2Ver2_terminate.h"
#include "./arPLS2Ver2/arPLS2Ver2_emxAPI.h"
#include "./arPLS2Ver2/arPLS2Ver2_initialize.h"
#include "eaqtuiinterface.h"
#include "eaqtdata.h"
#include "curve.h"

namespace Ui {
class EAQTArplsBackgroundCorrectionDialog;
}

class EAQTArplsBackgroundCorrectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EAQTArplsBackgroundCorrectionDialog(QWidget *parent = nullptr);
    ~EAQTArplsBackgroundCorrectionDialog();


private:
    Ui::EAQTArplsBackgroundCorrectionDialog *ui;

    // data size
    int32_t _m;
    int32_t _n;

    // main curve collection
    CurveCollection *_curves;
    // actual (selected) curve nb
    int32_t _act;
    // main vectors
    QVector<QVector<double>> *_bkg;
    QVector<QVector<double>> *_weights;
    QVector<int> *_iter;

    // lambda and ratio param
    int32_t _valueLambda;
    double _valueRatio;

    // default param values
    int32_t minLambda = 10;
    int32_t maxLambda = 10000;
    int32_t defaultLambda = 1000;
    double minRatio = 0.0001;
    double maxRatio = 0.1;
    double defaultRatio = 0.01;

    void tests();
    void plotSignals();
    void applyArPLS();
    void tryArPLS(Curve *c, int current);
    emxArray_real_T *argInit_Unboundedx1_real_T(QVector<double> *y);
    void main_arPLS2(QVector<double> *y, int32_t lambda, double ratio, int32_t maxIter, int32_t includeEndsNb,
                     double threshold, int current);


private slots:
    void calculateBkg();
    void setValueDoubleSpinBoxRatio(int);
    void setValueHorizontalSliderRatio(double);

};




#endif // EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H
