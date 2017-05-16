
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

EAQtCalibrationDialog::EAQtCalibrationDialog(CalibrationData *cd, QHash<QString,QString>* oldSettings)
{
    if ( cd->yValues.size() != cd->curves->count() ) {
        throw 1;
    }

    if ( cd->yValues.size() != cd->xValues.size() ) {
        cd->xValues.clear();
    }

    _cd = cd;

    QString cdot = QChar(0x00B7);
    QString sup1 = QChar(0x207B);
    sup1.append(QChar(0x00B9));

    _vVolumes.push_back(multipliers{ 1.0, "L"});
    _vVolumes.push_back(multipliers{ 0.01, "cL"});
    _vVolumes.push_back(multipliers{ 0.001, "mL"});
    _vVolumes.push_back(multipliers{ 0.000001, "µL"});

    _vConcs.push_back( multipliers{1.0,"g" + cdot + "L" + sup1} );
    _vConcs.push_back( multipliers{0.001,"mg" + cdot+ "L" + sup1} );
    _vConcs.push_back( multipliers{0.000001,"µg" + cdot + "L" + sup1} );
    _vConcs.push_back( multipliers{0.000000001,"ng" + cdot + "L" + sup1} );
    _vConcs.push_back( multipliers{1.0,"mg" + cdot + "mL" + sup1} );
    _vConcs.push_back( multipliers{0.001,"µg" + cdot + "mL" + sup1} );
    _vConcs.push_back( multipliers{0.000001,"ng" + cdot + "mL" + sup1} );

    if ( oldSettings->size() != 4 ) {
        oldSettings->reserve(4);
        oldSettings->insert("avu",_vVolumes[3].name);
        oldSettings->insert("svu",_vVolumes[2].name);
        oldSettings->insert("stdcu", _vConcs[1].name);
        oldSettings->insert("samcu", _vConcs[2].name);
    }

    _settings = oldSettings;

    _cAdditionVolumeUnits = new QComboBox();
    for ( int i=0; i<_vVolumes.size(); ++i) {
        _cAdditionVolumeUnits->addItem(_vVolumes[i].name);
        if ( _vVolumes[i].name.compare(oldSettings->value("avu")) == 0 ) {
            _cAdditionVolumeUnits->setCurrentIndex(i);
        }
    }

    _cSampleVolumeUnits = new QComboBox();
    for ( int i=0; i<_vVolumes.size(); ++i) {
        _cSampleVolumeUnits->addItem(_vVolumes[i].name);
        if ( _vVolumes[i].name.compare(oldSettings->value("scu")) == 0 ) {
            _cSampleVolumeUnits->setCurrentIndex(i);
        }
    }

    _cStandardConcUnits = new QComboBox();
    for ( int i=0; i<_vConcs.size(); ++i) {
        _cStandardConcUnits->addItem(_vConcs[i].name);
        if ( _vConcs[i].name.compare(oldSettings->value("stdcu")) == 0 ) {
            _cStandardConcUnits->setCurrentIndex(i);
        }
    }

    _cSampleConcUnits = new QComboBox();
    for ( int i=0; i<_vConcs.size(); ++i) {
        _cSampleConcUnits->addItem(_vConcs[i].name);
        if ( _vConcs[i].name.compare(oldSettings->value("samcu")) == 0 ) {
            _cSampleConcUnits->setCurrentIndex(i);
        }
    }

    _calculateBox = new QGroupBox();
    QGridLayout *calcLayout = new QGridLayout();
    _calculateConc = new QCheckBox(tr("Calculate concentrations"));
    _calculateConc->setChecked(false);
    _calculateConc->setVisible(true);
    calcLayout->addWidget(_calculateConc,0,0,1,3);
    _lSampleVolume = new QLabel(tr("Sample volume: "));
    _lSampleVolume->setVisible(false);
    _leSampleVolume = new QLineEdit();
    _leSampleVolume->setText("0.0");
    _leSampleVolume->setValidator(new QDoubleValidator(0.0,999999.9,10));
    _leSampleVolume->setVisible(false);
    connect(_leSampleVolume,SIGNAL(textChanged(QString)), this, SLOT(recalculateConc()));
    _cSampleVolumeUnits->setVisible(false);
    connect(_cSampleVolumeUnits,SIGNAL(currentIndexChanged(int)), this, SLOT(recalculateConc()));
    calcLayout->addWidget(_lSampleVolume,1,0,1,1);
    calcLayout->addWidget(_leSampleVolume,1,1,1,1);
    calcLayout->addWidget(_cSampleVolumeUnits,1,2,1,1);
    _lStandardConc = new QLabel(tr("Standard conc: "));
    _lStandardConc->setVisible(false);
    _leStandardConc = new QLineEdit();
    _leStandardConc->setText("0.0");
    _leStandardConc->setValidator(new QDoubleValidator(0.0,999999.9,10));
    _leStandardConc->setVisible(false);
    connect(_leStandardConc,SIGNAL(textChanged(QString)), this, SLOT(recalculateConc()));
    _cStandardConcUnits->setVisible(false);
    connect(_cStandardConcUnits,SIGNAL(currentIndexChanged(int)), this, SLOT(recalculateConc()));
    calcLayout->addWidget(_lStandardConc,2,0,1,1);
    calcLayout->addWidget(_leStandardConc,2,1,1,1);
    calcLayout->addWidget(_cStandardConcUnits,2,2,1,1);
    _calculateBox->setLayout(calcLayout);
    connect(_calculateConc,SIGNAL(toggled(bool)),this,SLOT(toggleCalculateConc(bool)));

    QGridLayout *gl = new QGridLayout();
    QGridLayout *glm = new QGridLayout();
    QScrollArea *sa = new QScrollArea();
    sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    sa->setFixedWidth(400);
    sa->setMaximumHeight(600);
    this->_dialog = new QDialog();
    this->_leConcentrations.resize(_cd->yValues.size());
    this->_leAdditionVolumes.resize(_cd->yValues.size());
    uint i;
    QFont *fontLabel = new QFont(_dialog->font());
    fontLabel->setBold(true);
    QLabel *lname = new QLabel(tr("Name"));
    lname->setFont(*fontLabel);
    QLabel *lcurr = new QLabel(tr("Current"));
    lcurr->setFont(*fontLabel);

    QVBoxLayout* cb1 = new QVBoxLayout();
    QLabel *lconc = new QLabel(tr("Concentration"));
    cb1->addWidget(lconc);
    connect(_cSampleConcUnits,SIGNAL(currentIndexChanged(int)), this, SLOT(recalculateConc()));
    cb1->addWidget(_cSampleConcUnits);
    lconc->setFont(*fontLabel);

    QVBoxLayout* cb2 = new QVBoxLayout();
    _labelAddition = new QLabel(tr("Addition"));
    _labelAddition->setFont(*fontLabel);
    _labelAddition->setVisible(false);
    cb2->addWidget(_labelAddition);
    connect(_cAdditionVolumeUnits,SIGNAL(currentIndexChanged(int)), this, SLOT(recalculateConc()));
    _cAdditionVolumeUnits->setVisible(false);
    cb2->addWidget(_cAdditionVolumeUnits);

    lconc->setFont(*fontLabel);
    gl->addWidget(lname,0,0,1,1);
    gl->addWidget(lcurr,0,1,1,1);
    gl->addLayout(cb1,0,2,1,1);
    gl->addLayout(cb2,0,3,1,1);
    for ( i = 0; i<_cd->yValues.size(); ++i ) {
        QLabel *l1 = new QLabel(_cd->curves->get(i)->CName() + ": ");
        QLabel *l2 = new QLabel(EAQtData::getInstance().dispI(_cd->yValues[i]) + " ");
        this->_leConcentrations[i] = new QLineEdit();
        this->_leConcentrations[i]->setValidator(new QDoubleValidator(0.0,999999.9,10));
        if ( _cd->xValues.size() == _cd->yValues.size() ) {
            _leConcentrations[i]->setText(tr("%1").arg(_cd->xValues[i],0,'f',8));
        } else {
            this->_leConcentrations[i]->setText("0.0");
        }
        this->_leConcentrations[i]->setMaxLength(12);
        this->_leConcentrations[i]->setFixedWidth(QFontMetrics(_dialog->font()).width("9999999999"));
        _leConcentrations[i]->setStyleSheet(":enabled { background-color: white; color: black; } :disabled {background-color: light gray; color: black }" );

        this->_leAdditionVolumes[i] = new QLineEdit();
        this->_leAdditionVolumes[i]->setValidator(new QDoubleValidator(0.0,999999.9,10));
        this->_leAdditionVolumes[i]->setText("0.0");
        this->_leAdditionVolumes[i]->setMaxLength(12);
        this->_leAdditionVolumes[i]->setFixedWidth(QFontMetrics(_dialog->font()).width("9999999999"));
        this->_leAdditionVolumes[i]->setVisible(false);
        connect(_leAdditionVolumes[i], SIGNAL(textChanged(QString)), this, SLOT(recalculateConc()));

        gl->addWidget(l1,i+1,0,1,1,Qt::AlignRight);
        gl->addWidget(l2,i+1,1,1,1);
        gl->addWidget(this->_leConcentrations[i],i+1,2,1,1);
        gl->addWidget(_leAdditionVolumes[i],i+1,3,1,1);
    }
    _scrollAreaWidget = new QWidget();
    _scrollAreaWidget->setLayout(gl);
    _scrollAreaWidget->setFixedWidth(300);
    sa->setLayout(new QVBoxLayout);
    sa->setWidget(_scrollAreaWidget);
    glm->addWidget(_calculateBox,0,0,1,2);
    glm->addWidget(sa,1,0,1,2);
    QPushButton *butCal = new QPushButton(tr("Plot calibration"));
    QPushButton *butCancel = new QPushButton(tr("Cancel"));
    glm->addWidget(butCal,2,0,1,1);
    glm->addWidget(butCancel,2,1,1,1);
    QWidget* qw = preparePlot();
    glm->addWidget(qw,0,2,3,1);
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
    //TODO: confidence intervals instead of standard deviation
    double x0StdDev = -1;
    _cd->xValues.resize(_leConcentrations.size());
    int csize = _cd->xValues.size();
    for ( int i = 0; i<csize; ++i) {
        _cd->xValues.replace(i,_leConcentrations[i]->text().toDouble());
    }
    EAQtSignalProcessing::correlation(_cd->xValues,_cd->yValues, &(_cd->correlationCoef));
    EAQtSignalProcessing::linearRegression(_cd->xValues,_cd->yValues,&(_cd->slope),&(_cd->slopeStdDev),&(_cd->intercept),&(_cd->interceptStdDev),&(x0StdDev));
    _cd->wasFitted = true;
    _calibrationPlot->setVisible(true);
    _calibrationPoints->setData(_cd->xValues,_cd->yValues,false);
    double x0 = -_cd->intercept/_cd->slope;
    _calibrationLine->point1->setCoords(1,_cd->slope*1+_cd->intercept);
    _calibrationLine->point2->setCoords(x0, 0);
    _calibrationPlot->rescaleAxes();
    if ( _calibrationPlot->xAxis->range().lower > x0 ) {
        _calibrationPlot->xAxis->setRangeLower(x0);
    }
    if ( _calibrationPlot->yAxis->range().lower > 0 ) {
        _calibrationPlot->yAxis->setRangeLower(0);
    }
    _calibrationPlot->xAxis->setLabel("c / " + _cSampleConcUnits->currentText());
    double spanx = _calibrationPlot->xAxis->range().upper - _calibrationPlot->xAxis->range().lower;
    _calibrationPlot->xAxis->setRangeLower(_calibrationPlot->xAxis->range().lower - (0.1*spanx));
    _calibrationPlot->xAxis->setRangeUpper(_calibrationPlot->xAxis->range().upper + (0.1*spanx));
    double spany = _calibrationPlot->yAxis->range().upper - _calibrationPlot->yAxis->range().lower;
    _calibrationPlot->yAxis->setRangeLower(_calibrationPlot->yAxis->range().lower - (0.1*spany));
    _calibrationPlot->yAxis->setRangeUpper(_calibrationPlot->yAxis->range().upper + (0.1*spany));
    _calibrationPlot->replot();
    _calibrationR->setText(tr("r = %1").arg(_cd->correlationCoef,0,'f',4));
    _calibrationEq->setText(tr("i = %1(±%2)c + %3(±%4)").arg(_cd->slope,0,'f',4).arg(_cd->slopeStdDev,0,'f',4).arg(_cd->intercept,0,'f',4).arg(_cd->interceptStdDev,0,'f',4));
    if ( x0StdDev > -1 ) {
        _additionResult->setText(tr("result: (%1±%2) %3").arg(-x0,0,'f',4).arg(x0StdDev,0,'f',4).arg(_cSampleConcUnits->currentText()));
        _additionResult->setVisible(true);
    } else {
        _additionResult->setVisible(false);
    }
    _cd->xUnits = _cSampleConcUnits->currentText();
    _cd->yUnits = "µA";
    _calibrationEq->setVisible(true);
    _calibrationR->setVisible(true);
    _butSaveCal->setVisible(true);
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
    _additionResult = new QLineEdit();
    _additionResult->setVisible(false);
    _additionResult->setReadOnly(true);
    _additionResult->setAlignment(Qt::AlignCenter);
    _calibrationPlot = new QCustomPlot();
    _calibrationPlot->xAxis->setLabel(tr("c"));
    _calibrationPlot->yAxis->setLabel(tr("i / µA"));
    _calibrationPlot->setVisible(false);
    _calibrationPlot->setMinimumWidth(500);
    _calibrationPlot->setMinimumHeight(450);
    //connect(_calibrationPlot,SIGNAL(beforeReplot()),this,SLOT(beforeReplot()));
    QPen pen;
    pen.setColor(COLOR::measurement);
    pen.setWidth(1);
    _calibrationPlot->xAxis->grid()->setZeroLinePen(pen);
    _calibrationPlot->yAxis->grid()->setZeroLinePen(pen);
    _calibrationLine = new QCPItemStraightLine(_calibrationPlot);
    _calibrationLine->setPen(QPen(QColor(COLOR::active)));
    _calibrationPoints = _calibrationPlot->addGraph();
    _calibrationPoints->setLineStyle(QCPGraph::lsNone);
    _calibrationPoints->setScatterStyle(QCPScatterStyle::ssCircle);
    _calibrationPoints->setPen(QPen(COLOR::regular));
    _butSaveCal = new QPushButton(tr("Save calibration"));
    _butSaveCal->setVisible(false);
    connect(_butSaveCal,SIGNAL(clicked(bool)),this,SLOT(saveCalibration()));
    gl->addWidget(_calibrationPlot,0,0,1,1);
    gl->addWidget(_calibrationEq,1,0,1,1);
    gl->addWidget(_calibrationR,2,0,1,1);
    gl->addWidget(_additionResult,3,0,1,1);
    gl->addWidget(_butSaveCal,4,0,1,1);
    w->setLayout(gl);
    return w;
}

