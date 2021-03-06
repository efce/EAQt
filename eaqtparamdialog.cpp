
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
#include "eaqtparamdialog.h"
#include "eaqtparamcgmde.h"
#include "eaqtparambreaks.h"
#include "eaqtparampotentialprogram.h"
#include "eaqtdata.h"

EAQtParamDialog::EAQtParamDialog(EAQtDataInterface *pd, bool isLsv)
{
    this->_pData = pd;
    this->_isLsv = isLsv;

    this->_cgmdeTime = getParam(PARAM::valveTime);
    this->_cgmdeDelay = getParam(PARAM::valveDelay);
    this->_cgmdeNr = getParam(PARAM::valveCntr);
    this->_cgmdeMode = getParam(PARAM::cgmdeMode);
    this->_dropsBeforeMes = _pData->getDropsBeforeMes();

    for ( int i = 0; i<21; ++i ) {
        this->_breaksData[i] = this->getParam(PARAM::breakCntr+1+i);
    }

    this->_metrics = new QFontMetrics(QApplication::font());
    this->_dialog = new QDialog();
    this->_dialog->setModal(true);
    this->_dialog->setWindowTitle(tr("Parameters of ") + (isLsv?QString("LSV"):QString("PV")));
    this->_dialog->setLayout(this->generateLayout());
    this->_dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
    this->prepareDialog();
}

void EAQtParamDialog::show()
{
    this->_dialog->show();
}

int32_t EAQtParamDialog::getParam(int32_t num)
{
    if (this->_isLsv)
        return this->_pData->ParamLSV(num);
    else
        return this->_pData->ParamPV(num);
}

void EAQtParamDialog::setParam(int32_t num, int32_t val)
{
    if (this->_isLsv)
        this->_pData->ParamLSV(num,val);
    else
        this->_pData->ParamPV(num,val);
}

QHBoxLayout* EAQtParamDialog::generateLayout()
{
    QHBoxLayout *grid = new QHBoxLayout;

    const int dialogHeight = 30*this->_metrics->height();

    QVBoxLayout *vbl1 = new QVBoxLayout();
    QWidget *cnt1 = new QWidget();
    vbl1->addWidget(createMesTypeGroup());
    if ( !this->_isLsv ) {
        vbl1->addWidget(createProbingGroup());
    } else {
        createProbingGroup();
        // But dont show probing, as it cannot be changes for LSV //
    }
    cnt1->setLayout(vbl1);
    cnt1->setFixedHeight(dialogHeight);
    grid->addWidget(cnt1);

    this->_checkboxIsMicro = new QCheckBox(tr("Microelectrode"));
    _checkboxIsMicro->setCheckable(true);
    _checkboxIsMicro->setChecked(false);
    QGroupBox *elBox = new QGroupBox();
    QVBoxLayout *elecVB = new QVBoxLayout();
    elecVB->addWidget(this->_checkboxIsMicro);
    elecVB->addWidget(createElecType());
    elecVB->addWidget(createCurrentRangeGroup());
    elBox->setLayout(elecVB);
    elBox->setFixedHeight(30*this->_metrics->height());
    grid->addWidget(elBox);
    connect(this->_checkboxIsMicro,SIGNAL(toggled(bool)),this,SLOT(microelectrodeChanged(bool)));

    QVBoxLayout *vbl2 = new QVBoxLayout();
    QWidget *cnt2 = new QWidget();
    vbl2->addWidget(createMeasurement());
    vbl2->addWidget(createRestItems());
    cnt2->setLayout(vbl2);
    cnt2->setFixedHeight(dialogHeight);
    grid->addWidget(cnt2);

    QVBoxLayout *vbl3 = new QVBoxLayout();
    QWidget *cnt3 = new QWidget();
    vbl3->addWidget(createSetup());
    vbl3->addWidget(createOkAndCancel());
    cnt3->setLayout(vbl3);
    cnt3->setFixedHeight(dialogHeight);
    grid->addWidget(cnt3);

    grboxAdvancedSettings = createAdvanced();
    grid->addWidget(grboxAdvancedSettings);
    return grid;
}

QGroupBox* EAQtParamDialog::createOkAndCancel()
{
    QGroupBox* gb = new QGroupBox();
    _butAdvancedSettings = new QPushButton(tr("Toggle advanced >>"));
    _butAdvancedSettings->setFixedHeight(this->_metrics->height()*2);
    connect(_butAdvancedSettings,SIGNAL(clicked(bool)),this,SLOT(toggleAdvanced()));
    QPushButton* butOK = new QPushButton(tr("Save changes"));
    butOK->setFixedHeight(this->_metrics->height()*2);
    connect(butOK,SIGNAL(clicked(bool)),this,SLOT(saveAndQuit()));
    QPushButton* butCancel = new QPushButton(tr("Cancel changes"));
    connect(butCancel,SIGNAL(clicked(bool)),this,SLOT(cancelAndQuit()));
    butCancel->setFixedHeight(this->_metrics->height()*2);
    QPushButton* butStart = new QPushButton(tr("Start measurement"));
    connect(butStart,SIGNAL(clicked(bool)),this,SLOT(saveAndStart()));
    butStart->setFixedHeight(this->_metrics->height()*2);
    QVBoxLayout* bl = new QVBoxLayout();
    bl->addWidget(_butAdvancedSettings);
    bl->addWidget(butStart);
    bl->addWidget(butOK);
    bl->addWidget(butCancel);
    bl->setMargin(5);
    gb->setLayout(bl);
    gb->setFixedHeight(20*this->_metrics->height());
    return gb;
}

QGroupBox* EAQtParamDialog::createCurrentRangeGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Current range"));

    this->_paramCrangeMacro.resize(7);
    this->_paramCrangeMicro.resize(4);

    _paramCrangeMicro[PARAM::crange_micro_5uA] = new QRadioButton(tr("5 µA"));
    _paramCrangeMicro[PARAM::crange_micro_500nA] = new QRadioButton(tr("500 nA"));
    _paramCrangeMicro[PARAM::crange_micro_50nA] = new QRadioButton(tr("50 nA"));
    _paramCrangeMicro[PARAM::crange_micro_5nA] = new QRadioButton(tr("5 nA"));

    _paramCrangeMacro[PARAM::crange_macro_100mA] = new QRadioButton(tr("100 mA"));
    _paramCrangeMacro[PARAM::crange_macro_10mA] = new QRadioButton(tr("10 mA"));
    _paramCrangeMacro[PARAM::crange_macro_1mA] = new QRadioButton(tr("1 mA"));
    _paramCrangeMacro[PARAM::crange_macro_100uA] = new QRadioButton(tr("100 µA"));
    _paramCrangeMacro[PARAM::crange_macro_10uA] = new QRadioButton(tr("10 µA"));
    _paramCrangeMacro[PARAM::crange_macro_1uA] = new QRadioButton(tr("1 µA"));
    _paramCrangeMacro[PARAM::crange_macro_100nA] = new QRadioButton(tr("100 nA"));

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setStretch(0,0);
    vbox->setSpacing(0);
    vbox->addWidget(_paramCrangeMicro[PARAM::crange_micro_5uA]);
    vbox->addWidget(_paramCrangeMicro[PARAM::crange_micro_500nA]);
    vbox->addWidget(_paramCrangeMicro[PARAM::crange_micro_50nA]);
    vbox->addWidget(_paramCrangeMicro[PARAM::crange_micro_5nA]);

    vbox->addWidget(_paramCrangeMacro[PARAM::crange_macro_100mA]);
    vbox->addWidget(_paramCrangeMacro[PARAM::crange_macro_10mA]);
    vbox->addWidget(_paramCrangeMacro[PARAM::crange_macro_1mA]);
    vbox->addWidget(_paramCrangeMacro[PARAM::crange_macro_100uA]);
    vbox->addWidget(_paramCrangeMacro[PARAM::crange_macro_10uA]);
    vbox->addWidget(_paramCrangeMacro[PARAM::crange_macro_1uA]);
    vbox->addWidget(_paramCrangeMacro[PARAM::crange_macro_100nA]);

    vbox->setSpacing(1);
    vbox->addStretch(0);

    groupBox->setLayout(vbox);
    //groupBox->setFixedHeight(17*metrics->height());

    return groupBox;
}

