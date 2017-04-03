
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
#include "./Eigen/Dense"
#include "eaqtdata.h"
#include "eaqtsignalprocessing.h"
#include "eaqtcalibrationdialog.h"

extern double testF95[22], testF99[20];
double expshift;
#define TINY 1.0e-20;

EAQtSignalProcessing::EAQtSignalProcessing(CurveCollection *cc, QCPGraph *graph) : QObject()
{
    _curves = cc;
    graph->setVisible(false);
    _graph = graph;
    // TODO: probably to rewrite:
    _interParam[0] = 1;
    _interParam[1] = 0;
    _interParam[2] = 7;
    _interParam[3] = 3;
    _interParam[4] = 0;
    _interParam[5] = 0;
    _interParam[6] = 7;
    _interParam[7] = 15;
    _interParam[8] = 7;
    _interParam[9] = 0;
    bkg_flag = 0;
}

void EAQtSignalProcessing::shiftCurve(double dY)
{
    Curve* c = _curves->get(EAQtData::getInstance().Act());
    if ( c == NULL )
        return;
    QVector<double> values = c->getYVector();
    for ( int i = 0; i<values.size(); ++i ) {
        c->setYValue(i, values[i]+dY);
    }
}

void EAQtSignalProcessing::calibrationData(uint32_t a1, uint32_t a2)
{
    QVector<double> calY;
    calY.resize(_curves->count());
    for ( uint i = 0; i<_curves->count(); ++i ) {
        Curve *curve;
        curve = _curves->get(i);
        QVector<double> values = curve->getYVector();
        double min = values[a1];
        double max = values[a2];
        for ( uint32_t pos=a1; pos<=a2; ++pos ) {
            if ( values[pos] > max ) {
                max = values[pos];
            } else if ( values[pos] < min ) {
                min = values[pos];
            }
        }
        calY[i] = max - min;
    }
    EAQtCalibrationDialog *cd = new EAQtCalibrationDialog(calY);
    cd->exec();
    delete cd;
}

void EAQtSignalProcessing::linearRegression(QVector<double> x, QVector<double> y, double* slope, double* intercept)
{
    Eigen::MatrixXd A;
    A.resize(x.size(),2);
    for ( int i = 0; i<x.size(); ++i ) {
        A(i,0) = 1;
        A(i,1) = x[i];
    }
    Eigen::VectorXd b;
    b.resize(y.size());
    for ( int i = 0; i<y.size();++i ) {
        b(i) = y[i];
    }
    Eigen::VectorXd leastSquare = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
    *intercept = leastSquare(0);
    *slope = leastSquare(1);
}

void EAQtSignalProcessing::polynomialFit(QVector<double> x, QVector<double> y, int order, QVector<double> *coeff)
{
    Eigen::MatrixXd A(x.size(), order+1);
    Eigen::VectorXd y_mapped = Eigen::VectorXd::Map(&y.front(), y.size());
    Eigen::VectorXd result;

    assert(x.size() == y.size());
    assert(x.size() >= order+1);

    // create matrix
    for (int i = 0; i < x.size(); i++) {
        for (int j = 0; j < order+1; j++) {
            A(i, j) = pow(x.at(i), j);
        }
    }

    // solve for linear least squares fit
    result = A.householderQr().solve(y_mapped);

    coeff->resize(order+1);
    for (int i = 0; i < order+1; i++) {
        coeff->replace(i, result[i]);
     }
}

void EAQtSignalProcessing::sgSmooth(QVector<double> *y, int order, int span)
{
    QVector<double>x(span);
    for ( int i =0; i<span; ++i ) {
        x[i]=i+1;
    }
    QVector<double> coeff;
    double newy;
    int ii;
    int posy = ceil(span/2);
    for ( int i = 0; i<y->size()-span; ++i) {
        polynomialFit(x, y->mid(i,span), order, &coeff);
        newy=0;
        for (ii=0; ii<=order;++ii) {
            newy += coeff[ii]*pow(posy,ii);
        }
        y->replace(i+posy-1, newy);
    }

    polynomialFit(x, y->mid(0,span), order, &coeff);
    for ( int i = 0; i<posy-1; ++i) {
        newy=0;
        for (ii=0; ii<=order;++ii) {
            newy += coeff[ii]*pow(i+1,ii);
        }
        y->replace(i,newy);
    }

    polynomialFit(x, y->mid(y->size()-span-1,span), order, &coeff);
    for ( int i = posy+1; i<=span; ++i) {
        newy=0;
        for (ii=0; ii<=order;++ii) {
            newy += coeff[ii]*pow(i+1,ii);
        }
        y->replace(i+y->size()-span-1,newy);
    }
}

void EAQtSignalProcessing::generateBackground(uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4)
{
    Curve *c = _curves->get(EAQtData::getInstance().Act());
    bkg_curveX = c->getXVector();
    QVector<double> bx;
    QVector<double> by;
    bx = c->getXVector().mid(r1,r2);
    bx.append(c->getXVector().mid(r3,r4));

    by=c->getYVector().mid(r1,r2);
    by.append(c->getYVector().mid(r3,r4));

    QVector<double> coeff;
    polynomialFit(bx,by,3,&coeff);
    bkg_curveY.resize(bkg_curveX.size());
    for ( int i = 0; i<c->getXVector().size(); ++i ) {
        bkg_curveY[i] = coeff[3]*pow(bkg_curveX[i],3) + coeff[2]*pow(bkg_curveX[i],2) + coeff[1]*bkg_curveX[i] + coeff[0];
    }

    _graph->setData(bkg_curveX, bkg_curveY);
    _graph->setVisible(true);
}

void EAQtSignalProcessing::subtractBackground()
{
    Curve* c = _curves->get(EAQtData::getInstance().Act());
    QVector<double> values = c->getYVector();
    for ( int i = 0 ; i<values.size(); ++i ) {
        c->setYValue(i,values[i] - bkg_curveY[i]);
    }
    _graph->setVisible(false);
}

void EAQtSignalProcessing::hideBackground()
{
    _graph->setVisible(false);
}

void EAQtSignalProcessing::correlation(QVector<double> x, QVector<double> y, double* correlationCoef)
{
    double w;
    w = 0;
    for ( int i = 0; i<x.size(); ++i) {
        w+=x[i];
    }
    double meanx = w/x.size();

    w=0;
    for ( int i = 0; i<y.size(); ++i ) {
        w+=y[i];
    }
    double meany = w/y.size();

    double sumtop = 0;
    for ( int i = 0; i<x.size(); ++i ) {
        sumtop += (x[i] - meanx) * (y[i]-meany);
    }

    double wx = 0;
    double wy = 0;
    for ( int i = 0; i<x.size(); ++i ) {
        wx += pow((x[i] - meanx),2);
        wy += pow((y[i] - meany),2);
    }

    if ( wx == 0 || wy == 0 ) {
        *correlationCoef = 0.0;
        return;
    }

    *correlationCoef = sumtop/sqrt(wx*wy);
}
