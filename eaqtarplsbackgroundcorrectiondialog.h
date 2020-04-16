#ifndef EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H
#define EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H

#include <QDialog>
#include <QWidget>
#include "./Qcustomplot/qcustomplot.h"
#include "./arPLS2/rt_nonfinite.h"
#include "./arPLS2/arPLS2.h"
#include "./arPLS2/arPLS2_terminate.h"
#include "./arPLS2/arPLS2_emxAPI.h"
#include "./arPLS2/arPLS2_initialize.h"
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
    void showCurves();
    void plotSignals();
    void plotSignals(CurveCollection*);
    void plotSignals(CurveCollection);

private:
    Ui::EAQTArplsBackgroundCorrectionDialog *ui;
    CurveCollection *_signals;
    CurveCollection *_bkg;

    CurveCollection cc1 = EAQtData::getInstance().getCurves();
    CurveCollection cc2 = EAQtData::getInstance().getCurves();
    CurveCollection *cc3;

    // params
    uint minLambda = 10;
    uint maxLambda = 10000;
    uint defaultLambda = 1000;
    double minRatio = 0.0001;
    double maxRatio = 0.1;
    double defaultRatio = 0.001;

private slots:
    void setValueDoubleSpinBoxRatio(int);
    void setValueHorizontalSliderRatio(double);
};

#endif // EAQTARPLSBACKGROUNDCORRECTIONDIALOG_H
