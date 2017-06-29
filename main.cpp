
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
#include <QApplication>
#include "./pstdin/pstdin.h" //Added for Windows compatibility //
#include "const.h"
#include "eaqtmainwindow.h"
#include "eaqtdata.h"
#include "eaqtdatainterface.h"
#include "eaqtuiinterface.h"

int main(int argc, char *argv[])
{
    QLocale::setDefault(QLocale("en_US"));

    QApplication a(argc, argv);
    QFont f(":/fonts/fonts/LiberationSans-Regular.ttf");
    f.setPixelSize(13);
    f.setKerning(true);
    a.setFont(f);

    QTranslator translator;
    translator.load("eaqt_pl",":/lang");
    a.installTranslator(&translator);

    EAQtMainWindow w;
    EAQtData::getInstance().initialUpdate((EAQtUIInterface*)&w);
    w.InitialUpdate(EAQtData::getInstance());
    QIcon logo(":/img/logo");
    w.setWindowIcon(logo);
    if ( argc > 0 ) {
        QFileInfo fi(argv[1]);
        if ( fi.isFile() ) {
            QString name = fi.absoluteFilePath();
            if ( name.right(5).compare(".volt",Qt::CaseInsensitive) == 0
            || name.right(6).compare(FILES::saveCompressExt,Qt::CaseInsensitive) == 0 ) {
                EAQtData::getInstance().CurReadFilePro(&name,0);
            } else if ( name.right(4).compare(".vol",Qt::CaseInsensitive) == 0 ) {
                EAQtData::getInstance().CurReadFileOld(&name,0);
            } else {
                QMessageBox mb;
                mb.setWindowTitle(w.tr("Error"));
                mb.setText(w.tr("Unsupported file type."));
            }
        }
    }
    w.show();
    return a.exec();
}
