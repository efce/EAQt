
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
#include "eaqtparamcgmde.h"
#include "eaqtparamdialog.h"

EAQtParamCGMDE::EAQtParamCGMDE(QObject *parent) : QObject()
{
    this->_parent = parent;
    this->_dialog = new QDialog();
    EAQtParamDialog* pd = (EAQtParamDialog*) this->_parent;
    _dialog->setModal(true);
    _dialog->setWindowTitle(tr("CGMDE settings"));
    QGridLayout *grid = new QGridLayout;
    QLabel *labValveCntr  = new QLabel(tr("Number of impulses:"));
    QLineEdit *lneValveCntr = new QLineEdit(QString::number(pd->getCGMDE(PARAM::valveCntr)));
    lneValveCntr->setAccessibleName("41");
    QLabel *labTimeValve  = new QLabel(tr("Valve [ms]:"));
    QLineEdit *lneTimeValve = new QLineEdit(QString::number(pd->getCGMDE(PARAM::valveTime)));
    lneTimeValve->setAccessibleName("42");
    QLabel *labValveDelay  = new QLabel(tr("Impulse delay [ms]:"));
    QLineEdit *lneValveDelay = new QLineEdit(QString::number(pd->getCGMDE(PARAM::valveDelay)));
    lneValveDelay->setAccessibleName("43");

    _cgmdeMode = new QGroupBox(tr("CGMDE mode:"));
    _cgmdeMode->setAccessibleName("44");
    QVBoxLayout *vbMode = new QVBoxLayout();
    QRadioButton* drop_mes = new QRadioButton(tr("Single drop"));
    drop_mes->setAccessibleName("0");
    QRadioButton* drop_step = new QRadioButton(tr("New drop before each step"));
    drop_step->setAccessibleName("1");
    QRadioButton* grow_norm = new QRadioButton(tr("Grow drop before each step"));
    grow_norm->setAccessibleName("2");
    QRadioButton* grow_sum = new QRadioButton(tr("Grow and sum current"));
    grow_sum->setAccessibleName("3");
    QRadioButton* grow_lin = new QRadioButton(tr("Grow linearly and sum"));
    grow_lin->setAccessibleName("4");
    vbMode->addWidget(drop_mes);
    vbMode->addWidget(drop_step);
    vbMode->addWidget(grow_norm);
    vbMode->addWidget(grow_sum);
    vbMode->addWidget(grow_lin);
    vbMode->addStretch(0);
    vbMode->setSpacing(1);
    _cgmdeMode->setLayout(vbMode);
    foreach(QRadioButton* mode, _cgmdeMode->findChildren<QRadioButton*>()) {
        if (mode->accessibleName().toInt() == pd->getCGMDE(PARAM::cgmdeMode)) {
            mode->setChecked(true);
            break;
        }
    }

    QPushButton *ok = new QPushButton(tr("Confirm"));
    QObject::connect(ok,SIGNAL(clicked()),this,SLOT(Confirmed()));
    QPushButton *cancel = new QPushButton("Cancel");
    QObject::connect(cancel,SIGNAL(clicked()),_dialog,SLOT(close()));
    grid->addWidget(labValveCntr, 0, 0);
    grid->addWidget(lneValveCntr, 0, 1);
    grid->addWidget(labTimeValve, 1, 0);
    grid->addWidget(lneTimeValve, 1, 1);
    grid->addWidget(labValveDelay, 2, 0);
    grid->addWidget(lneValveDelay, 2, 1);
    grid->addWidget(_cgmdeMode, 0, 2, 5, 1);
    grid->addWidget(ok, 3, 0);
    grid->addWidget(cancel, 3, 1);
    _dialog->setLayout(grid);
    _dialog->setMaximumHeight(10);
    this->_dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void EAQtParamCGMDE::Confirmed()
{
    EAQtParamDialog* pd = (EAQtParamDialog*) this->_parent;

    foreach(QLineEdit *label, _dialog->findChildren<QLineEdit*>()) {
        pd->setCGMDE(label->accessibleName().toInt(),label->text().toInt());
    }

    QListIterator<QRadioButton*> modes(_cgmdeMode->findChildren<QRadioButton*>());
    while (modes.hasNext()) {
        QRadioButton* mode = modes.next();
        if (mode->isChecked()) {
            pd->setCGMDE(_cgmdeMode->accessibleName().toInt(), mode->accessibleName().toInt());
            break;
        }
    }

    _dialog->close();
}

void EAQtParamCGMDE::showCGMDE()
{
    _dialog->show();
}
