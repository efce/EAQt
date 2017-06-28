
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
#ifndef EAQTTESTCGMDEDIALOG_H
#define EAQTTESTCGMDEDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include "eaqtdata.h"

class EAQtTestCGMDEDialog : public QObject
{
    Q_OBJECT

public:
    EAQtTestCGMDEDialog();
    ~EAQtTestCGMDEDialog();
public slots:
    void show();
    void close();
    void updateTest();

private:
    QDialog *_dialog;
    EAQtData::TestCGMDE *_values;

    QLineEdit *_leValveTime;
    QLineEdit *_leBreakTime;
    QLineEdit *_lePotential;

    QLineEdit *_leTestsNum;
    QLineEdit *_leAverageResult;
    QLineEdit *_leStandardDev;
    QLineEdit *_lePulseNum;
    QCheckBox *_cbUsePotential;

    QPushButton *_butTestStart;
    QPushButton *_butTestStop;
    QPushButton *_butClose;
    QPushButton *_butReset;

    int _testsNum;
    double _averageResult;
    double _stardardDev;
    int _pulseNum;
    QVector<int> _vResults;

    void updateDialog();
    void readDialog();
    bool sendTestToEA();
    void calculateTest();

private slots:
    void resetResults();
    void TestStart();
    void TestStop();
};

#endif // EAQTTESTCGMDEDIALOG_H
