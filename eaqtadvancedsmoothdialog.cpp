
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
#include "eaqtadvancedsmoothdialog.h"
#include "eaqtdata.h"
#include "eaqtsignalprocessing.h"

EAQtAdvancedSmoothDialog::EAQtAdvancedSmoothDialog(int lastUsed, const std::vector<double>& params) : QObject()
{
    _params = params;

    _dialog = new QDialog();
    _dialog->setModal(true);
    _dialog->setWindowTitle(tr("Advanced curve smoothing"));
    _dialog->setLayout(generateLayout(lastUsed));
    methodChanged();
    updateFrequencyPlot();
}


QGridLayout* EAQtAdvancedSmoothDialog::generateLayout(int select)
{
    QGridLayout* gl = new QGridLayout();

    int vpos=0;
    _radMethod.resize(3);
    _radMethod[method_sg] = new QRadioButton();
    _radMethod[method_sg]->setText(tr("Savitzky-Golay"));
    _radMethod[method_sg]->setChecked(true);
    connect(_radMethod[method_sg],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
    QLabel *lSpan = new QLabel(tr("Span: "));
    QLabel *lOrder = new QLabel(tr("Order: "));
    _leSGOrder = new QLineEdit;
    _leSGOrder->setEnabled(false);
    _leSGOrder->setValidator(new QIntValidator(1,9));
    _leSGSpan = new QLineEdit;
    _leSGSpan->setEnabled(false);
    _leSGSpan->setValidator(new QIntValidator(5,99));
    gl->addWidget(_radMethod[method_sg],vpos++,0,1,2);
    gl->addWidget(lSpan,vpos,0,1,1);
    gl->addWidget(_leSGSpan,vpos++,1,1,1);
    gl->addWidget(lOrder,vpos,0,1,1);
    gl->addWidget(_leSGOrder,vpos++,1,1,1);
    gl->addItem(new QSpacerItem(1,10),vpos++,0,1,2);

    _radMethod[method_spline] = new QRadioButton();
    _radMethod[method_spline]->setText(tr("Spline (not implemented)"));
    connect(_radMethod[method_spline],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
    _radMethod[method_spline]->setChecked(false);
    _radMethod[method_spline]->setEnabled(false);
    gl->addWidget(_radMethod[method_spline],vpos++,0,1,2);
    gl->addItem(new QSpacerItem(1,10),vpos++,0,1,2);

    _radMethod[method_fourier] = new QRadioButton();
    _radMethod[method_fourier]->setText(tr("Fourier"));
    _radMethod[method_fourier]->setChecked(false);
    connect(_radMethod[method_fourier],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
    QLabel *lTresh = new QLabel(tr("Treshhold [Hz]: "));
    _leFTreshhold = new QLineEdit;
    _leFTreshhold->setEnabled(false);
    _leFTreshhold->setValidator(new QDoubleValidator(0,1e10,10));
    gl->addWidget(_radMethod[method_fourier],vpos++,0,1,2);
    gl->addWidget(lTresh,vpos,0,1,1);
    gl->addWidget(_leFTreshhold,vpos++,1,1,1);

    if ( _radMethod.size() > select && select >= 0 ) {
        _radMethod[select]->setChecked(true);
    }
    if ( _params.size() == 3 ) {
        _leSGOrder->setText(tr("%1").arg(_params[pl_order],0,'f',5));
        _leSGSpan->setText(tr("%1").arg(_params[pl_span],0,'f',5));
        _leFTreshhold->setText(tr("%1").arg(_params[pl_treshhold],0,'f',5));
    }

    _plotFreq = new QCustomPlot();
    _plotFreq->setVisible(true);
    _plotFreq->xAxis->setLabel(tr("Frequency / Hz"));
    _plotFreq->yAxis->setLabel(tr("Power / dB"));
    _plotFreq->setMinimumHeight(500);
    _plotFreq->setMinimumWidth(500);
    _plotFreq->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
    gl->addWidget(_plotFreq,0,2,vpos,1);

    return gl;
}

void EAQtAdvancedSmoothDialog::methodChanged()
{
    if ( _radMethod[method_sg]->isChecked() ) {
        _leFTreshhold->setEnabled(false);
        _leSGOrder->setEnabled(true);
        _leSGSpan->setEnabled(true);
    } else if ( _radMethod[method_spline]->isChecked() ) {
        _leFTreshhold->setDisabled(true);
        _leSGOrder->setDisabled(true);
        _leSGSpan->setDisabled(true);
    } else if ( _radMethod[method_fourier]->isChecked() ) {
        _leFTreshhold->setDisabled(false);
        _leSGOrder->setDisabled(true);
        _leSGSpan->setDisabled(true);
    }
}

void EAQtAdvancedSmoothDialog::updateFrequencyPlot()
{
    CurveCollection* cc = EAQtData::getInstance().getCurves();
    if ( EAQtData::getInstance().Act() == SELECT::none
    || cc->count() < 1 ) {
        return;
    }
    _plotFreq->clearGraphs();
    if ( _graphs.size() > 0 ) {
        for ( int i = 0; i<_graphs.size(); ++i ) {
            delete _graphs[i];
        }
    }
    _graphs.resize(0);

    if ( EAQtData::getInstance().Act() == SELECT::all ) {
        for ( uint32_t i = 0; i< cc->count(); ++i ) {
            updateCurveFrequency(cc->get(i));
        }
    } else {
        updateCurveFrequency(cc->get(EAQtData::getInstance().Act()));
    }
    _plotFreq->xAxis->rescale();
    _plotFreq->yAxis->rescale();
    _plotFreq->replot();
}

void EAQtAdvancedSmoothDialog::exec()
{
    _dialog->exec();
}

void EAQtAdvancedSmoothDialog::hide()
{
    _dialog->hide();
}

void EAQtAdvancedSmoothDialog::updateCurveFrequency(Curve* c)
{
    int index = _graphs.size();
    _graphs.push_back(_plotFreq->addGraph());
    _graphs[index]->setPen(QPen(COLOR::regular));
    _graphs[index]->setVisible(false);

    double samplingFreq;
    if ( EAQtData::getInstance().getXAxis() != XAXIS::nonaveraged ) {
        if ( c->Param(PARAM::method) != PARAM::method_lsv ) {
            samplingFreq = 1000 / (2*(c->Param(PARAM::tw) + c->Param(PARAM::tp)));
        } else {
            samplingFreq = 1000 / MEASUREMENT::LSVtime[c->Param(PARAM::dEdt)];
        }
    } else {
        samplingFreq = 1000*c->Param(PARAM::nonaveragedsampling);
        if ( samplingFreq == 0 ) {
            return;
        }
    }
    QVector<double> frequencies;
    QVector<double> img;
    QVector<double> real;

    //EAQtSignalProcessing::dft(samplingFreq,c->getYVector(),frequencies,real,img);
    EAQtSignalProcessing::kissFFT(samplingFreq,c->getYVector(),frequencies,real,img);

    QVector<double> power;
    power.resize(real.size());
    for ( int i =0; i<real.size();++i) {
        power[i] = log(pow(real[i],2) + pow(img[i],2));
    }
    int half = ceil(frequencies.size()/2);
    _graphs[index]->setData(frequencies.mid(0,half-1),power.mid(0,half-1));
    _graphs[index]->setVisible(true);
}
