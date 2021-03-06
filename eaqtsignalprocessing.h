
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
#ifndef EAQTSIGNALPROCESSING_H
#define EAQTSIGNALPROCESSING_H

#include "curvecollection.h"

class EAQtSignalProcessing : public QObject
{
    Q_OBJECT

public:
    EAQtSignalProcessing(QCPCurve* gr);
    QVector<double> generateBackground(Curve* c, int32_t a1, int32_t a2, int32_t b1, int32_t b2);
    void subtractBackground(Curve* c, QVector<double> bkg_y);
    void showBackground();
    void showBackground(QVector<double> x, QVector<double> y);
    void hideBackground();
    void calibrationData(QVector<std::array<int,2>>&);
    void curvesStats(QVector<std::array<int,2>>&);
    void shiftCurve(double dY);
    static void kissFFT(double samplingFrequency, const QVector<double>& values, QVector<double>& frequency, QVector<double>& freqReal, QVector<double>& freqImg);
    static void kissIFFT(double samplingFreq, const QVector<double>& freqImg, const QVector<double>& freqReal, QVector<double>& values);
    static void linearRegression(const QVector<double>& x, const QVector<double>& y, double* slope, double* slopeStdDev, double* intercept, double* interceptStdDev, double *x0StdDev);
    static void correlation(const QVector<double>& x, const QVector<double>& y, double* correlationCoef);
    static void polynomialFit(const QVector<double>& x, const QVector<double>& y, int order, QVector<double>* coeff);
    static void dft(double samplingFrequency, const QVector<double>& values, QVector<double>& frequency, QVector<double>& freqReal, QVector<double>& freqImg);
    static void idft(const QVector<double>& freqImg, const QVector<double>& freqReal, QVector<double>& values);
    static void sgSmooth(QVector<double>* y, int order, int span);
    static void medianfilter(const QVector<double>& signal, QVector<double>& result, int windowSize=5);
    static double relativeHeight(Curve* c, int32_t start, int32_t end);
    static double calcLOD(double slope, double slopeStdDev, double intercept, double interceptStdDev, double blankStdDev, double k);
    static int secondSignificantDigitDecimalPlace(double value);
    enum FitMethod : int {
        poly1,
        poly2,
        poly3,
        exponential,
        SIZE
    };
    static QVector<QString> getFitNames();
    static int getFitMethod();
    static void setFitMethod(int);
    static double tinv0975(uint x);
    static double calcStdDev(QVector<double>);
    QVector<uint> findPeaks(QVector<double> y);


private:
    static int _selectedFitMethod;
    QCPCurve* _graph;
    QVector<double> _backgroundX;
    QVector<double> _backgroundY;
    QDialog* _calibrationDialog;
    QDialog* _calibrationPlotDialog;
    QVector<double> bkg_curveX;
    QVector<double> bkg_curveY;
    int gen_info;
    int nn;
    int bkg_flag;
    int _interParam[10];
    uint32_t _cursorIndex[4];
    static double _PI;
    static double _TINY;
};

#endif // EAQTSIGNALPROCESSING_H
