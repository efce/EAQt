
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
    w.show();
    return a.exec();
}
