#ifndef CALIBRATIONPLOT_H
#define CALIBRATIONPLOT_H

#include <QTextEdit>
#include "Qcustomplot/qcustomplot.h"
#include "calibrationdata.h"

class CalibrationPlot: public QWidget
{
public:
    CalibrationPlot(CalibrationData* cd);
    void update();

private:
    CalibrationData *_cd;
    QCustomPlot *_plot;
    QTextEdit *_te;
    QVBoxLayout *_layout;
    void setupPlot();
    void setupTextEdit();
    QCPItemStraightLine *_calibrationLine;
    QCPGraph *_calibrationPoints;

private slots:
    void beforeReplot(); //NOT USED
};

#endif // CALIBRATIONPLOT_H
