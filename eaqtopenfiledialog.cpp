
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
#include <QtWidgets/QGridLayout>
#include <QObject>
#include "eaqtopenfiledialog.h"
#include "mdir.h"

EAQtOpenFileDialog::EAQtOpenFileDialog(EAQtData* pData) : QObject()
{
    this->_pData = pData;
    this->_fd = new QFileDialog();
    _fd->setOption( QFileDialog::DontUseNativeDialog, true );
    _fd->setNameFilter("volt (*.volt);;vol (*.vol)");
    QGridLayout* l = (QGridLayout*) _fd->layout();
    this->_curvesInFileList = new QListWidget();
    _curvesInFileList->setFixedWidth(QFontMetrics(_fd->font()).width("WWWWWWWWWWWWWWW"));
    _fd->setLabelText( QFileDialog::Accept, tr("Load all") );
    _fd->setLabelText( QFileDialog::Reject, tr("Close") );
    //this->curvesInFileList->setFixedHeight(200);
    l->addWidget(this->_curvesInFileList,0,3,2,1);
    connect(this->_fd,SIGNAL(currentChanged(QString)),this,SLOT(updateList(QString)));
    connect(_curvesInFileList,SIGNAL(clicked(QModelIndex)),this,SLOT(loadSelected(QModelIndex)));
    connect(_fd,SIGNAL(fileSelected(QString)),this,SLOT(loadFile(QString)));
    _fd->show();
}

EAQtOpenFileDialog::~EAQtOpenFileDialog()
{
    delete _curvesInFileList;
    delete _fd;
}

void EAQtOpenFileDialog::updateList(QString fileToShow)
{
    QFileInfo *fileinfo = new QFileInfo(fileToShow);
    if ( !fileinfo->isFile() ) {
        this->_fileSelected.clear();
        delete fileinfo;
        return;
    }
    this->_fileSelected = fileToShow;
    QFile *file = new QFile(fileToShow);
    if ( !file->open(QIODevice::ReadOnly) ) {
        return;
    }
    if ( file->fileName().right(4).compare(".vol",Qt::CaseInsensitive) == 0 ) {
        this->_pData->MDirReadOld(*file);
        MDirCollection* vDir = this->_pData->getMDir();
        file->close();
        delete file;
        delete fileinfo;
        this->_curvesInFileList->clear();
        if ( vDir->count() > 0 ) {
            for (uint i = 0; i<vDir->count(); ++i) {
                this->_curvesInFileList->addItem(vDir->get(i)->CName());
            }
        }
    } else {
        this->_pData->MDirReadPro(*file);
        MDirCollection* vDir = this->_pData->getMDir();
        file->close();
        delete file;
        delete fileinfo;
        this->_curvesInFileList->clear();
        if ( vDir->count() > 0 ) {
            for (uint i = 0; i<vDir->count(); ++i) {
                this->_curvesInFileList->addItem(vDir->get(i)->CName());
            }
        }
    }
}

void EAQtOpenFileDialog::loadSelected(QModelIndex qmi)
{
    int idToLoad = qmi.row();
    this->_pData->openFile(&this->_fileSelected,idToLoad);
}

void EAQtOpenFileDialog::loadFile(QString filename)
{
    this->_pData->openFile(&filename,0);
}
