
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
#include "eaqtcalibrationdialog.h"
#include "eaqtdata.h"
#include "eaqtsignalprocessing.h"

EAQtCalibrationDialog::EAQtCalibrationDialog(QVector<double> calY)
{
    if ( calY.size() != EAQtData::getInstance().getCurves()->count() ) {
        throw 1;
    }
    _signals = calY;
    QGridLayout *gl = new QGridLayout();
    QGridLayout *glm = new QGridLayout();
    QScrollArea *sa = new QScrollArea();
    sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    sa->setFixedWidth(300);
    sa->setMaximumHeight(500);
    this->_dialog = new QDialog();
    this->_leConcentrations.resize(calY.size());
    uint i;
    QFont *fontLabel = new QFont(_dialog->font());
    fontLabel->setBold(true);
    QLabel *lname = new QLabel(tr("Name"));
    lname->setFont(*fontLabel);
    QLabel *lcurr = new QLabel(tr("Current"));
    lcurr->setFont(*fontLabel);
    QLabel *lconc = new QLabel(tr("Concentration"));
    lconc->setFont(*fontLabel);
    gl->addWidget(lname,0,0,1,1);
    gl->addWidget(lcurr,0,1,1,1);
    gl->addWidget(lconc,0,2,1,1);
    for ( i = 0; i<calY.size(); ++i ) {
        QLabel *l1 = new QLabel(EAQtData::getInstance().getCurves()->get(i)->CName() + ": ");
        QLabel *l2 = new QLabel(EAQtData::getInstance().dispI(calY[i]) + " ");
        this->_leConcentrations[i] = new QLineEdit();
        this->_leConcentrations[i]->setValidator(new QDoubleValidator(0.0,999999.9,10));
        this->_leConcentrations[i]->setText("0.0");
        this->_leConcentrations[i]->setMaxLength(12);
        this->_leConcentrations[i]->setFixedWidth(QFontMetrics(_dialog->font()).width("9999999999"));
        gl->addWidget(l1,i+1,0,1,1,Qt::AlignRight);
        gl->addWidget(l2,i+1,1,1,1);
        gl->addWidget(this->_leConcentrations[i],i+1,2,1,1);
    }
    QWidget *saw = new QWidget();
    saw->setLayout(gl);
    sa->setLayout(new QVBoxLayout);
    sa->setWidget(saw);
    glm->addWidget(sa,0,0,1,2);
    QPushButton *butCal = new QPushButton(tr("Plot calibration"));
    QPushButton *butCancel = new QPushButton(tr("Cancel"));
    glm->addWidget(butCal,1,0,1,1);
    glm->addWidget(butCancel,1,1,1,1);
    QWidget* qw = preparePlot();
    glm->addWidget(qw,0,2,2,1);
    connect(butCancel,SIGNAL(clicked(bool)),this->_dialog,SLOT(close()));
    connect(butCal,SIGNAL(clicked(bool)), this,SLOT(drawCalibration()));
    this->_dialog->setLayout(glm);
}

void EAQtCalibrationDialog::exec()
{
   this->_dialog->exec();
}

void EAQtCalibrationDialog::drawCalibration()
{
    QVector<double> conc;
    conc.resize(_leConcentrations.size());
    for ( int i = 0; i<conc.size(); ++i) {
        conc[i] = _leConcentrations[i]->text().toDouble();
    }
    double correlationCoef;
    EAQtSignalProcessing::correlation(conc,_signals, &correlationCoef);
    double slope, intercept;
    EAQtSignalProcessing::linearRegression(conc,_signals,&slope,&intercept);
    _calibrationPlot->setVisible(true);
    _calibrationPoints->setData(conc,_signals,false);
    double x0 = -intercept/slope;
    _calibrationLine->point1->setCoords(1,slope*1+intercept);
    _calibrationLine->point2->setCoords(x0, 0);
    _calibrationPlot->rescaleAxes();
    if ( _calibrationPlot->xAxis->range().lower > x0 ) {
        _calibrationPlot->xAxis->setRangeLower(x0);
    }
    if ( _calibrationPlot->yAxis->range().lower > 0 ) {
        _calibrationPlot->yAxis->setRangeLower(0);
    }
    double spanx = _calibrationPlot->xAxis->range().upper - _calibrationPlot->xAxis->range().lower;
    _calibrationPlot->xAxis->setRangeLower(_calibrationPlot->xAxis->range().lower - (0.1*spanx));
    _calibrationPlot->xAxis->setRangeUpper(_calibrationPlot->xAxis->range().upper + (0.1*spanx));
    double spany = _calibrationPlot->yAxis->range().upper - _calibrationPlot->yAxis->range().lower;
    _calibrationPlot->yAxis->setRangeLower(_calibrationPlot->yAxis->range().lower - (0.1*spany));
    _calibrationPlot->yAxis->setRangeUpper(_calibrationPlot->yAxis->range().upper + (0.1*spany));
    _calibrationPlot->replot();
    _calibrationR->setText(tr("r = %1").arg(correlationCoef,0,'f',4));
    _calibrationEq->setText(tr("i = %1c + %2").arg(slope,0,'f',4).arg(intercept,0,'f',4));
    _calibrationEq->setVisible(true);
    _calibrationR->setVisible(true);
}

QWidget* EAQtCalibrationDialog::preparePlot()
{
    QWidget* w = new QWidget();
    QGridLayout *gl = new QGridLayout();
    _calibrationEq = new QLineEdit();
    _calibrationEq->setReadOnly(true);
    _calibrationEq->setVisible(false);
    _calibrationEq->setAlignment(Qt::AlignCenter);
    _calibrationR = new QLineEdit();
    _calibrationR->setVisible(false);
    _calibrationR->setReadOnly(true);
    _calibrationR->setAlignment(Qt::AlignCenter);
    _calibrationPlot = new QCustomPlot();
    _calibrationPlot->xAxis->setLabel(tr("c"));
    _calibrationPlot->yAxis->setLabel(tr("i / µA"));
    _calibrationPlot->setVisible(false);
    _calibrationPlot->setMinimumWidth(500);
    _calibrationPlot->setMinimumHeight(450);
    _calibrationLine = new QCPItemStraightLine(_calibrationPlot);
    _calibrationPoints = _calibrationPlot->addGraph();
    _calibrationPoints->setLineStyle(QCPGraph::lsNone);
    _calibrationPoints->setScatterStyle(QCPScatterStyle::ssCircle);
    gl->addWidget(_calibrationPlot,0,0,1,2);
    gl->addWidget(_calibrationEq,1,0,1,1);
    gl->addWidget(_calibrationR,1,1,1,1);
    w->setLayout(gl);
    return w;
}
