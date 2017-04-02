
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
#include <QtWidgets>
#include "eaqtparambreaks.h"
#include "eaqtparamdialog.h"

EAQtParamBreaks::EAQtParamBreaks(QObject *parent, int nBreaks) : QObject()
{
    this->_wasInitialized = false;
    if (nBreaks == 0) {
        return;
    }
    this->_wasInitialized = true;

    this->_dialog = new QDialog();
    this->_dialog->setWindowTitle(tr("Breaks setttings"));
    this->_dialog->setModal(true);
    this->_parent = parent;
    this->_breaksActive = nBreaks;
    this->_dialog->setMaximumHeight(nBreaks+5);
    this->_dialog->setLayout(this->createLayout());
    this->_dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
    this->_dialog->exec();
}

EAQtParamBreaks::~EAQtParamBreaks()
{
    if ( this->_wasInitialized )
        delete this->_dialog;
}


QGridLayout* EAQtParamBreaks::createLayout()
{
    int const nMaxBreaks = 8;
    QGridLayout* grid = new QGridLayout;
    QLabel *lineHedears[4];
    lineHedears[0] = new QLabel(tr("break # "));
    lineHedears[1] = new QLabel(tr(" E [mV] "));
    lineHedears[2] = new QLabel(tr(" t [min] "));
    lineHedears[3] = new QLabel(tr(" t [s] "));
    for (int i=0;i<4;i++) {
        grid->addWidget(lineHedears[i],0,i,1,1,Qt::AlignCenter);
    }

    QLineEdit *lineEdits[3*nMaxBreaks];
    QLabel *lineLabels[nMaxBreaks];
    QFontMetrics *fm = new QFontMetrics(this->_dialog->font());
    EAQtParamDialog* pd = (EAQtParamDialog*) this->_parent;
    for (int i=0;i<this->_breaksActive;i++) {
        lineEdits[3*i+0] = new QLineEdit(tr("%1").arg(pd->getBreaks(PARAM::breakE+i)));
        lineEdits[3*i+0]->setAccessibleName(tr("%1mv").arg(i+1));
        lineEdits[3*i+0]->setFixedWidth(fm->width("999999"));
        lineEdits[3*i+0]->setValidator(pd->getValidator(EAQtParamDialog::vt_potential));
        lineEdits[3*i+1] = new QLineEdit(tr("%1").arg(pd->getBreaks(PARAM::breakmin+i)));
        lineEdits[3*i+1]->setFixedWidth(fm->width("9999"));
        lineEdits[3*i+1]->setAccessibleName(tr("%1min").arg(i+1));
        lineEdits[3*i+1]->setValidator(pd->getValidator(EAQtParamDialog::vt_mins));
        lineEdits[3*i+2] = new QLineEdit(tr("%1").arg(pd->getBreaks(PARAM::breaksec+i)));
        lineEdits[3*i+2]->setAccessibleName(tr("%1sec").arg(i+1));
        lineEdits[3*i+2]->setFixedWidth(fm->width("999"));
        lineEdits[3*i+2]->setValidator(pd->getValidator(EAQtParamDialog::vt_secs));
        lineLabels[i] = new QLabel (tr("%1:").arg(i+1));
        grid->addWidget(lineLabels[i],i+1,0,1,1,Qt::AlignCenter);
        grid->addWidget(lineEdits[3*i+0],i+1,1,1,1,Qt::AlignCenter);
        grid->addWidget(lineEdits[3*i+1],i+1,2,1,1,Qt::AlignCenter);
        grid->addWidget(lineEdits[3*i+2],i+1,3,1,1,Qt::AlignCenter);
    }

    QPushButton *apply = new QPushButton;
    apply->setText(tr("Save"));
    apply->setFixedHeight(30);
    connect(apply,SIGNAL(clicked()),this,SLOT(applyBreaks()));

    QPushButton *cancel = new QPushButton;
    cancel->setText(tr("Cancel"));
    cancel->setFixedHeight(30);
    connect(cancel,SIGNAL(clicked()),_dialog,SLOT(close()));

    grid->addWidget(apply,this->_breaksActive+1,0,1,2);
    grid->addWidget(cancel,this->_breaksActive+1,2,1,2);

    grid->setVerticalSpacing(2);
    grid->setContentsMargins(1,1,5,5);

    return grid;
}

void EAQtParamBreaks::applyBreaks()
{
    EAQtParamDialog* pd = (EAQtParamDialog*) this->_parent;
    foreach( QLineEdit *label, this->_dialog->findChildren<QLineEdit*>() ) {
        if ( label->accessibleName().left(1).toInt() == 0 )
            continue;

        if ( label->accessibleName().right(3) == "min" )
            pd->setBreaks(PARAM::breakmin+label->accessibleName().left(1).toInt()-1,
                                       label->text().toInt() );
        else if ( label->accessibleName().right(3) == "sec" )
            pd->setBreaks(PARAM::breaksec+label->accessibleName().left(1).toInt()-1,
                                       label->text().toInt() );
        else if ( label->accessibleName().right(2) == "mv" )
            pd->setBreaks(PARAM::breakE+label->accessibleName().left(1).toInt()-1,
                                       label->text().toInt() );
        else
            throw 0;
    }
    this->_dialog->close();
}
