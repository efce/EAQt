
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
#include "eaqtuiinterface.h"
#include "curve.h"

class EAQtAdvancedSmoothDialog : public QObject
{
    Q_OBJECT

private:
    QDialog *_dialog;
    QVector<QRadioButton*> _radMethod;
    QLineEdit *_leSGSpan;
    QLineEdit *_leSGOrder;
    QLineEdit *_leFTreshhold;
    QPushButton *_butApply;
    QPushButton *_butClose;
    QCustomPlot *_plotFreq;
    QVector<QCPGraph*> _graphs;
    QVector<QVector<double>> _frequencies;
    QVector<double> _samplingFreq;
    QVector<QVector<double>> _imgValues;
    QVector<QVector<double>> _realValues;
    std::vector<double> _params;
    EAQtUIInterface *_pUI;
    enum method : int {
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
    void updateCurveFrequency(Curve* c);
    void trySG(Curve *c, int span, int order);

public:
    EAQtAdvancedSmoothDialog(int lastUsed, const std::vector<double>& params, EAQtUIInterface *pui);
    ~EAQtAdvancedSmoothDialog();
    void exec();
    void hide();

public slots:
    void methodChanged();
    void apply();

};

#endif // EAQTADVANCEDSMOOTHDIALOG_H
