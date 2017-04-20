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
    _graph = _plotFreq->addGraph();
    _graph->setPen(QPen(COLOR::regular));
    _graph->setVisible(false);
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
    int sel = 0;
    if ( EAQtData::getInstance().Act() >= 0 ) {
        sel = EAQtData::getInstance().Act();
    }
    Curve* c = EAQtData::getInstance().getCurves()->get(sel);
    if ( c == NULL ) {
        return;
    }

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

    EAQtSignalProcessing::dft(samplingFreq,c->getYVector(),frequencies,real,img);

    QVector<double> power;
    power.resize(real.size());
    for ( int i =0; i<real.size();++i) {
        power[i] = log(pow(real[i],2) + pow(img[i],2));
    }
    int half = ceil(frequencies.size()/2);
    _graph->setData(frequencies.mid(0,half-1),power.mid(0,half-1));
    _graph->setVisible(true);
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
