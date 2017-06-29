
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
#include "eaqtbackgroundcorrectiondialog.h"
#include "eaqtsignalprocessing.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

EAQtBackgroundCorrectionDialog::EAQtBackgroundCorrectionDialog() : QObject()
{
    _dialog = new QDialog();
    _dialog->setModal(true);
    _dialog->setWindowTitle(tr("Background correction settings"));
    QVBoxLayout *vbl = new QVBoxLayout();

    QGroupBox *gbx = new QGroupBox(tr("Fit method:"));
    QVBoxLayout *gbxl = new QVBoxLayout();
    QVector<QString> fitnames = EAQtSignalProcessing::getFitNames();
    _fitMethods.resize(fitnames.size());
    for ( int i = 0; i<fitnames.size(); ++i) {
        QRadioButton *rb = new QRadioButton(fitnames[i]);
        if ( EAQtSignalProcessing::getFitMethod() == i ) {
            rb->setChecked(true);
        } else {
            rb->setChecked(false);
        }
        gbxl->addWidget(rb);
        _fitMethods[i] = rb;
    }
    gbx->setLayout(gbxl);
    QHBoxLayout *hbl = new QHBoxLayout();
    _butOK = new QPushButton(tr("OK"));
    connect(_butOK,SIGNAL(clicked(bool)),this,SLOT(OK()));
    hbl->addWidget(_butOK);
    _butClose = new QPushButton(tr("Close"));
    connect(_butClose,SIGNAL(clicked(bool)),_dialog,SLOT(close()));
    hbl->addWidget(_butClose);
    vbl->addWidget(gbx);
    vbl->addLayout(hbl);
    _dialog->setLayout(vbl);
}

EAQtBackgroundCorrectionDialog::~EAQtBackgroundCorrectionDialog()
{
    delete _dialog;
}

void EAQtBackgroundCorrectionDialog::exec()
{
    _dialog->exec();
}

void EAQtBackgroundCorrectionDialog::OK()
{
    for ( int i = 0; i<_fitMethods.size();++i ) {
        if ( _fitMethods[i]->isChecked() ) {
            EAQtSignalProcessing::setFitMethod(i);
            break;
        }
    }
    _dialog->close();
}
