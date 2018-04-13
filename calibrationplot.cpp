
  /*****************************************************************************************************************
  *  Electrochmical analyzer software EAQt to be used with 8KCA and M161
  *
  *  Copyright (C) 2017  Filip Ciepiela <filip.ciepiela@agh.edu.pl> and Małgorzata Jakubowska <jakubows@agh.edu.pl>
  *  This program is free software; you can redistribute it and/or modify 
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 3 of the License, or
  *  (at your option) any later version.
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software Foundation,
  *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
  *******************************************************************************************************************/
#include "calibrationplot.h"
#include "eaqtsignalprocessing.h"

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
    _plot->yAxis->setLabel(tr("i / %1").arg(_cd->yUnits));
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

CalibrationPlot::~CalibrationPlot()
{
    delete _plot;
    delete _te;
    delete _layout;
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
    QString text = tr("for α=0.05:<br>");
    int deg_freedom = _cd->xValues.size()-2;
    double talpha = EAQtSignalProcessing::tinv0975(deg_freedom);
    double confIntervalSlope = talpha * _cd->slopeStdDev;// / sqrt(_cd->xValues.size());
    double confIntervalIntercept = talpha * _cd->interceptStdDev;// / sqrt(_cd->xValues.size());
    if ( _cd->slope == 0 || !qIsFinite(_cd->intercept) ) {
        text += tr("Regression line cannot be plotted.<br>");
        text += tr("r = %1 <br>").arg(_cd->correlationCoef,0,'f',4);
        text += tr("i = %1(±%2)c + %3(±%4)<br>").arg(_cd->slope,0,'f',4).arg(confIntervalSlope,0,'f',4).arg(_cd->intercept,0,'f',4).arg(confIntervalIntercept,0,'f',4);
        _calibrationLine->setVisible(false);
        _plot->xAxis->setLabel(tr("c / %1").arg(_cd->xUnits));
        _plot->yAxis->setLabel(tr("i / %1").arg(_cd->yUnits));
        _plot->rescaleAxes();
        _plot->replot();
    } else {
        double x0 = -_cd->intercept/_cd->slope;
        _calibrationLine->point1->setCoords(1,_cd->slope*1+_cd->intercept);
        _calibrationLine->point2->setCoords(x0, 0);
        _calibrationLine->setVisible(true);
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
        text += tr("r = %1 <br>").arg(_cd->correlationCoef,0,'f',4);
        text += tr("i = %1(±%2)c + %3(±%4) <br>").arg(_cd->slope,0,'f',4).arg(confIntervalSlope,0,'f',4).arg(_cd->intercept,0,'f',4).arg(confIntervalIntercept,0,'f',4);
        if ( _cd->x0StdDev > -1 ) {
            double confIntervalX0 = EAQtSignalProcessing::tinv0975(_cd->xValues.size()-1) * _cd->x0StdDev / sqrt(_cd->xValues.size());
            text += tr("result: (%1±%2) %3").arg(-x0,0,'f',4).arg(confIntervalX0,0,'f',4).arg(_cd->xUnits);
        }
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