void EAQtCalibrationDialog::beforeReplot()
{
    int pxx = _calibrationPlot->yAxis->coordToPixel(0);
    int pxy = _calibrationPlot->xAxis->coordToPixel(0);
    _calibrationPlot->xAxis->setOffset(-_calibrationPlot->axisRect()->height()-_calibrationPlot->axisRect()->top()+pxx);
    _calibrationPlot->yAxis->setOffset(_calibrationPlot->axisRect()->left()-pxy);
}

void EAQtCalibrationDialog::toggleCalculateConc(bool status)
{
    for ( int i = 0; i<_leConcentrations.size(); ++i) {
        _leConcentrations[i]->setDisabled(status);
        _leAdditionVolumes[i]->setVisible(status);
    }
    _lSampleVolume->setVisible(status);
    _leSampleVolume->setVisible(status);
    _cSampleVolumeUnits->setVisible(status);

    _lStandardConc->setVisible(status);
    _leStandardConc->setVisible(status);
    _cStandardConcUnits->setVisible(status);

    _labelAddition->setVisible(status);
    _cAdditionVolumeUnits->setVisible(status);

    if ( status ) {
        recalculateConc();
        _scrollAreaWidget->setFixedWidth(380);
    } else {
        _scrollAreaWidget->setFixedWidth(300);
    }

}

