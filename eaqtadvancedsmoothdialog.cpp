
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

EAQtAdvancedSmoothDialog::EAQtAdvancedSmoothDialog(int lastUsed, const std::vector<double>& params, EAQtUIInterface *pui) : QObject()
{
    _params = params;
    _pUI = pui;

    _dialog = new QDialog();
    _dialog->setModal(true);
    _dialog->setWindowTitle(tr("Advanced curve smoothing"));
    _dialog->setLayout(generateLayout(lastUsed));
    methodChanged();
    updateFrequencyPlot();
}

EAQtAdvancedSmoothDialog::~EAQtAdvancedSmoothDialog()
{
    delete _dialog;
}


QGridLayout* EAQtAdvancedSmoothDialog::generateLayout(int select)
{
    QGridLayout* gl = new QGridLayout();

    int vpos=0;
    _radMethod.resize(4);

    _radMethod[method_sg] = new QRadioButton();
    _radMethod[method_sg]->setText(tr("Savitzky-Golay"));
    _radMethod[method_sg]->setChecked(true);
    connect(_radMethod[method_sg],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
    QLabel *lSpan = new QLabel(tr("Span: "));
    QLabel *lOrder = new QLabel(tr("Order: "));
    _leSGOrder = new QLineEdit;
    _leSGOrder->setEnabled(false);
    _leSGOrder->setValidator(new QIntValidator(1,9));
    _leSGOrder->setText("3");
    _leSGSpan = new QLineEdit;
    _leSGSpan->setEnabled(false);
    _leSGSpan->setValidator(new QIntValidator(5,99));
    _leSGSpan->setText("13");
    gl->addWidget(_radMethod[method_sg],vpos++,0,1,2);
    gl->addWidget(lSpan,vpos,0,1,1);
    gl->addWidget(_leSGSpan,vpos++,1,1,1);
    gl->addWidget(lOrder,vpos,0,1,1);
    gl->addWidget(_leSGOrder,vpos++,1,1,1);
    gl->addItem(new QSpacerItem(1,10),vpos++,0,1,2);

    _radMethod[method_median] = new QRadioButton();
    _radMethod[method_median]->setText(tr("Median filter"));
    _radMethod[method_median]->setChecked(false);
    connect(_radMethod[method_median],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
    gl->addWidget(_radMethod[method_median],vpos++,0,1,2);
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

    _butClose = new QPushButton(tr("Close"));
    connect(_butClose,SIGNAL(clicked(bool)),_dialog,SLOT(close()));
    _butApply = new QPushButton(tr("Apply"));
    connect(_butApply,SIGNAL(clicked(bool)),this,SLOT(apply()));
    gl->addWidget(_butApply,++vpos,0,1,1);
    gl->addWidget(_butClose,vpos,1,1,1);

    return gl;
}

void EAQtAdvancedSmoothDialog::methodChanged()
{
    if ( _radMethod[method_sg]->isChecked() ) {
        _leFTreshhold->setEnabled(false);
        _leSGOrder->setEnabled(true);
        _leSGSpan->setEnabled(true);
    } else if ( _radMethod[method_median]->isChecked() ) {
        _leFTreshhold->setDisabled(true);
        _leSGOrder->setDisabled(true);
        _leSGSpan->setDisabled(true);
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
    _graphs.clear();
    _samplingFreq.clear();
    _frequencies.clear();
    _realValues.clear();
    _imgValues.clear();

    if ( EAQtData::getInstance().Act() == SELECT::all ) {
        for ( int32_t i = 0; i< cc->count(); ++i ) {
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
            samplingFreq = 1000.0 / (2.0*(c->Param(PARAM::tw) + c->Param(PARAM::tp)));
        } else {
            samplingFreq = 1000.0 / (double)MEASUREMENT::LSVtime[c->Param(PARAM::dEdt)];
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
    _frequencies.push_back(frequencies);
    _imgValues.push_back(img);
    _realValues.push_back(real);
    _samplingFreq.push_back(samplingFreq);

    QVector<double> power;
    power.resize(real.size());
    for ( int i =0; i<real.size();++i) {
        power[i] = log(pow(real[i],2) + pow(img[i],2));
    }
    int half = ceil(frequencies.size()/2);
    _graphs[index]->setData(frequencies.mid(0,half-1),power.mid(0,half-1));
    _graphs[index]->setVisible(true);
}

void EAQtAdvancedSmoothDialog::apply()
{
    int selected = -1;
    for ( int i = 0; i<_radMethod.size(); ++i ) {
        if (_radMethod[i]->isChecked() ) {
            selected = i;
            break;
        }
    }
    switch (selected) {
    case method_sg:
        {
            int order = _leSGOrder->text().toInt();
            int span = _leSGSpan->text().toInt();
            if ( order >= span ) {
                QMessageBox mb(_dialog);
                mb.setText(tr("Span has to be larger than order."));
                mb.exec();
                return;
            }
            if ( EAQtData::getInstance().Act() == SELECT::all ) {
                CurveCollection* cc = EAQtData::getInstance().getCurves();
                Curve *c;
                for ( int32_t i = 0; i<cc->count(); ++i) {
                    c = cc->get(i);
                    if ( c != nullptr ) {
                        trySG(c,span,order);
                    }
                }
            } else {
                Curve *c = EAQtData::getInstance().getCurves()->get(EAQtData::getInstance().Act());
                if ( c != nullptr ) {
                trySG(c, span, order);
                }
            }

            break;
        }
    case method_median:
        {
            if ( EAQtData::getInstance().Act() == SELECT::all ) {
                CurveCollection* cc = EAQtData::getInstance().getCurves();
                Curve *c;
                for ( int32_t i = 0; i<cc->count(); ++i) {
                    c = cc->get(i);
                    if ( c != nullptr ) {
                        tryMedian(c, 5);
                    }
                }
            } else {
                Curve *c = EAQtData::getInstance().getCurves()->get(EAQtData::getInstance().Act());
                if ( c != nullptr ) {
                    tryMedian(c, 5);
                }
            }
            break;
        }

    case method_fourier:
        {
            double threshold = _leFTreshhold->text().toDouble();
            if ( EAQtData::getInstance().Act() == SELECT::all ) {
                for ( int i = 0; i <_frequencies.size(); ++i ) {
                    Curve *c = EAQtData::getInstance().getCurves()->get(i);
                    tryIFFT(c,threshold, _samplingFreq[i],_frequencies[i],_imgValues[i], _realValues[i]);
                }
            } else {
                Curve *c = EAQtData::getInstance().getCurves()->get(EAQtData::getInstance().Act());
                tryIFFT(c, threshold, _samplingFreq[0],_frequencies[0],_imgValues[0], _realValues[0]);
            }

            break;
        }

    default:
    case method_spline:
        break;
    }
    _pUI->updateAll(true);
    updateFrequencyPlot();
}

void EAQtAdvancedSmoothDialog::trySG(Curve *c, int span, int order)
{
    if ( c->Param(PARAM::ptnr) <= span ) {
        QMessageBox mb(_dialog);
        mb.setText(tr("Span has to be smaller than number of points in curve."));
        mb.exec();
        return;
    }
    QVector<double> y = c->getYVector();
    EAQtSignalProcessing::sgSmooth(&y,order,span);
    int32_t sy = y.size();
    for ( int32_t i = 0; i<sy;++i) {
        c->setYValue(i,y[i]);
    }

}

void EAQtAdvancedSmoothDialog::tryMedian(Curve *c, int windowSize)
{
    if ( c->Param(PARAM::ptnr) <= windowSize ) {
        QMessageBox mb(_dialog);
        mb.setText(tr("Curve has to be larger than %1.").arg(windowSize));
        mb.exec();
        return;
    }
    QVector<double> y = c->getYVector();
    QVector<double> res;
    EAQtSignalProcessing::medianfilter(y, res, windowSize);
    int32_t sres = res.size();
    for ( int32_t i = 0; i<sres;++i) {
        c->setYValue(i,res[i]);
    }
}

void EAQtAdvancedSmoothDialog::tryIFFT(Curve* c, double treshhold, double samplingFreq, QVector<double>& freq, QVector<double>& vImg, QVector<double>& vReal)
{

        int threshPos = -1;
        int32_t points = freq.size();
        for ( int32_t ii = 0; ii<points; ++ii ) {
            if ( freq.at(ii) > treshhold ) {
                threshPos = ii;
                break;
            }
        }
        if ( threshPos == -1 ) {
            //TODO: error?
            return;
        }
        int32_t rangeDN = threshPos;
        int32_t rangeUP = freq.size() - threshPos;
        for ( int32_t ii = rangeDN; ii<=rangeUP; ++ii ) {
            vImg.replace(ii,0.0);
            vReal.replace(ii,0);
        }
        QVector<double> newy;
        EAQtSignalProcessing::kissIFFT(samplingFreq, vImg, vReal, newy);

        for ( int32_t ii=0; ii<points;++ii) {
            c->setYValue(ii,newy[ii]);
        }
}
