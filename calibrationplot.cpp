#include "calibrationplot.h"

CalibrationPlot::CalibrationPlot(CalibrationData *cd) : QWidget()
{
    _cd = cd;
    _layout = new QVBoxLayout();
    setupPlot();
    setupTextEdit();
    this->setLayout(_layout);
}


void CalibrationPlot::setupPlot()
{
    _plot = new QCustomPlot();
    _plot->xAxis->setLabel(tr("c / %1").arg(_cd->xUnits));
    _plot->yAxis->setLabel(tr("i / µA"));
    _plot->setVisible(true);
    _plot->setMinimumWidth(500);
    _plot->setMinimumHeight(450);
    //connect(_plot,SIGNAL(beforeReplot()),this,SLOT(beforeReplot()));
    QPen pen;
    pen.setColor(COLOR::measurement);
    pen.setWidth(1);
    _plot->xAxis->grid()->setZeroLinePen(pen);
    _plot->yAxis->grid()->setZeroLinePen(pen);
    _calibrationLine = new QCPItemStraightLine(_plot);
    _calibrationLine->setPen(QPen(QColor(COLOR::active)));
    _calibrationPoints = _plot->addGraph();
    _calibrationPoints->setLineStyle(QCPGraph::lsNone);
    _calibrationPoints->setScatterStyle(QCPScatterStyle::ssCircle);
    _calibrationPoints->setPen(QPen(COLOR::regular));
    _layout->addWidget(_plot);
}

void CalibrationPlot::setupTextEdit()
{
    _te = new QTextEdit();
    _te->setReadOnly(true);
    _layout->addWidget(_te);
    _te->setAlignment(Qt::AlignCenter);
}

void CalibrationPlot::update()
{
    _plot->setVisible(true);
    _calibrationPoints->setData(_cd->xValues,_cd->yValues,false);
    double x0 = -_cd->intercept/_cd->slope;
    _calibrationLine->point1->setCoords(1,_cd->slope*1+_cd->intercept);
    _calibrationLine->point2->setCoords(x0, 0);
    _plot->rescaleAxes();
    if ( _plot->xAxis->range().lower > x0 ) {
        _plot->xAxis->setRangeLower(x0);
    }
    if ( _plot->yAxis->range().lower > 0 ) {
        _plot->yAxis->setRangeLower(0);
    }
    _plot->xAxis->setLabel(tr("c / %1").arg(_cd->xUnits));
    double spanx = _plot->xAxis->range().upper - _plot->xAxis->range().lower;
    _plot->xAxis->setRangeLower(_plot->xAxis->range().lower - (0.1*spanx));
    _plot->xAxis->setRangeUpper(_plot->xAxis->range().upper + (0.1*spanx));
    double spany = _plot->yAxis->range().upper - _plot->yAxis->range().lower;
    _plot->yAxis->setRangeLower(_plot->yAxis->range().lower - (0.1*spany));
    _plot->yAxis->setRangeUpper(_plot->yAxis->range().upper + (0.1*spany));
    _plot->yAxis->setLabel(tr("i / %1").arg(_cd->yUnits));
    _plot->replot();
    QString text;
    text = tr("r = %1 <br>").arg(_cd->correlationCoef,0,'f',4);
    text += tr("i = %1(±%2)c + %3(±%4) <br>").arg(_cd->slope,0,'f',4).arg(_cd->slopeStdDev,0,'f',4).arg(_cd->intercept,0,'f',4).arg(_cd->interceptStdDev,0,'f',4);
    if ( _cd->x0StdDev > -1 ) {
        text += tr("result: (%1±%2) %3").arg(-x0,0,'f',4).arg(_cd->x0StdDev,0,'f',4).arg(_cd->xUnits);
    }
    _te->setText(text);
    _te->setVisible(true);
}

void CalibrationPlot::beforeReplot()
{
    int pxx = _plot->yAxis->coordToPixel(0);
    int pxy = _plot->xAxis->coordToPixel(0);
    _plot->xAxis->setOffset(-_plot->axisRect()->height()-_plot->axisRect()->top()+pxx);
    _plot->yAxis->setOffset(_plot->axisRect()->left()-pxy);
}
