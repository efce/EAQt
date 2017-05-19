
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
#ifndef EAQTAVERAGEDIALOG_H
#define EAQTAVERAGEDIALOG_H

#include <QObject>
#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include "eaqtuiinterface.h"

class EAQtAverageDialog : public QObject
{
    Q_OBJECT

public:
    EAQtAverageDialog(EAQtUIInterface* pui);
    ~EAQtAverageDialog();
    void exec();

private:
    QDialog *_dialog;
    QVector<QCheckBox*> _list;
    QPushButton *_butAverage;
    QPushButton *_butClose;
    QPushButton *_butSelectAll;
    QPushButton *_butSelectNone;
    QVBoxLayout *_listBox;
    QScrollArea *_sa;
    QWidget *_scrollAreaWidget;
    EAQtUIInterface* _pUI;
    void generateList();
    void showError();

private slots:
    void average();
    void selectNone();
    void selectAll();
};

#endif // EAQTAVERAGEDIALOG_H