QGroupBox* EAQtParamDialog::createSetup()
{
    this->_paramEl23.resize(3);
    QGroupBox *el23Selection = new QGroupBox(tr("Setup"));
    QVBoxLayout *vbel23 = new QVBoxLayout();
    _paramEl23[PARAM::el23_two] = new QRadioButton(tr("two electrodes"));
    _paramEl23[PARAM::el23_three] = new QRadioButton(tr("three electrodes"));
    _paramEl23[PARAM::el23_dummy] = new QRadioButton(tr("internal"));
    vbel23->addWidget(_paramEl23[PARAM::el23_two]);
    vbel23->addWidget(_paramEl23[PARAM::el23_three]);
    vbel23->addWidget(_paramEl23[PARAM::el23_dummy]);
    vbel23->setSpacing(1);
    vbel23->addStretch(0);
    el23Selection->setLayout(vbel23);
    return el23Selection;
}

QGroupBox* EAQtParamDialog::createMeasurement()
{
    this->_paramMessc.resize(3);
    QGroupBox *messcSelection = new QGroupBox(tr("Measurement"));
    QVBoxLayout *vbmessc = new QVBoxLayout();
    _paramMessc[PARAM::messc_single] = new QRadioButton(tr("single","MESSC"));
    _paramMessc[PARAM::messc_cyclic] = new QRadioButton(tr("cyclic","MESSC"));
    _paramMessc[PARAM::messc_multicyclic] = new QRadioButton(tr("multicyclic","MESSC"));
    connect(_paramMessc[PARAM::messc_multicyclic],SIGNAL(toggled(bool)),this,SLOT(multicyclicChanged(bool)));
    if ( !this->_isLsv ) {
        _paramMessc[PARAM::messc_multicyclic]->setVisible(false);
    }
    vbmessc->addWidget(_paramMessc[PARAM::messc_single]);
    vbmessc->addWidget(_paramMessc[PARAM::messc_cyclic]);
    vbmessc->addWidget(_paramMessc[PARAM::messc_multicyclic]);
    vbmessc->setSpacing(1);
    vbmessc->addStretch(0);
    messcSelection->setLayout(vbmessc);
    return messcSelection;
}

