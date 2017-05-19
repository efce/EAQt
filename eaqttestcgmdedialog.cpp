
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
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include "eaqttestcgmdedialog.h"


EAQtTestCGMDEDialog::EAQtTestCGMDEDialog() : QObject()
{
    _values = &EAQtData::getInstance()._testCGMDE;
    _vResults.resize(0);

    _dialog = new QDialog();
    _dialog->setFixedWidth(550);
    _dialog->setFixedHeight(170);
    QHBoxLayout* mainLay = new QHBoxLayout();
    _dialog->setWindowTitle(tr("CGMDE test"));
    _dialog->setModal(true);

    QGroupBox *params = new QGroupBox(tr("Test parameters"));
    QGridLayout *glparams = new QGridLayout();
    QLabel *lValveTime = new QLabel(tr("Valve time [ms]: "));
    _leValveTime = new QLineEdit();
    _leValveTime->setText(tr("%1").arg(_values->GP));
    _leValveTime->setValidator(new QIntValidator(0,999,_dialog));
    glparams->addWidget(lValveTime,0,0,1,1);
    glparams->addWidget(_leValveTime,0,1,1,1);
    QLabel *lBreakTime = new QLabel(tr("Break time [ms]: "));
    _leBreakTime = new QLineEdit();
    _leBreakTime->setText(tr("%1").arg(_values->BT));
    _leBreakTime->setValidator(new QIntValidator(0,9999,_dialog));
    glparams->addWidget(lBreakTime,1,0,1,1);
    glparams->addWidget(_leBreakTime,1,1,1,1);
    _lePotential = new QLineEdit();
    _lePotential->setText(tr("%1").arg(_values->ElPot));
    _lePotential->setEnabled(_values->ChPot);
    _cbUsePotential = new QCheckBox("Potential [mV]: ");
    _cbUsePotential->setChecked(_values->ChPot);
    connect(_cbUsePotential,SIGNAL(toggled(bool)),_lePotential,SLOT(setEnabled(bool)));
    glparams->addWidget(_cbUsePotential,2,0,1,1);
    glparams->addWidget(_lePotential,2,1,1,1);
    params->setLayout(glparams);
    mainLay->addWidget(params);

    QGroupBox *results = new QGroupBox(tr("Test results"));
    QGridLayout *glres = new QGridLayout();
    QLabel *lTestsNum = new QLabel(tr("Number of tests: "));
    _leTestsNum = new QLineEdit();
    _leTestsNum->setReadOnly(true);
    glres->addWidget(lTestsNum,0,0,1,1);
    glres->addWidget(_leTestsNum,0,1,1,1);

    QLabel *lAverage = new QLabel(tr("Average: "));
    _leAverageResult = new QLineEdit();
    _leAverageResult->setReadOnly(true);
    glres->addWidget(lAverage,1,0,1,1);
    glres->addWidget(_leAverageResult,1,1,1,1);

    QLabel *lStd = new QLabel(tr("Standard dev:"));
    _leStandardDev = new QLineEdit();
    _leStandardDev->setReadOnly(true);
    glres->addWidget(lStd,2,0,1,1);
    glres->addWidget(_leStandardDev,2,1,1,1);

    QLabel *lPulses = new QLabel(tr("Number of pulses: "));
    _lePulseNum = new QLineEdit();
    _lePulseNum->setReadOnly(true);
    glres->addWidget(lPulses,3,0,1,1);
    glres->addWidget(_lePulseNum,3,1,1,1);

    results->setLayout(glres);
    mainLay->addWidget(results);

    QGroupBox *actions = new QGroupBox();
    QVBoxLayout *vbl = new QVBoxLayout();
    _butReset = new QPushButton(tr("Reset"));
    connect(_butReset,SIGNAL(clicked(bool)),this,SLOT(resetResults()));

    _butClose = new QPushButton(tr("Close"));
    connect(_butClose,SIGNAL(clicked(bool)),this,SLOT(close()));

    _butTestStart = new QPushButton(tr("Start test"));
    connect(_butTestStart,SIGNAL(clicked(bool)),this,SLOT(TestStart()));

    _butTestStop = new QPushButton(tr("Stop test"));
    _butTestStop->setDisabled(true);
    connect(_butTestStop,SIGNAL(clicked(bool)),this,SLOT(TestStop()));

    vbl->addWidget(_butReset);
    vbl->addWidget(_butTestStart);
    vbl->addWidget(_butTestStop);
    vbl->addWidget(_butClose);
    actions->setLayout(vbl);
    mainLay->addWidget(actions);

    _dialog->setLayout(mainLay);
    resetResults();
}

