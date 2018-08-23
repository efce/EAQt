
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
    QLabel *labNrImp  = new QLabel(tr("Numer of impulses:"));
    QLineEdit *lneNrImp = new QLineEdit(QString::number(pd->getCGMDE(PARAM::valveCntr)));
    lneNrImp->setAccessibleName("41");
    QLabel *labTimeValve  = new QLabel(tr("Valve [ms]:"));
    QLineEdit *lneTimeValve = new QLineEdit(QString::number(pd->getCGMDE(PARAM::valveTime)));
    lneTimeValve->setAccessibleName("42");
    QLabel *labNrImpBreak  = new QLabel(tr("Impulse delay [ms]:"));
    QLineEdit *lneNrImpBreak = new QLineEdit(QString::number(pd->getCGMDE(PARAM::valveDelay)));
    lneNrImpBreak->setAccessibleName("43");
    QPushButton *ok = new QPushButton(tr("Confirm"));
    QObject::connect(ok,SIGNAL(clicked()),this,SLOT(Confirmed()));
    QPushButton *cancel = new QPushButton("Cancel");
    QObject::connect(cancel,SIGNAL(clicked()),_dialog,SLOT(close()));
    grid->addWidget(labNrImp,0,0);
    grid->addWidget(lneNrImp,0,1);
    grid->addWidget(labTimeValve,1,0);
    grid->addWidget(lneTimeValve,1,1);
    grid->addWidget(labNrImpBreak,2,0);
    grid->addWidget(lneNrImpBreak,2,1);
    grid->addWidget(ok,3,0);
    grid->addWidget(cancel,3,1);
    _dialog->setLayout(grid);
    _dialog->setMaximumHeight(10);
    this->_dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void EAQtParamCGMDE::Confirmed()
{
    EAQtParamDialog* pd = (EAQtParamDialog*) this->_parent;
    foreach( QLineEdit *label, _dialog->findChildren<QLineEdit*>() ) {
        pd->setCGMDE(label->accessibleName().toInt(),label->text().toInt());
    }
    _dialog->close();
}

void EAQtParamCGMDE::showCGMDE()
{
    _dialog->show();
}