QGroupBox *EAQtParamDialog::createRestItems()
{
    this->_butManageBreaks = new EAQtParamDialogBreaksButton;
    _butManageBreaks->setText(tr("Setup breaks"));
    _butManageBreaks->setFixedHeight(2*this->_metrics->height());

    QGroupBox *groupBox = new QGroupBox;

    QGridLayout *qgrid = new QGridLayout;

    this->_lineEdits.resize(lid_NR_OF_ELEMENTS);
    this->_lineLabels.resize(lid_NR_OF_ELEMENTS);
    int pos = 0;
    int i = 0;

    _lineEdits[lid_Ep] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::Ep)));
    _lineEdits[lid_Ep]->setAccessibleName("Ep");
    connect(_lineEdits[lid_Ep],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_Ep]->setValidator(this->getValidator(EAQtParamDialog::vt_potential));
    _lineEdits[lid_Ep]->setMaxLength(5);
    _lineEdits[lid_Ep]->setFixedWidth(_metrics->width("9999999"));
    _lineLabels[lid_Ep] = new QLabel(tr("Ep [mV]:"));//9
    i=lid_Ep;
    qgrid->addWidget(_lineLabels[i], pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_Ek] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::Ek)));
    _lineEdits[lid_Ek]->setAccessibleName("Ek");
    connect(_lineEdits[lid_Ek],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_Ek]->setValidator(this->getValidator(EAQtParamDialog::vt_potential));
    _lineEdits[lid_Ek]->setMaxLength(5);
    _lineEdits[lid_Ek]->setFixedWidth(_metrics->width("9999999"));
    _lineLabels[lid_Ek] = new QLabel(tr("Ek [mV]:"));//10
    i=lid_Ek;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_Estep] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::Estep)));
    _lineEdits[lid_Estep]->setAccessibleName("Estep");
    //connect(lineEdits[lid_Estep],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_Estep]->setValidator(this->getValidator(EAQtParamDialog::vt_potential));
    _lineEdits[lid_Estep]->setMaxLength(5);
    _lineEdits[lid_Estep]->setFixedWidth(_metrics->width("9999999"));
    _lineLabels[lid_Estep] = new QLabel(tr("Estep [mV]:"));//11
    i=lid_Estep;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_E0_dE] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::E0)));
    _lineEdits[lid_E0_dE]->setAccessibleName("dE");
    //connect(lineEdits[lid_E0_dE],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_E0_dE]->setValidator(this->getValidator(EAQtParamDialog::vt_potential));
    _lineEdits[lid_E0_dE]->setMaxLength(5);
    _lineEdits[lid_E0_dE]->setFixedWidth(_metrics->width("999999"));
    _lineLabels[lid_E0_dE] = new QLabel(tr("dE [mV]:"));//12
    if ( !this->_isLsv ) {
        i=lid_E0_dE;
        qgrid->addWidget(_lineLabels[i], ++pos, 0);
        qgrid->addWidget(_lineEdits[i], pos, 1);
    }

    _lineEdits[lid_sqw_freq] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::sqw_frequency)));
    _lineEdits[lid_sqw_freq]->setValidator(this->getValidator(EAQtParamDialog::vt_sqw_freq));
    _lineEdits[lid_sqw_freq]->setAccessibleName("sqw_freq");
    _lineEdits[lid_sqw_freq]->setVisible(false);
    _lineEdits[lid_sqw_freq]->setFixedWidth(_metrics->width("9999999"));
    _lineEdits[lid_sqw_freq]->setMaxLength(5);
    connect(_lineEdits[lid_sqw_freq], SIGNAL(editingFinished()), this, SLOT(calculateFreq()));
    _lineLabels[lid_sqw_freq] = new QLabel(tr("f [Hz]:"));
    _lineLabels[lid_sqw_freq]->setVisible(false);
    i=lid_sqw_freq;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_tp] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::tp)));
    _lineEdits[lid_tp]->setAccessibleName("tp");
    //connect(lineEdits[lid_tp],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_tp]->setValidator(this->getValidator(EAQtParamDialog::vt_tp));
    _lineEdits[lid_tp]->setMaxLength(3);
    _lineEdits[lid_tp]->setFixedWidth(_metrics->width("99999"));
    _lineLabels[lid_tp] = new QLabel(tr("tp [ms]:"));//13
    i=lid_tp;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_tw] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::tw)));
    _lineEdits[lid_tw]->setAccessibleName(tr("nr%1").arg(PARAM::tw));
    //connect(lineEdits[lid_tw],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_tw]->setValidator(this->getValidator(EAQtParamDialog::vt_potential_time));
    _lineEdits[lid_tw]->setMaxLength(3);
    _lineEdits[lid_tw]->setFixedWidth(_metrics->width("99999"));
    _lineLabels[lid_tw] = new QLabel(tr("tw [ms]:"));//14
    i=lid_tw;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_td] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::td)));
    _lineEdits[lid_td]->setAccessibleName(tr("nr%1").arg(PARAM::td));
    //connect(lineEdits[lid_td],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_td]->setValidator(this->getValidator(EAQtParamDialog::vt_repetitions));
    _lineEdits[lid_td]->setMaxLength(4);
    _lineEdits[lid_td]->setFixedWidth(_metrics->width("999999"));
    _lineLabels[lid_td] = new QLabel(tr("td [ms]:"));//15
    if ( !this->_isLsv ) {
        i=lid_td;
        qgrid->addWidget(_lineLabels[i], ++pos, 0);
        qgrid->addWidget(_lineEdits[i], pos, 1);
    }

    _lineEdits[lid_aver] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::aver)));
    _lineEdits[lid_aver]->setAccessibleName(tr("nr%1").arg(PARAM::aver));
    //connect(lineEdits[lid_points],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_aver]->setValidator(this->getValidator(EAQtParamDialog::vt_mixer));
    _lineEdits[lid_aver]->setMaxLength(3);
    _lineEdits[lid_aver]->setFixedWidth(_metrics->width("99999"));
    _lineLabels[lid_aver] = new QLabel(tr("average:"));
    i=lid_aver;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_mixer] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::mesStirrerDelay)));
    _lineEdits[lid_mixer]->setAccessibleName(tr("nr%1").arg(PARAM::mesStirrerDelay));
    //connect(lineEdits[lid_points],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_mixer]->setValidator(this->getValidator(EAQtParamDialog::vt_mixer));
    _lineEdits[lid_mixer]->setMaxLength(3);
    _lineEdits[lid_mixer]->setFixedWidth(_metrics->width("99999"));
    _lineLabels[lid_mixer] = new QLabel(tr("mixer [s]:"));
    i=lid_mixer;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_points] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::ptnr))); // ZLE bo to jeszcze moze byc ts
    _lineEdits[lid_points]->setAccessibleName(tr("nr%1").arg(PARAM::ptnr));
    //connect(lineEdits[lid_points],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_points]->setValidator(this->getValidator(EAQtParamDialog::vt_points));
    _lineEdits[lid_points]->setMaxLength(6);
    _lineEdits[lid_points]->setFixedWidth(_metrics->width("9999999"));
    _lineLabels[lid_points] = new QLabel(tr("points:")); //16
    i=lid_points;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    _lineEdits[lid_breaks] = new QLineEdit(tr("%1").arg(this->getParam(PARAM::breakCntr))); // ZLE bo to jeszcze moze byc ts
    _lineEdits[lid_breaks]->setAccessibleName(tr("nr%1").arg(PARAM::breakCntr));
    //connect(lineEdits[lid_breaks],SIGNAL(textEdited(QString)),this,SLOT(onlEditchanged(QString)));
    _lineEdits[lid_breaks]->setValidator(this->getValidator(EAQtParamDialog::vt_breaks));
    _lineEdits[lid_breaks]->setMaxLength(1);
    connect(_butManageBreaks,SIGNAL(clicked()),this,SLOT(showBreaks()));
    connect(_lineEdits[lid_breaks],SIGNAL(textEdited(QString)),_butManageBreaks,SLOT(breaksChanged(QString)));
    _lineEdits[lid_breaks]->setFixedWidth(_metrics->width("999"));
    _lineLabels[lid_breaks] = new QLabel(tr("breaks:"));
    i=lid_breaks;
    qgrid->addWidget(_lineLabels[i], ++pos, 0);
    qgrid->addWidget(_lineEdits[i], pos, 1);

    if ( this->_isLsv ) {
        qgrid->addWidget(_butManageBreaks,++pos,0,1,2);

        _lineEdits[lid_Estep]->setVisible(false);
        _lineLabels[lid_Estep]->setVisible(false);
        _lineEdits[lid_tp]->setVisible(false);
        _lineLabels[lid_tp]->setVisible(false);
        _lineEdits[lid_tw]->setVisible(false);
        _lineLabels[lid_tw]->setVisible(false);
        _lineEdits[lid_points]->setVisible(false);
        _lineLabels[lid_points]->setVisible(false);

        this->_startEbox = new QGroupBox(tr("Use start potential"));
        _startEbox->setCheckable(true);
        _startEbox->setChecked(false);
        _lineLabels[lid_td]->setText(tr("tstart [s]:"));//15
        _lineLabels[lid_E0_dE]->setText(tr("Estart [mV]:"));//11
        QGridLayout *strLay = new QGridLayout();
        strLay->addWidget(_lineLabels[lid_td],0,0);
        strLay->addWidget(_lineLabels[lid_E0_dE],1,0);
        strLay->addWidget(_lineEdits[lid_td],0,1);
        strLay->addWidget(_lineEdits[lid_E0_dE],1,1);
        _startEbox->setLayout(strLay);
        _startEbox->setFlat(true);
        qgrid->addWidget(_startEbox,++pos,0,1,2);
    } else {
        qgrid->addWidget(_butManageBreaks,++pos,0,1,2);
    }
    qgrid->setSpacing(1);
    //qgrid->addStretch(0);
    groupBox->setLayout(qgrid);
    return groupBox;
}