EAQtTestCGMDEDialog::~EAQtTestCGMDEDialog()
{
    _dialog->close();
    delete _dialog;
}

void EAQtTestCGMDEDialog::show()
{
    _dialog->show();
    return;
}

void EAQtTestCGMDEDialog::close()
{
    if ( EAQtData::getInstance().isMeasurement() ) {
        TestStop();
    }
    _dialog->close();
    return;
}

void EAQtTestCGMDEDialog::resetResults()
{
    _testsNum = 0;
    _pulseNum = 0;
    _stardardDev = 0;
    _averageResult = 0;
    _vResults.clear();
    updateDialog();
}

void EAQtTestCGMDEDialog::updateDialog()
{
    _leTestsNum->setText(tr("%1").arg(_testsNum));
    _leAverageResult->setText(tr("%1").arg(_averageResult,0,'f',2));
    _leStandardDev->setText(tr("%1").arg(_stardardDev,0,'f',2));
    _lePulseNum->setText(tr("%1").arg(_pulseNum));
}

void EAQtTestCGMDEDialog::readDialog()
{
    _values->ChPot = (int32_t) _cbUsePotential->isChecked();
    _values->ElPot = _lePotential->text().toInt();
    _values->BT = _leBreakTime->text().toInt();
    _values->GP = _leValveTime->text().toInt();
}

void EAQtTestCGMDEDialog::TestStart()
{
    if ( EAQtData::getInstance().isMeasurement() ) {
        EAQtData::getInstance().NetworkError(tr("Measurement is active, cannot perform test"));
    }

    readDialog();

    if ( sendTestToEA() ) {
        _butTestStart->setDisabled(true);
        _butTestStop->setDisabled(false);
        _vResults.push_back(0);
        _testsNum++;
        return;
    } else {
        EAQtData::getInstance().NetworkError(tr("Could not send to EA"));
        return;
    }
}

void EAQtTestCGMDEDialog::TestStop()
{
    EAQtData::getInstance().sendTestCGMDEStop();
    _butTestStart->setDisabled(false);
    _butTestStop->setDisabled(true);
    calculateTest();
}

bool EAQtTestCGMDEDialog::sendTestToEA()
{
    return EAQtData::getInstance().sendTestCGMDE();
}

void EAQtTestCGMDEDialog::updateTest()
{
    _vResults[_testsNum-1] = EAQtData::getInstance().getCGMDETestNum();
    _pulseNum = EAQtData::getInstance().getCGMDETestNum();
    updateDialog();
}

void EAQtTestCGMDEDialog::calculateTest()
{
    if ( _testsNum > 1 ) {
        _averageResult = 0;
        for ( int i =0; i<_vResults.size();++i) {
        _averageResult +=_vResults[i];
        }
        _averageResult = _averageResult/_vResults.size();

        _stardardDev = 0;
        for ( int i = 0; i<_vResults.size(); ++i ) {
        _stardardDev += pow( (_vResults[i] - _averageResult), 2);
        }
        _stardardDev /= (_vResults.size()-1);
    } else {
        _stardardDev = 0;
        _averageResult = _pulseNum;
    }
    updateDialog();
}