void EAQtCalibrationDialog::recalculateConc()
{
    _settings->insert("avu",_cAdditionVolumeUnits->currentText());
    _settings->insert("svu",_cSampleVolumeUnits->currentText());
    _settings->insert("stdcu",_cStandardConcUnits->currentText());
    _settings->insert("samcu",_cSampleConcUnits->currentText());
    _cd->xValues.resize(_leConcentrations.size());

    if ( !_calculateConc->isChecked() ) {
        return;
    }
    int i;
    double volSample = _leSampleVolume->text().toDouble();
    for ( i=0; i<_vVolumes.size(); ++i ) {
        if ( _cSampleVolumeUnits->currentText().compare(_vVolumes[i].name) == 0 ) {
            volSample *= _vVolumes[i].multiply;
            break;
        }
    }

    double stdConc = _leStandardConc->text().toDouble();
    for ( i=0; i<_vConcs.size(); ++i ) {
        if ( _cStandardConcUnits->currentText().compare(_vConcs[i].name) == 0 ) {
            stdConc *= _vConcs[i].multiply;
            break;
        }
    }

    double multiplyConc = 1;
    for ( i=0; i<_vConcs.size(); ++i ) {
        if ( _cSampleConcUnits->currentText().compare(_vConcs[i].name) == 0 ) {
            multiplyConc = _vConcs[i].multiply;
            break;
        }
    }

    double multiplyVol = 1;
    for ( i=0; i<_vVolumes.size(); ++i ) {
        if ( _cAdditionVolumeUnits->currentText().compare(_vVolumes[i].name) == 0 ) {
            multiplyVol *= _vVolumes[i].multiply;
            break;
        }
    }

    double work;
    double v;
    for ( i=0; i<_leConcentrations.size(); ++i ) {
        v = _leAdditionVolumes[i]->text().toDouble() * multiplyVol;
        if ( (v + volSample) == 0 ) {
            work = 0.0;
        } else {
            work = (v*stdConc) / (v+volSample);
        }
        _leConcentrations[i]->setText(tr("%1").arg(work/multiplyConc, 0, 'f', 6));
    }
}

void EAQtCalibrationDialog::saveCalibration()
{
    _fd = new QFileDialog(0,tr("Save Calibration"),"",tr(".eacal (.eacal)"));
    _fd->setModal(true);
    _fd->setOption(QFileDialog::DontUseNativeDialog);
    _fd->setLabelText( QFileDialog::Accept, tr("Save"));
    QGridLayout* l = (QGridLayout*) _fd->layout();
    _cbIncludeCurves = new QCheckBox(tr("Include original curves"));
    _cbIncludeCurves->setChecked(false);
    l->addWidget(_cbIncludeCurves,4,0,1,4);
    connect(_fd,SIGNAL(fileSelected(QString)),this,SLOT(saveInFile(QString)));
    _fd->exec();
}

void EAQtCalibrationDialog::saveInFile(QString fileName)
{
    if ( fileName.right(6).compare(".eacal",Qt::CaseInsensitive) != 0 ) {
        fileName.append(".eacal");
    }
    QFile f(fileName);
    if ( !f.open(QIODevice::ReadWrite) ) {
        //TODO: error
    }
    bool includeCurves = _cbIncludeCurves->isChecked();
    _cd->save(&f,includeCurves);
    f.close();
}