QGroupBox *EAQtParamDialog::createMesTypeGroup()
{
    QGroupBox *groupBox = new QGroupBox();
    if ( !this->_isLsv ) {
        this->_paramMethod.resize(method_SIZE);
        groupBox->setTitle(tr("Voltammetry"));
        _paramMethod[method_scv] = new QRadioButton(tr("staircase (SCV)"));
        connect(_paramMethod[method_scv],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
        _paramMethod[method_npv] = new QRadioButton(tr("normal pulse (NPV)"));
        connect(_paramMethod[method_npv],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
        _paramMethod[method_dpv] = new QRadioButton(tr("differential pulse (DPV)"));
        connect(_paramMethod[method_dpv],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
        _paramMethod[method_osqw] = new QRadioButton(tr("Osteryoung sqaurewave (OSQW)"));
        connect(_paramMethod[method_osqw],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));
        _paramMethod[method_csqw] = new QRadioButton(tr("[NEW] sqaurewave (SQW)"));
        connect(_paramMethod[method_csqw],SIGNAL(toggled(bool)),this,SLOT(methodChanged()));

        _butProgramPotential = new QPushButton;
        _butProgramPotential->setText(tr("Program E"));
        _butProgramPotential->setFixedHeight(30);
        _checkboxIsPro = new QCheckBox(tr("Potential change program"));
        connect(_butProgramPotential,SIGNAL(clicked()),this,SLOT(showPotentialProgram()));
        connect(_checkboxIsPro,SIGNAL(clicked(bool)),_butProgramPotential,SLOT(setEnabled(bool)));
        connect(_checkboxIsPro,SIGNAL(toggled(bool)),this,SLOT(checkPotentialProgram()));

        QVBoxLayout *vbox = new QVBoxLayout;
        vbox->addWidget(_checkboxIsPro);
        vbox->addWidget(_butProgramPotential);
        for ( int i = 0; i<_paramMethod.size(); ++i ) {
            vbox->addWidget(_paramMethod[i]);
        }
        vbox->addStretch(0);
        vbox->setSpacing(1);
        groupBox->setLayout(vbox);

    } else {
        groupBox->setTitle(tr("LSV sweep rate"));
        QVBoxLayout* speedVBL = new QVBoxLayout();

        this->_paramLSVdEdt.resize(MEASUREMENT::LSVnrOfDEdt);
        for ( int i = 0; i<_paramLSVdEdt.size(); ++i ) {
            double speed = MEASUREMENT::LSVstepE[i]/MEASUREMENT::LSVtime[i];
            if ( speed < 0.025 )
                this->_paramLSVdEdt[i] = new QRadioButton(tr("%1 mV/s").arg(speed*1000,0,'f',1));
            else if ( speed < 1 )
                this->_paramLSVdEdt[i] = new QRadioButton(tr("%1 mV/s").arg(speed*1000,0,'f',0));
            else
                this->_paramLSVdEdt[i] = new QRadioButton(tr("%1 V/s").arg(speed,0,'f',0));

            speedVBL->addWidget(this->_paramLSVdEdt[i]);
        }
        speedVBL->addStretch(1);
        speedVBL->setSpacing(1);
        groupBox->setLayout(speedVBL);
    }
    return groupBox;
}

QGroupBox* EAQtParamDialog::createElecType()
{
    QGroupBox* gb = new QGroupBox(tr("Electrode"));
    QVBoxLayout* lay = new QVBoxLayout();

    this->_paramElec.resize(6);
    this->_paramElec[PARAM::electr_dme] = new QRadioButton(tr("DME"));
    connect(this->_paramElec[PARAM::electr_dme],SIGNAL(toggled(bool)),this,SLOT(checkElectr()));
    this->_paramElec[PARAM::electr_cgmde] = new QRadioButton(tr("CGMDE"));
    connect(this->_paramElec[PARAM::electr_cgmde],SIGNAL(toggled(bool)),this,SLOT(checkElectr()));
    this->_paramElec[PARAM::electr_solid] = new QRadioButton(tr("Solid"));
    this->_paramElec[PARAM::electr_microDme] = new QRadioButton(tr("µDME"));
    connect(this->_paramElec[PARAM::electr_microDme],SIGNAL(toggled(bool)),this,SLOT(checkElectr()));
    this->_paramElec[PARAM::electr_microCgmde] = new QRadioButton(tr("µCGMDE"));
    connect(this->_paramElec[PARAM::electr_microCgmde],SIGNAL(toggled(bool)),this,SLOT(checkElectr()));
    this->_paramElec[PARAM::electr_microSolid] = new QRadioButton(tr("µSolid"));
    for (int i =0; i<this->_paramElec.size(); ++i) {
        this->_paramElec[i]->setVisible(false);
        lay->addWidget(this->_paramElec[i]);
    }
    this->_butCGMDE_settings = new QPushButton;
    _butCGMDE_settings->setText(tr("CGMDE settings"));
    _butCGMDE_settings->setFixedHeight(30);
    connect(_butCGMDE_settings,SIGNAL(clicked()),this,SLOT(showCGMDE()));
    lay->addWidget(_butCGMDE_settings);
    lay->addStretch(0);
    lay->setSpacing(1);
    gb->setLayout(lay);
    gb->setFixedHeight(10*_metrics->height());
    return gb;
}

QGroupBox* EAQtParamDialog::createProbingGroup()
{
    this->_paramSampl.resize(2);
    QGroupBox *samplSelection = new QGroupBox(tr("Probing"));
    QVBoxLayout *vbsampl = new QVBoxLayout();
    _paramSampl[PARAM::sampl_single] = new QRadioButton(tr("single","SAMPL"));
    _paramSampl[PARAM::sampl_double] = new QRadioButton(tr("double","SAMPL"));
    vbsampl->addWidget(_paramSampl[0]);
    vbsampl->addWidget(_paramSampl[1]);
    vbsampl->addStretch(0);
    vbsampl->setSpacing(1);
    samplSelection->setLayout(vbsampl);
    samplSelection->setFixedHeight(6*_metrics->height());

    return samplSelection;
}

QGroupBox* EAQtParamDialog::createAdvanced()
{
    QGroupBox* gb = new QGroupBox();
    QGridLayout *gl = new QGridLayout();
    QVBoxLayout *meselay = new QVBoxLayout();
    _advWidgets.nonaveraged = new QCheckBox(tr("Do not record nonaveraged\n current samples"));
    _advWidgets.nonaveraged->setChecked( (getParam(PARAM::nonaveragedsampling)==0) );
    _advWidgets.useMesFile = new QGroupBox(tr("Measurement series"));
    _advWidgets.useMesFile->setCheckable(true);
    _advWidgets.isMultielectrode = new QGroupBox(tr("Multielectrode setup"));
    _advWidgets.isMultielectrode->setCheckable(true);
    _advWidgets.isMultielectrode->setChecked( (this->getParam(PARAM::multi)!=0) );
    this->connect(_advWidgets.isMultielectrode, SIGNAL(toggled(bool)), this, SLOT(multielectrodeChange(bool)));
    std::vector<bool> enabled = _pData->getChannelsEnabled();
    QVector<QString> names = _pData->getChannelsNames();
    _advWidgets.useChannel.resize(enabled.size());
    _advWidgets.channelNames.resize(enabled.size());
    QGridLayout *multiLay = new QGridLayout();
    std::vector<QColor> colors = {
        COLOR::measurement,
        COLOR::channel1,
        COLOR::channel2,
        COLOR::channel3,
        COLOR::channel4,
        COLOR::channel5,
        COLOR::channel6,
        COLOR::channel7,
    };
    for ( uint i = 0; i<enabled.size(); ++i ) {
        _advWidgets.useChannel[i] = new QCheckBox();
        _advWidgets.channelNames[i] = new QLineEdit();
        _advWidgets.channelNames[i]->setText(names[i]);
        _advWidgets.channelNames[i]->setMaxLength(10);
        QPalette palette;
        palette.setColor(QPalette::Base, colors[i]);
        palette.setColor(QPalette::Text, QColor(255,255,255));
        _advWidgets.channelNames[i]->setPalette(palette);
        multiLay->addWidget(_advWidgets.useChannel[i],i,0,1,1);
        multiLay->addWidget(_advWidgets.channelNames[i],i,1,1,1);
        connect(_advWidgets.useChannel[i]
                ,SIGNAL(toggled(bool))
                ,_advWidgets.channelNames[i]
                ,SLOT(setEnabled(bool)));
        _advWidgets.useChannel[i]->setChecked(enabled[i]);
        _advWidgets.channelNames[i]->setEnabled(enabled[i]);
    }
    _advWidgets.isMultielectrode->setLayout(multiLay);

    _advWidgets.mesFilePath = new QLabel();
    _advWidgets.mesFilePath->setWordWrap(true);
    _advWidgets.mesFilePath->setFixedWidth(200);
    _advWidgets.mesFilePath->setText(_pData->getMesSeriesFile());

    QLabel *mesFilePathLabel = new QLabel(tr("File path:"));

    QPushButton *setFile = new QPushButton(tr("Select file"));
    connect(setFile,SIGNAL(clicked(bool)),this,SLOT(setSeriaFile()));

    meselay->addWidget(mesFilePathLabel);
    meselay->addWidget(_advWidgets.mesFilePath);
    meselay->addWidget(setFile);
    meselay->addSpacing(0);
    meselay->addStretch(0);
    _advWidgets.useMesFile->setLayout(meselay);
    _advWidgets.useMesFile->setChecked(_pData->getUseMesFile());
    gl->addWidget(_advWidgets.nonaveraged,0,0,1,1);
    gl->addWidget(_advWidgets.useMesFile,1,0,2,1);
    gl->addWidget(_advWidgets.isMultielectrode,0,1,3,1);
    gb->setLayout(gl);
    return gb;
}

void EAQtParamDialog::toggleAdvanced()
{
    this->grboxAdvancedSettings->setVisible(!this->grboxAdvancedSettings->isVisible());
}

void EAQtParamDialog::setSeriaFile()
{
    QFileDialog* qfd = new QFileDialog();
    //qfd->setNameFilter("EACfg (*.eacfg)");
    QString fp = qfd->getOpenFileName(NULL,tr("Load measurement series"),"",QString("EACFG (*.eacfg)"));
    delete qfd;
    _advWidgets.mesFilePath->setText(fp);
}

void EAQtParamDialog::showBreaks()
{
    int nNrOfBreaksSet;
    nNrOfBreaksSet = -1;
    nNrOfBreaksSet=this->_lineEdits[lid_breaks]->text().toInt();
    if ( nNrOfBreaksSet <= 0)
        return;
    new EAQtParamBreaks(this, nNrOfBreaksSet);

    return;
}

void  EAQtParamDialog::showCGMDE() {
    EAQtParamCGMDE *dCGMD = new EAQtParamCGMDE(this);
    dCGMD->showCGMDE();
    return;
}

QIntValidator* EAQtParamDialog::getValidator(validatorType vt)
{
    QIntValidator *vali;
    switch (vt) {
    case vt_repetitions:
        vali = new QIntValidator(1,9999);
        return vali;
    case vt_breaks:
        vali = new QIntValidator(0,7);
        return vali;
    case vt_mixer:
        vali = new QIntValidator(0,999);
        return vali;
    case vt_potential_time:
        vali = new QIntValidator(0,999);
        return vali;
    case vt_tp:
        vali = new QIntValidator(1,999);
        return vali;
    case vt_potential:
        vali = new QIntValidator(-9999,9999);
        return vali;
     case vt_points:
        vali = new QIntValidator(1,999999);
        return vali;
    case vt_secs:
       vali = new QIntValidator(0,60);
       return vali;
    case vt_mins:
       vali = new QIntValidator(0,999);
       return vali;
    case vt_sqw_freq:
        vali = new QIntValidator(1,500);
        return vali;
     default:
        throw 0;
    }
}

void EAQtParamDialog::onlEditchanged(QString text)
{
    int EpVal = this->_lineEdits[lid_Ep]->text().toInt();
    int EkVal = this->_lineEdits[lid_Ek]->text().toInt();
    if ( EpVal == EkVal ) {
        if ( this->_isLsv ) {
            QMessageBox mb;
            mb.setText(tr("Ep cannot be equal to Ek in LSV mode."));
        } else {
            this->_lineEdits[lid_points]->setDisabled(false);
            this->_lineEdits[lid_Estep]->setDisabled(true);
            this->_lineLabels[lid_Estep]->setDisabled(true);
        }
    } else {
        this->_lineEdits[lid_points]->setDisabled(true);
        this->_lineEdits[lid_Estep]->setDisabled(false);
        this->_lineLabels[lid_Estep]->setDisabled(false);
    }
}

void EAQtParamDialog::multicyclicChanged(bool state)
{
    if ( state == true ) {
        this->_startEbox->setDisabled(false);
        this->_lineLabels[lid_aver]->setText(tr("cycles:"));
    } else {
        this->_startEbox->setChecked(false);
        this->_startEbox->setDisabled(true);
        this->_lineLabels[lid_aver]->setText(tr("average:"));
    }
}

void EAQtParamDialog::microelectrodeChanged(bool state)
{
    int checked = -1;
    if ( state == true ) {
        for ( int i = 0; i<this->_paramCrangeMacro.size(); ++i ) {
            if ( this->_paramCrangeMacro[i]->isChecked() ) {
                checked = i;
            }
            this->_paramCrangeMacro[i]->setChecked(false);
            this->_paramCrangeMacro[i]->setVisible(false);
        }
        for ( int i = 0;i<this->_paramCrangeMicro.size(); ++i ) {
            this->_paramCrangeMicro[i]->setVisible(true);
        }
        if ( checked < 0 ) {
            ;
        } else if ( checked >= this->_paramCrangeMicro.size() ) {
            this->_paramCrangeMicro[this->_paramCrangeMicro.size()-1]->setChecked(true);
        } else {
            this->_paramCrangeMicro[checked]->setChecked(true);
        }
        if ( this->_paramElec[PARAM::electr_dme]->isChecked() ) {
            this->_paramElec[PARAM::electr_dme]->setChecked(false);
            this->_paramElec[PARAM::electr_microSolid]->setChecked(true);
        } else if ( this->_paramElec[PARAM::electr_cgmde]->isChecked() ) {
            this->_paramElec[PARAM::electr_cgmde]->setChecked(false);
            this->_paramElec[PARAM::electr_microCgmde]->setChecked(true);
        } else if ( this->_paramElec[PARAM::electr_solid]->isChecked() ) {
            this->_paramElec[PARAM::electr_solid]->setChecked(false);
            this->_paramElec[PARAM::electr_microSolid]->setChecked(true);
        }
        this->_paramElec[PARAM::electr_dme]->setVisible(false);
        this->_paramElec[PARAM::electr_cgmde]->setVisible(false);
        this->_paramElec[PARAM::electr_solid]->setVisible(false);
        this->_paramElec[PARAM::electr_microDme]->setVisible(false);
        this->_paramElec[PARAM::electr_microCgmde]->setVisible(true);
        this->_paramElec[PARAM::electr_microSolid]->setVisible(true);
    } else {
        for ( int i = 0; i<this->_paramCrangeMacro.size(); ++i ) {
            this->_paramCrangeMacro[i]->setVisible(true);
        }
        for ( int i = 0;i<this->_paramCrangeMicro.size(); ++i ) {
            if ( this->_paramCrangeMicro[i]->isChecked() ) {
                checked = i;
            }
            this->_paramCrangeMicro[i]->setChecked(false);
            this->_paramCrangeMicro[i]->setVisible(false);
        }
        if (checked >=0) {
            this->_paramCrangeMacro[checked]->setChecked(true);
        }

        if ( this->_paramElec[PARAM::electr_microCgmde]->isChecked() ) {
            this->_paramElec[PARAM::electr_microCgmde]->setChecked(false);
            this->_paramElec[PARAM::electr_cgmde]->setChecked(true);
        } else if ( this->_paramElec[PARAM::electr_microSolid]->isChecked() ) {
            this->_paramElec[PARAM::electr_microSolid]->setChecked(false);
            this->_paramElec[PARAM::electr_solid]->setChecked(true);
        }
        this->_paramElec[PARAM::electr_dme]->setVisible(true);
        this->_paramElec[PARAM::electr_cgmde]->setVisible(true);
        this->_paramElec[PARAM::electr_solid]->setVisible(true);
        this->_paramElec[PARAM::electr_microDme]->setVisible(false);
        this->_paramElec[PARAM::electr_microCgmde]->setVisible(false);
        this->_paramElec[PARAM::electr_microSolid]->setVisible(false);
    }
}

void EAQtParamDialog::checkElectr()
{
    if ( this->_paramElec[PARAM::electr_microCgmde]->isChecked()
    || this->_paramElec[PARAM::electr_cgmde]->isChecked() ) {
        this->_butCGMDE_settings->setEnabled(true);
    } else {
        this->_butCGMDE_settings->setEnabled(false);
        if ( this->_paramElec[PARAM::electr_dme]->isChecked()
        || this->_paramElec[PARAM::electr_microDme]->isChecked() ) {
            this->_lineLabels[lid_td]->setText(tr("tk [ms]:"));
        } else {
            this->_lineLabels[lid_td]->setText(tr("td [ms]:"));
        }
    }
}

void EAQtParamDialog::prepareDialog()
{
    if ( this->_isLsv ) {
        this->grboxAdvancedSettings->setVisible(false);
        this->grboxAdvancedSettings->setEnabled(false);
        this->_butAdvancedSettings->setEnabled(false);
    } else {
        if ( !_pData->getUseMesFile()
        && getParam(PARAM::multi) == 0
        && getParam(PARAM::nonaveragedsampling) != 0 ) {
            this->grboxAdvancedSettings->setVisible(false);
        }
    }

    if ( this->getParam(PARAM::breakCntr) == 0 ) {
        this->_butManageBreaks->setDisabled(true);
    } else {
        this->_butManageBreaks->setEnabled(true);
    }
    if (this->getParam(PARAM::electr) >= PARAM::electr_multi) {
        this->_paramElec[PARAM::electr_microSolid]->setChecked(true);
    } else {
        this->_paramElec[this->getParam(PARAM::electr)]->setChecked(true);
    }
    if (this->getParam(PARAM::electr) >= PARAM::electr_multi) {
        this->_paramCrangeMicro[this->getParam(PARAM::crange)]->setChecked(true);
        _checkboxIsMicro->setChecked(true);
        this->multielectrodeChange(true);
    } else if ( this->getParam(PARAM::electr) >= PARAM::electr_micro ) {
        this->microelectrodeChanged(true);
        this->_paramCrangeMicro[this->getParam(PARAM::crange)]->setChecked(true);
        _checkboxIsMicro->setChecked(true);
    } else {
        this->microelectrodeChanged(false);
        this->_paramCrangeMacro[this->getParam(PARAM::crange)]->setChecked(true);
        _checkboxIsMicro->setChecked(false);
    }
//    if ( this->getParam(PARAM::electr) == PARAM::electr_dme ) {
//        this->checkboxIsMicro->setChecked(false);
//    }
    if ( this->getParam(PARAM::electr) == PARAM::electr_cgmde
    || this->getParam(PARAM::electr) == PARAM::electr_microCgmde ) {
        this->_butCGMDE_settings->setEnabled(true);
    } else {
        this->_butCGMDE_settings->setEnabled(false);
    }
    this->_paramEl23[this->getParam(PARAM::el23)]->setChecked(true);
    this->_paramSampl[this->getParam(PARAM::sampl)]->setChecked(true);
    this->_paramMessc[this->getParam(PARAM::messc)]->setChecked(true);
    if ( this->_isLsv ) {
        if ( this->getParam(PARAM::messc) != PARAM::messc_multicyclic ) {
            this->_startEbox->setDisabled(true);
        } else if ( this->getParam(PARAM::Escheck) == PARAM::Escheck_yes ) {
            this->_startEbox->setChecked(true);
            this->_lineLabels[lid_aver]->setText(tr("cycles:"));
        } else {
            this->_lineLabels[lid_aver]->setText(tr("cycles:"));
        }
        this->_paramLSVdEdt[this->getParam(PARAM::dEdt)]->setChecked(true);
    } else {
        switch (this->getParam(PARAM::method)) {
        case PARAM::method_scv:
            this->_paramMethod[method_scv]->setChecked(true);
            break;
        case PARAM::method_npv:
            this->_paramMethod[method_npv]->setChecked(true);
            break;
        case PARAM::method_dpv:
            this->_paramMethod[method_dpv]->setChecked(true);
            break;
        case PARAM::method_sqw_osteryoung:
            this->_paramMethod[method_osqw]->setChecked(true);
            break;
        case PARAM::method_sqw_classic:
            this->_paramMethod[method_csqw]->setChecked(true);
            break;
        }

        _lineEdits[lid_sqw_freq]->setText(tr("%1").arg(this->getParam(PARAM::sqw_frequency)));

        if ( this->getParam(PARAM::Ep) != this->getParam(PARAM::Ek) ) {
            _lineEdits[lid_points]->setDisabled(true);
        }

        if ( this->getParam(PARAM::pro) == PARAM::pro_yes ) { // potencjał programowany
            _checkboxIsPro->setChecked(true);
            _butProgramPotential->setEnabled(true);
        } else {
            _checkboxIsPro->setChecked(false);
            _butProgramPotential->setEnabled(false);
        }

        this->checkPotentialProgram();
        this->methodChanged();
    }
}

void EAQtParamDialog::sanitizeDialog()
{
}

void EAQtParamDialog::cancelAndQuit()
{
    this->_dialog->close();
    return;
}

void EAQtParamDialog::saveAndQuit()
{
    this->saveParams();
    if ( !_wasSaved ) {
        return;
    }
    this->_dialog->close();
}

void EAQtParamDialog::saveAndStart()
{
    this->saveParams();
    if ( !_wasSaved ) {
        return;
    }
    this->_dialog->close();
    this->_pData->MesStart(this->_isLsv);
}

void EAQtParamDialog::setCGMDE(int num, int val)
{
    switch (num) {
    case PARAM::valveCntr:
        this->_cgmdeNr = val;
        break;
    case PARAM::valveTime:
        this->_cgmdeTime = val;
        break;
    case PARAM::valveDelay:
        this->_cgmdeDelay = val;
        break;
     case PARAM::cgmdeMode:
        this->_cgmdeMode = val;
        break;
     case 99:
        this->_dropsBeforeMes = val;
    default:
        return;
    }
}

int EAQtParamDialog::getCGMDE(int num)
{
    switch (num) {
    case PARAM::valveCntr:
        return this->_cgmdeNr;
        break;
    case PARAM::valveTime:
        return this->_cgmdeTime;
        break;
    case PARAM::valveDelay:
        return this->_cgmdeDelay;
        break;
    case PARAM::cgmdeMode:
        return this->_cgmdeMode;
        break;
    case 99:
        return this->_dropsBeforeMes;
    default:
        return 0;
    }
}

void EAQtParamDialog::setBreaks(int n, int v)
{
    this->_breaksData[n-PARAM::breakCntr-1] = v;
}

int EAQtParamDialog::getBreaks(int n)
{
    return this->_breaksData[n-PARAM::breakCntr-1];
}

void EAQtParamDialog::saveParams()
{
    _wasSaved = false;
    this->setParam(PARAM::pro,PARAM::pro_no);
    if ( !_isLsv ) {
        if ( _checkboxIsPro->isChecked() ) {
            if ( EAQtData::getInstance().getPotentialProgram().size() < 3 ) {
                QMessageBox mb(_dialog);
                mb.setWindowTitle(tr("Error"));
                mb.setText(tr("Potential program requires at least three points."));
                mb.exec();
                return;
            } else if ( _paramSampl[PARAM::sampl_double]->isChecked()
            && (EAQtData::getInstance().getPotentialProgram().size()%2!=0) ) {
                QMessageBox mb(_dialog);
                mb.setWindowTitle(tr("Error"));
                mb.setText(tr("Double sampling requires even number of points for potential program."));
                mb.exec();
                return;
            } else {
                this->setParam(PARAM::pro,PARAM::pro_yes);
            }
        }
    } else {
        if ( _startEbox->isChecked() ) {
            setParam(PARAM::Escheck,PARAM::Escheck_yes);
        } else {
            setParam(PARAM::Escheck,PARAM::Escheck_no);
        }
    }
    int aver = this->_lineEdits[lid_aver]->text().toInt();
    if (aver <= 0) {
        aver = 1;
    }
    this->setParam(PARAM::aver, aver);
    this->setParam(PARAM::breakCntr, this->_lineEdits[lid_breaks]->text().toInt());
    this->setParam(PARAM::dE, this->_lineEdits[lid_E0_dE]->text().toInt());
    this->setParam(PARAM::Ek, this->_lineEdits[lid_Ek]->text().toInt());
    this->setParam(PARAM::Ep, this->_lineEdits[lid_Ep]->text().toInt());
    this->setParam(PARAM::Estep, this->_lineEdits[lid_Estep]->text().toInt());
    this->setParam(PARAM::mesStirrerDelay, this->_lineEdits[lid_mixer]->text().toInt());
    this->setParam(PARAM::ptnr, this->_lineEdits[lid_points]->text().toInt());
    this->setParam(PARAM::td, this->_lineEdits[lid_td]->text().toInt());
    this->setParam(PARAM::tp, this->_lineEdits[lid_tp]->text().toInt());
    this->setParam(PARAM::tw, this->_lineEdits[lid_tw]->text().toInt());
    this->setParam(PARAM::valveCntr,this->_cgmdeNr);
    this->setParam(PARAM::valveTime,this->_cgmdeTime);
    this->setParam(PARAM::valveDelay,this->_cgmdeDelay);
    this->setParam(PARAM::cgmdeMode,this->_cgmdeMode);
    _pData->setDropsBeforeMes(this->_dropsBeforeMes);

    for ( int i = 0; i< 21; ++i ) {
        this->setParam(PARAM::breakCntr+1+i,this->_breaksData[i]);
    }


    for ( int i = 0; i<_paramElec.size(); ++i ) {
        if ( this->_paramElec[i]->isChecked() ) {
            this->setParam(PARAM::electr, i);
            if (i == PARAM::electr_dme) {
                this->setParam(PARAM::mespv, PARAM::mespv_polarography);
            } else {
                this->setParam(PARAM::mespv, PARAM::mespv_voltammetry);
            }

            break;
        }
    }

    for ( int i = 0; i<_paramEl23.size(); ++i ) {
        if ( this->_paramEl23[i]->isChecked() ) {
            this->setParam(PARAM::el23, i);
            break;
        }
    }

    for ( int i = 0; i<_paramSampl.size(); ++i ) {
        if ( this->_paramSampl[i]->isChecked() ) {
            this->setParam(PARAM::sampl, i);
            break;
        }
    }

    for ( int i = 0; i<_paramMessc.size(); ++i ) {
        if ( this->_paramMessc[i]->isChecked() ) {
            this->setParam(PARAM::messc, i);
            break;
        }
    }

    for ( int i = 0; i<_paramCrangeMacro.size(); ++i ) {
        if ( this->_paramCrangeMacro[i]->isChecked() ) {
            this->setParam(PARAM::crange, i);
            break;
        }
    }

    for ( int i = 0; i<_paramCrangeMicro.size(); ++i ) {
        if ( this->_paramCrangeMicro[i]->isChecked() ) {
            this->setParam(PARAM::crange, i);
            break;
        }
    }

    if ( this->_isLsv ) {
        for ( int i = 0; i<_paramLSVdEdt.size(); ++i ) {
            if ( this->_paramLSVdEdt[i]->isChecked() ) {
                this->setParam(PARAM::dEdt, i);
                break;
            }
        }
        this->setParam(PARAM::multi,0);
        this->setParam(PARAM::method, PARAM::method_lsv);
    } else {
        for ( int i = 0; i<_paramMethod.size(); ++i ) {
            if ( this->_paramMethod[i]->isChecked() ) {
                switch (i) {
                case method_scv:
                    this->setParam(PARAM::method, PARAM::method_scv);
                    break;
                case method_npv:
                    this->setParam(PARAM::method, PARAM::method_npv);
                    break;
                case method_dpv:
                    this->setParam(PARAM::method, PARAM::method_dpv);
                    break;
                case method_osqw:
                    this->setParam(PARAM::method, PARAM::method_sqw_osteryoung);
                    break;
                case method_csqw:
                    this->setParam(PARAM::method, PARAM::method_sqw_classic);
                    this->calculateFreq();
                    int tp = this->_lineEdits[lid_tp]->text().toInt();
                    int tw = this->_lineEdits[lid_tw]->text().toInt();
                    this->setParam(PARAM::tp, this->_lineEdits[lid_tp]->text().toInt());
                    this->setParam(PARAM::tw, this->_lineEdits[lid_tw]->text().toInt());
                    break;
                }
                break;
            }
        }
        double freq = round(_lineEdits[lid_sqw_freq]->text().toDouble());
        this->setParam(PARAM::sqw_frequency, (int32_t)round(_lineEdits[lid_sqw_freq]->text().toDouble()));
        if ( _advWidgets.isMultielectrode->isChecked() ) {
            for ( uint i = 0; i<_advWidgets.useChannel.size(); ++i ) {
                _pData->setChannelName(i, _advWidgets.channelNames[i]->text());
                _pData->setChannelEnabled(i, _advWidgets.useChannel[i]->isChecked());
            }
            setParam(PARAM::electr, PARAM::electr_multiSolid);
        } else {
            setParam(PARAM::multi, 0);
        }
        setParam(PARAM::nonaveragedsampling, (int32_t)(!_advWidgets.nonaveraged->isChecked()));
        if ( _advWidgets.useMesFile->isChecked() ) {
            QString fp = _advWidgets.mesFilePath->text();
            _pData->setUseMesFile(true);
            if ( !fp.isEmpty() ) {
                _pData->setMesSeriesFile(fp);
            } else {
                _pData->setMesSeriesFile("");
                _pData->setUseMesFile(false);
            }
        } else {
            _pData->setUseMesFile(false);
        }
    }
    _wasSaved = true;
}

void EAQtParamDialog::methodChanged()
{
    if ( !_checkboxIsPro->isChecked() ) {
        _lineEdits[lid_tw]->setEnabled(true);
        _lineEdits[lid_tp]->setEnabled(true);
        _lineLabels[lid_tw]->setEnabled(true);
        _lineLabels[lid_tp]->setEnabled(true);
        _lineLabels[lid_E0_dE]->setEnabled(true);
        _lineEdits[lid_E0_dE]->setEnabled(true);
        _lineLabels[lid_sqw_freq]->setVisible(false);
        _lineEdits[lid_sqw_freq]->setVisible(false);

        if ( _paramMethod[method_scv]->isChecked() ) {
            _lineLabels[lid_E0_dE]->setEnabled(false);
            _lineEdits[lid_E0_dE]->setEnabled(false);
            _lineLabels[lid_Estep]->setText("Estep [mV]:");
        } else if ( _paramMethod[method_npv]->isChecked() ) {
            _lineLabels[lid_E0_dE]->setText("E0 [mV]:");
            _lineLabels[lid_Estep]->setText("Estep [mV]:");
        } else if ( _paramMethod[method_dpv]->isChecked()
        || _paramMethod[method_osqw]->isChecked() ) {
            _lineLabels[lid_E0_dE]->setText("dE [mV]:");
            _lineLabels[lid_Estep]->setText("Estep [mV]:");
        } else if ( _paramMethod[method_csqw]->isChecked() ) { // == PARAM::method_sqw_classic
            _lineEdits[lid_tw]->setEnabled(false);
            _lineEdits[lid_tp]->setEnabled(false);
            _lineLabels[lid_tw]->setEnabled(false);
            _lineLabels[lid_tp]->setEnabled(false);
            _lineLabels[lid_E0_dE]->setText("dE [mV]:");
            _lineLabels[lid_Estep]->setText("Estep [mV]:");
            _lineLabels[lid_sqw_freq]->setVisible(true);
            _lineEdits[lid_sqw_freq]->setVisible(true);
        }
    }
}

void EAQtParamDialog::showPotentialProgram()
{
    EAQtParamPotentialProgram *ppp = new EAQtParamPotentialProgram(_dialog);
    ppp->exec();
    delete ppp;
}

void EAQtParamDialog::checkPotentialProgram()
{
    if ( _checkboxIsPro->isChecked() ) {
        _lineEdits[lid_Ek]->setDisabled(true);
        _lineEdits[lid_Ep]->setDisabled(true);
        _lineEdits[lid_E0_dE]->setDisabled(true);
        _lineEdits[lid_Estep]->setDisabled(true);
        _lineEdits[lid_breaks]->setDisabled(true);
        _lineEdits[lid_breaks]->setText("0");
    } else {
        _lineEdits[lid_Ek]->setEnabled(true);
        _lineEdits[lid_Ep]->setEnabled(true);
        _lineEdits[lid_E0_dE]->setEnabled(true);
        _lineEdits[lid_Estep]->setEnabled(true);
        _lineEdits[lid_breaks]->setEnabled(true);
        this->methodChanged();
    }
}

void EAQtParamDialog::calculateFreq()
{
    int desiredFreq = (int)round(_lineEdits[lid_sqw_freq]->text().toDouble());
    int tp = (int)round(1000.0 / (double)desiredFreq / 2.0);
    int tw = 0;
    if (tp > 5) {
        tw = tp - 5;
        tp = tp - tw;
    } else {
        tw = 0;
    }
    double calculated = 1000.0/(2.0*(double)(tp+tw));
    _lineEdits[lid_sqw_freq]->setText(tr("%1").arg(calculated));
    _lineEdits[lid_tp]->setText(tr("%1").arg(tp));
    _lineEdits[lid_tw]->setText(tr("%1").arg(tw));
}

void EAQtParamDialog::multielectrodeChange(bool status)
{
    if (status) {
        this->_checkboxIsMicro->setChecked(true);
        this->microelectrodeChanged(true);
        this->_checkboxIsMicro->setDisabled(true);
        this->_paramElec[PARAM::electr_microSolid]->setChecked(true);
        this->_paramElec[PARAM::electr_microCgmde]->setDisabled(true);
    } else {
        this->_checkboxIsMicro->setDisabled(false);
        this->_paramElec[PARAM::electr_microCgmde]->setDisabled(false);
    }
}
