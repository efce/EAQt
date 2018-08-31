
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

  /*****************************************************************************************************************
  *  Electrocmical analyzer software EAQt to be used with 8KCA and M161
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
#ifndef EAQTPARAMDIALOG_H
#define EAQTPARAMDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include "eaqtdatainterface.h"
#include "eaqtparamdialogbreaksbutton.h"

class EAQtParamDialog : public QObject
{
    Q_OBJECT

public:
    EAQtParamDialog(EAQtDataInterface *pd, bool lsvDialog);
    void show();
    void setCGMDE(int, int);
    int getCGMDE(int num);
    void setBreaks(int n, int v);
    int getBreaks(int n);
    int32_t getParam(int32_t);
    enum validatorType {
        vt_repetitions,
        vt_breaks,
        vt_mixer,
        vt_potential_time,
        vt_tp,
        vt_potential,
        vt_points,
        vt_secs,
        vt_mins,
        vt_sqw_freq,
    };
    QIntValidator* getValidator(validatorType);

public slots:
    void showBreaks();
    void showCGMDE();
    void onlEditchanged(QString);
    void multicyclicChanged(bool);
    void microelectrodeChanged(bool);
    void multielectrodeChange(bool);
    void checkCGMDE();
    void saveAndQuit();
    void cancelAndQuit();
    void saveAndStart();
    void toggleAdvanced();
    void setSeriaFile();
    void methodChanged();
    void showPotentialProgram();
    void checkPotentialProgram();
    void calculateFreq();

private:
    QHBoxLayout* generateLayout();
    QGroupBox* createCurrentRangeGroup();
    QGroupBox* createRestItems();
    QGroupBox* createMesTypeGroup();
    QGroupBox* createElecType();
    QGroupBox* createProbingGroup();
    QGroupBox* createOkAndCancel();
    QGroupBox* createSetup();
    QGroupBox* createMeasurement();
    QGroupBox* createAdvanced();
    QGroupBox* grboxAdvancedSettings;
    void setParam(int32_t, int32_t);
    void sanitizeDialog();
    void prepareDialog();
    void saveParams();
    QDialog *_dialog;
    EAQtDataInterface *_pData;
    QFontMetrics* _metrics;
    bool _isLsv;
    bool _wasSaved;

    enum lineEditParamID {
        lid_Ep,
        lid_Ek,
        lid_Estep,
        lid_E0_dE,
        lid_sqw_freq,
        lid_tp,
        lid_tw,
        lid_td,
        lid_points,
        lid_aver,
        lid_mixer,
        lid_breaks,
        lid_NR_OF_ELEMENTS,
    };
    QVector<QLabel*> _lineLabels;
    QVector<QLineEdit*> _lineEdits;
    QGroupBox *_startEbox;
    QCheckBox *_checkboxIsMicro;
    EAQtParamDialogBreaksButton *_butManageBreaks;
    QPushButton* _butCGMDE_settings;
    QPushButton *_butProgramPotential;
    QPushButton *_butAdvancedSettings;
    QVector<QRadioButton*> _paramEl23;
    QVector<QRadioButton*> _paramSampl;
    QVector<QRadioButton*> _paramMessc;
    QCheckBox *_checkboxIsPro;
    QVector<QRadioButton*> _paramCrangeMacro;
    QVector<QRadioButton*> _paramCrangeMicro;
    QVector<QRadioButton*> _paramLSVdEdt;
    QVector<QRadioButton*> _paramElec;
    QVector<QRadioButton*> _paramMethod;
    enum PVMethods {
        method_scv,
        method_npv,
        method_dpv,
        method_osqw,
        method_csqw,
        method_SIZE
    };
    int _cgmdeTime;
    int _cgmdeDelay;
    int _cgmdeNr;
    int _cgmdeMode;
    int _dropsBeforeMes;
    int _breaksData[21];
    struct AdvancedWidgets {
        QCheckBox *nonaveraged;
        QGroupBox *isMultielectrode;
        QVector<QLineEdit*> channelNames {};
        QVector<QCheckBox*> useChannel {};
        QGroupBox *useMesFile;
        QLabel *mesFilePath;
    } _advWidgets;
};

#endif // EAQTPARAMDIALOG_H
