
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
#ifndef EAQTCALIBRATIONDIALOG_H
#define EAQTCALIBRATIONDIALOG_H

#include <QObject>
#include "eaqtdatainterface.h"

class EAQtCalibrationDialog : QObject
{
    Q_OBJECT

public:
    EAQtCalibrationDialog(EAQtDataInterface::CalibrationData *cd, QHash<QString,QString>* oldSettings);
    void exec();

private:
    struct multipliers {
        double multiply;
        QString name;
    };
    QVector<multipliers> _vVolumes;
    QVector<multipliers> _vConcs;
    QHash<QString,QString>* _settings;
    EAQtDataInterface::CalibrationData* _cd;

    QDialog *_dialog;
    QCheckBox *_calculateConc;
    QVector<QLineEdit*> _leConcentrations;
    QVector<QLineEdit*> _leAdditionVolumes;

    QGroupBox *_calculateBox;
    QLineEdit *_leSampleVolume;
    QLabel *_lSampleVolume;
    QLineEdit *_leStandardConc;
    QLabel *_lStandardConc;
    QComboBox *_cSampleVolumeUnits;
    QComboBox *_cAdditionVolumeUnits;
    QComboBox *_cStandardConcUnits;
    QComboBox *_cSampleConcUnits;
    QLabel *_labelAddition;
    QWidget *_scrollAreaWidget;

    QCustomPlot *_calibrationPlot;
    QVector<double> _signals;
    QCPItemStraightLine* _calibrationLine;
    QCPGraph* _calibrationPoints;
    QLineEdit *_calibrationEq;
    QLineEdit *_calibrationR;
    QLineEdit *_additionResult;
    QWidget *preparePlot();

    QFileDialog *_fd; // Save calibration as
    QCheckBox *_cbIncludeCurves; // Checkbox in save as dialog

private slots:
    void drawCalibration();
    void beforeReplot();
    void recalculateConc();
    void toggleCalculateConc(bool);
    void saveCalibration();
    void saveInFile(QString);
};

#endif // EAQTCALIBRATIONDIALOG_H
