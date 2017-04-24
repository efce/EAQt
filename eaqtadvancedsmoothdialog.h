
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
#ifndef EAQTADVANCEDSMOOTHDIALOG_H
#define EAQTADVANCEDSMOOTHDIALOG_H

#include <QObject>
#include <QDialog>
#include <QRadioButton>
#include <QPushButton>
#include "./Qcustomplot/qcustomplot.h"

class EAQtAdvancedSmoothDialog : public QObject
{
    Q_OBJECT

private:
    QDialog *_dialog;
    QVector<QRadioButton*> _radMethod;
    QLineEdit *_leSGSpan;
    QLineEdit *_leSGOrder;
    QLineEdit *_leFTreshhold;
    QPushButton *_butSmooth;
    QPushButton *_butCancel;
    QPushButton *_butShowFrequency;
    QCustomPlot *_plotFreq;
    QCPGraph *_graph;
    std::vector<double> _params;
    enum method {
        method_sg,
        method_spline,
        method_fourier
    };
    enum paramsList {
        pl_order,
        pl_span,
        pl_treshhold,
    };

    QGridLayout* generateLayout(int select);
    void updateFrequencyPlot();

public:
    EAQtAdvancedSmoothDialog(int lastUsed, const std::vector<double>& params);
    void exec();
    void hide();

public slots:
    void methodChanged();

};

#endif // EAQTADVANCEDSMOOTHDIALOG_H
