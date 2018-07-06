#ifndef EAQTCALCULATELOD_H
#define EAQTCALCULATELOD_H

#include <QObject>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include "calibrationdata.h"
#include "eaqtsignalprocessing.h"

class EAQtCalculateLODDialog : QObject
{
    Q_OBJECT

public:
    EAQtCalculateLODDialog(CalibrationData *calibration);
    ~EAQtCalculateLODDialog();
    void exec();

private:
    QDialog *_dialog;
    QLineEdit *_leSTDBlank;
    QLabel *_lTXTBlank;
    QPushButton *_butCalculate;
    QLabel *_lResult;
    CalibrationData *_calibration;

private slots:
    void calculate();
};

#endif // EAQTCALCULATELOD_H
