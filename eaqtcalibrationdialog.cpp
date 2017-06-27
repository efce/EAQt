
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
    _fontLabel = new QFont(_dialog->font());
    _fontLabel->setBold(true);
    QLabel *lname = new QLabel(tr("Name"));
    lname->setFont(*_fontLabel);
    QLabel *lcurr = new QLabel(tr("Current"));
    lcurr->setFont(*_fontLabel);

    QVBoxLayout* cb1 = new QVBoxLayout();
    QLabel *lconc = new QLabel(tr("Concentration"));
    cb1->addWidget(lconc);
    connect(_cSampleConcUnits,SIGNAL(currentIndexChanged(int)), this, SLOT(recalculateConc()));
    cb1->addWidget(_cSampleConcUnits);
    lconc->setFont(*_fontLabel);

    QVBoxLayout* cb2 = new QVBoxLayout();
    _labelAddition = new QLabel(tr("Addition"));
    _labelAddition->setFont(*_fontLabel);
    _labelAddition->setVisible(false);
    cb2->addWidget(_labelAddition);
    connect(_cAdditionVolumeUnits,SIGNAL(currentIndexChanged(int)), this, SLOT(recalculateConc()));
    _cAdditionVolumeUnits->setVisible(false);
    cb2->addWidget(_cAdditionVolumeUnits);

    lconc->setFont(*_fontLabel);
    gl->addWidget(lname,0,0,1,1);
    gl->addWidget(lcurr,0,1,1,1);
    gl->addLayout(cb1,0,2,1,1);
    gl->addLayout(cb2,0,3,1,1);
    QDoubleValidator *validator = new QDoubleValidator(0.0,999999.9,7);
    validator->setNotation(QDoubleValidator::StandardNotation);
    for ( i = 0; i<_cd->yValues.size(); ++i ) {
        QLabel *l1 = new QLabel(_cd->curves->get(i)->CName() + ": ");
        QLabel *l2 = new QLabel(EAQtData::getInstance().dispI(_cd->yValues[i]) + " ");
        this->_leConcentrations[i] = new QLineEdit();
        this->_leConcentrations[i]->setValidator(validator);
        if ( _cd->xValues.size() == _cd->yValues.size() ) {
            _leConcentrations[i]->setText(tr("%1").arg(_cd->xValues[i],0,'f',8));
        } else {
            this->_leConcentrations[i]->setText("0.0");
        }
        this->_leConcentrations[i]->setMaxLength(12);
        this->_leConcentrations[i]->setFixedWidth(QFontMetrics(_dialog->font()).width("9999999999"));
        _leConcentrations[i]->setStyleSheet(":enabled { background-color: white; color: black; } :disabled {background-color: light gray; color: black }" );

        this->_leAdditionVolumes[i] = new QLineEdit();
        this->_leAdditionVolumes[i]->setValidator(validator);
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
    QPushButton *butCancel = new QPushButton(tr("Close"));
    glm->addWidget(butCal,2,0,1,1);
    glm->addWidget(butCancel,2,1,1,1);
    _calibrationPlot = new CalibrationPlot(_cd);
    _calibrationPlot->setVisible(false);
    glm->addWidget(_calibrationPlot,0,2,2,1);

    _butSaveCal = new QPushButton(tr("Save calibration"));
    _butSaveCal->setVisible(false);
    connect(_butSaveCal,SIGNAL(clicked(bool)),this,SLOT(saveCalibration()));
    glm->addWidget(_butSaveCal,2,2,1,1);

    connect(butCancel,SIGNAL(clicked(bool)),this->_dialog,SLOT(close()));
    connect(butCal,SIGNAL(clicked(bool)), this,SLOT(drawCalibration()));
    this->_dialog->setLayout(glm);
    _dialog->setMinimumHeight(600);
}

EAQtCalibrationDialog::~EAQtCalibrationDialog()
{
    delete _dialog;
    delete _fontLabel;
}

void EAQtCalibrationDialog::exec()
{
   this->_dialog->exec();
}

void EAQtCalibrationDialog::drawCalibration()
{
    //TODO: confidence intervals instead of standard deviation
    _cd->xValues.resize(_leConcentrations.size());
    int csize = _cd->xValues.size();
    for ( int i = 0; i<csize; ++i) {
        _cd->xValues.replace(i,_leConcentrations[i]->text().toDouble());
    }
    EAQtSignalProcessing::correlation(_cd->xValues,_cd->yValues, &(_cd->correlationCoef));
    EAQtSignalProcessing::linearRegression(_cd->xValues,_cd->yValues,&(_cd->slope),&(_cd->slopeStdDev),&(_cd->intercept),&(_cd->interceptStdDev),&(_cd->x0StdDev));
    _cd->wasFitted = true;
    _cd->xUnits = _cSampleConcUnits->currentText();
    _cd->yUnits = "µA";
    _cd->x0value = _cd->intercept/_cd->slope;
    _calibrationPlot->update();
    _calibrationPlot->setVisible(true);
    _butSaveCal->setVisible(true);
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
    _fd = new QFileDialog(0,tr("Save Calibration"),"",QString(".eacal (*.eacal)"));
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
