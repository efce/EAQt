
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
#include "eaqtaccessoriesdialog.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

EAQtAccessoriesDialog::EAQtAccessoriesDialog(EAQtDataInterface* pd) : QObject()
{
    this->_pData = pd;
    this->_dialog = new QDialog();
    this->_dialog->setWindowTitle(tr("Accessories settings"));
    QHBoxLayout *mainBox = new QHBoxLayout();
    mainBox->addLayout(createForms());
    mainBox->addLayout(createButtons());
    this->_dialog->setLayout(mainBox);
    this->_dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);

}

void EAQtAccessoriesDialog::exec()
{
    this->_dialog->exec();

}

QGridLayout* EAQtAccessoriesDialog::createButtons()
{
    QGridLayout* gl = new QGridLayout();
    this->_buttons.resize(5);

    this->_buttons[0] = new QPushButton( tr("Test hammer") );
    connect(this->_buttons[0],SIGNAL(clicked(bool)),this,SLOT(testHammer()));
    this->_buttons[1] = new QPushButton( tr("Test CGMDE valve (10 ms)") );
    connect(this->_buttons[1],SIGNAL(clicked(bool)),this,SLOT(testCGMDE()));
    this->_buttons[2] = new QPushButton( tr("Apply") );
    connect(this->_buttons[2],SIGNAL(clicked(bool)),this,SLOT(apply()));
    this->_buttons[3] = new QPushButton( tr("OK") );
    connect(this->_buttons[3],SIGNAL(clicked(bool)),this,SLOT(ok()));
    this->_buttons[4] = new QPushButton( tr("Cancel") );
    connect(this->_buttons[4],SIGNAL(clicked(bool)),this->_dialog,SLOT(close()));

    for ( int i=0; i<this->_buttons.size(); ++i) {
        gl->addWidget(this->_buttons[i],i,0,1,1);

    }

    return gl;
}

QGridLayout* EAQtAccessoriesDialog::createForms()
{
    QGridLayout *gl = new QGridLayout();
    QFontMetrics *metrics = new QFontMetrics(this->_dialog->font());

    QHBoxLayout *hbl = new QHBoxLayout();

    this->_forms.resize(3);

    QLabel* labMix = new QLabel(tr("mixer speed [%]:"));
    this->_forms[0] = new QLineEdit();
    this->_forms[0]->setText(tr("%1").arg(this->_pData->ParamPV(PARAM::stirrerSpeed)));
    this->_forms[0]->setValidator(new QIntValidator(0,100));
    this->_forms[0]->setFixedWidth(metrics->width("9999"));

    QLabel *labHamForce = new QLabel(tr("hammer force [%]:"));
    this->_forms[1] = new QLineEdit();
    this->_forms[1]->setText(tr("%1").arg(this->_pData->ParamPV(PARAM::knockPower)));
    this->_forms[1]->setValidator(new QIntValidator(0,100));
    this->_forms[1]->setFixedWidth(metrics->width("9999"));

    QLabel *labHammerTime = new QLabel(tr("hammer time [ms]:"));
    this->_forms[2] = new QLineEdit();
    this->_forms[2]->setText(tr("%1").arg(this->_pData->ParamPV(PARAM::knockTime)));
    this->_forms[2]->setValidator(new QIntValidator(0,999));
    this->_forms[2]->setFixedWidth(metrics->width("9999"));


    QGroupBox *deaeration = new QGroupBox(tr("deaeration:"));
    deaeration->setFixedWidth(metrics->width("--deaeration:--"));
    QVBoxLayout *layDe = new QVBoxLayout();
    this->_gas.resize(2);
    this->_gas[0] = new QRadioButton(tr("no"));
    this->_gas[1] = new QRadioButton(tr("yes"));
    layDe->addWidget(this->_gas[1]);
    layDe->addWidget(this->_gas[0]);
    deaeration->setLayout(layDe);
    this->_gas[this->_pData->isGasOn()]->setChecked(true);

    QGroupBox *mixing = new QGroupBox(tr("mixing:"));
    mixing->setFixedWidth(deaeration->width());
    QVBoxLayout *layMix = new QVBoxLayout();
    this->_mixer.resize(2);
    this->_mixer[0] = new QRadioButton(tr("no"));
    this->_mixer[1] = new QRadioButton(tr("yes"));
    layMix->addWidget(this->_mixer[1]);
    layMix->addWidget(this->_mixer[0]);
    mixing->setLayout(layMix);
    this->_mixer[this->_pData->isMixerOn()]->setChecked(true);

    hbl->addWidget(deaeration);
    hbl->addWidget(mixing);

    gl->addWidget(labMix,0,0,1,2,Qt::AlignRight);
    gl->addWidget(this->_forms[0],0,2,1,1);
    gl->addWidget(labHamForce,1,0,1,2,Qt::AlignRight);
    gl->addWidget(this->_forms[1],1,2,1,1);
    gl->addWidget(labHammerTime,2,0,1,2,Qt::AlignRight);
    gl->addWidget(this->_forms[2],2,2,1,1);

    gl->addLayout(hbl,3,0,1,3);

    delete metrics;
    return gl;
}

void EAQtAccessoriesDialog::saveAndSend()
{
    this->_pData->ParamPV(PARAM::stirrerSpeed,this->_forms[0]->text().toInt());
    this->_pData->ParamPV(PARAM::knockPower,this->_forms[1]->text().toInt());
    this->_pData->ParamPV(PARAM::knockTime,this->_forms[2]->text().toInt());
    if ( this->_gas[0]->isChecked() ) {
        this->_pData->setGas(false);
    } else {
        this->_pData->setGas(true);
    }
    if ( this->_mixer[0]->isChecked() ) {
        this->_pData->setMixer(false);
    } else {
        this->_pData->setMixer(true);
    }
    this->_pData->sendAccessories();
}

void EAQtAccessoriesDialog::ok()
{
    saveAndSend();
    this->_dialog->close();
}

void EAQtAccessoriesDialog::apply()
{
    saveAndSend();
}

void EAQtAccessoriesDialog::testCGMDE()
{
    this->_pData->setTestCGMDEValve(true);
    this->_pData->sendAccessories();
}

void EAQtAccessoriesDialog::testHammer()
{
    this->_pData->setTestHammer(true);
    this->_pData->sendAccessories();
}
