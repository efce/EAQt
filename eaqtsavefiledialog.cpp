
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
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardPaths>
#include "eaqtsavefiledialog.h"
#include "const.h"

EAQtSaveFiledialog::EAQtSaveFiledialog(QWidget* parent, QString cname, QString ccomment, QString pathToShow, QString file) : QObject()
{
    this->_parent = parent;
    this->_pathToShow = pathToShow;
    _ccomment = ccomment;
    _cname = cname;
    _filename = file;
    _saveDetails.wasCanceled = false;
    _saveDetails.curveComment = _ccomment;
    _saveDetails.curveName = _cname;
    _saveDetails.fileName = _filename;
}

EAQtSaveFiledialog::SaveDetails EAQtSaveFiledialog::getSaveDetails()
{
    QFileDialog* fd = new QFileDialog();
    if ( _pathToShow.isEmpty() && _filename.isEmpty() ) {
        fd->setDirectory(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    }
    if ( _pathToShow.isEmpty() ) {
        fd->setDirectory(_pathToShow);
    }
    if ( !_filename.isEmpty() ) {
        QFileInfo fi(_filename);
        fd->setDirectory(fi.absoluteDir());
        fd->selectFile(fi.baseName());
    }
    fd->selectFile(_filename);
    fd->setModal(true);
    fd->setOption( QFileDialog::DontUseNativeDialog, true );
    fd->setNameFilter(FILES::saveFile);
    if ( _filename.isEmpty()
    || _filename.right(FILES::saveCompressExt.size()).compare(FILES::saveCompressExt,Qt::CaseInsensitive) == 0 ) {
        fd->selectNameFilter(FILES::saveDef);
    } else {
        fd->selectNameFilter(FILES::saveDefUncompress);
    }
    QGridLayout* l = (QGridLayout*) fd->layout();
    QGridLayout* lay = new QGridLayout();
    this->_leCurveComment = new QPlainTextEdit();
    QFontMetrics *metrics = new QFontMetrics(fd->font());
    this->_leCurveComment->setFixedHeight(2.9*metrics->height() );
    _leCurveComment->setPlainText(_ccomment);
    QLabel* lbCurveComment = new QLabel(tr("Curve comment:"));

    this->_leCurveName = new QLineEdit();
    _leCurveName->setText(_cname);
    QLabel* lbCurveName = new QLabel(tr("Curve name:"));

    lay->addWidget(lbCurveName,0,0,1,1,Qt::AlignRight);
    lay->addWidget(this->_leCurveName,0,1,1,3);
    lay->addWidget(lbCurveComment,1,0,1,1,Qt::AlignRight);
    lay->addWidget(this->_leCurveComment,1,1,2,3);

    fd->setLabelText( QFileDialog::Accept, tr("Save") );
    fd->setLabelText( QFileDialog::Reject, tr("Cancel") );
    l->addLayout(lay,4,0,1,4);

    connect(this->_leCurveName,SIGNAL(textChanged(QString)),this,SLOT(updateCurveName(QString)));
    connect(this->_leCurveComment,SIGNAL(textChanged()),this,SLOT(updateCurveComment()));
    connect(fd,SIGNAL(fileSelected(QString)),this,SLOT(updateFileName(QString)));
    connect(fd,SIGNAL(rejected()),this,SLOT(wasCanceled()));

    fd->exec();

    if ( _saveDetails.fileName.isEmpty() ) {
        this->_saveDetails.wasCanceled = true;
    }

    return this->_saveDetails;
}

void EAQtSaveFiledialog::updateCurveComment()
{
    this->_saveDetails.curveComment = this->_leCurveComment->toPlainText();
}

void EAQtSaveFiledialog::updateCurveName(QString name)
{
    this->_saveDetails.curveName = name;
}

void EAQtSaveFiledialog::updateFileName(QString fn)
{
    this->_saveDetails.fileName = fn;
}

void EAQtSaveFiledialog::wasCanceled()
{
    _saveDetails.wasCanceled = true;
}
