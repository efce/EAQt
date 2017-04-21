
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
#include "./kiss_fft130/kissfft.hh"
#include "eaqtdata.h"
#include "eaqtsignalprocessing.h"
#include "eaqtcalibrationdialog.h"

double EAQtSignalProcessing::_TINY = 1.0e-20;
double EAQtSignalProcessing::_PI = 2*asin(1.0);

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

void EAQtSignalProcessing::kissFFT(double samplingFrequency,
                                   const QVector<double> &values,
                                   QVector<double> &frequency,
                                   QVector<double> &freqReal,
                                   QVector<double> &freqImg
                                  )
{
    int N = values.size();

    typedef double T;

    typedef kissfft<T> FFT;
    typedef std::complex<T> cpx_type;

    FFT fft(N,false);
    vector<cpx_type> inbuf(N);
    vector<cpx_type> outbuf(N);
    for (int k=0;k<N;++k) {
        inbuf[k]= cpx_type((T)values[k],(T)0);
    }

    fft.transform( &inbuf[0] , &outbuf[0] );

    freqImg.resize(N);
    freqReal.resize(N);
    frequency.resize(N);

    for ( int i =0; i<N; ++i ) {
        freqImg[i] = outbuf[i].imag();
        freqReal[i] = outbuf[i].real();
        frequency[i] = samplingFrequency*(double)i/(double)N;
    }
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
    uint32_t rs[4];
    rs[0] = r1;
    rs[1] = r2;
    rs[2] = r3;
    rs[3] = r4;
    bool sorted = false;
    while ( !sorted ) {
        sorted = true;
        for ( int i = 0; i<3; ++i) {
            if ( rs[i] > rs[i+1] ) {
                uint32_t t = rs[i];
                rs[i] = rs[i+1];
                rs[i+1] = t;
                sorted = false;
            }
        }
    }
    r1 = rs[0];
    r2 = rs[1];
    r3 = rs[2];
    r4 = rs[3];

    bx = c->getXVector().mid(r1,(r2-r1));
    bx.append(c->getXVector().mid(r3,(r4-r3)));

    by=c->getYVector().mid(r1,(r2-r1));
    by.append(c->getYVector().mid(r3,(r4-r3)));

    QVector<double> coeff;
    polynomialFit(bx,by,3,&coeff);
    bkg_curveY.resize(bkg_curveX.size());
    for ( int i = 0; i<c->getXVector().size(); ++i ) {
        bkg_curveY[i] = coeff[3]*pow(bkg_curveX[i],3) + coeff[2]*pow(bkg_curveX[i],2) + coeff[1]*bkg_curveX[i] + coeff[0];
    }

//    bkg_curveX.append(bx);
//    bkg_curveY.append(by);

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

void EAQtSignalProcessing::dft(double samplingFrequency, const QVector<double>& values, QVector<double>& frequency, QVector<double>& freqReal, QVector<double>& freqImg)
{
    int N = values.size();
    freqImg.resize( N );
    freqReal.resize( N );
    frequency.resize( N );
    /*
     * Exponential solution //
     *
    std::vector<std::complex<double>> X;
    X.resize(N);
    std::complex<double> i;
    i = -1;
    i = sqrt(i);
    complex<double> Wn = exp(i*(complex<double>)2.0*_PI/(complex<double>)N);
    for ( int k = 0; k<N; ++k) {
        X[k] = 0;
        for ( int n=0; n<N;++n) {
            X[k] += values->at(n) * pow(Wn, (complex<double>)(-k*n));
        }
        freqReal->replace(k, X[k].real());
        freqImg->replace(k, X[k].imag());
    }
    */

    // Sinusoidal solution //
    double angle;
    for ( int k = 0; k<N; ++k ) {
        freqImg.replace(k,0);
        freqReal.replace(k,0);
        frequency.replace(k, samplingFrequency*(double)k/(double)N  );
        for ( int n = 0; n<N; ++n ) {
            angle = -2.0 * _PI * (double)k * (double)n / (double)N;
            freqImg[k] += values[n]*sin(angle);//freqImg->replace(k, freqImg->at(k)+ values->at(n)*sin(angle));
            freqReal[k] += values[n]*cos(angle);
        }
    }
}

void EAQtSignalProcessing::idft(const QVector<double>& freqImg, const QVector<double>& freqReal, QVector<double>& values)
{
    int N = freqImg.size();
    values.resize(N);

    /*
     * Exponential solution
     */
    /*
    std::complex<double> i;
    i = -1;
    i = sqrt(i);
    complex<double> Wn = exp(i*(complex<double>)2.0*_PI/(complex<double>)N);
    complex<double> iv;
    for ( int n =0; n<N; ++n ) {
        iv = 0;
        for ( int k=0; k<N; ++k ) {
            complex<double> tmp;
            tmp = (complex<double>)freqReal->at(k) + (complex<double>)freqImg->at(k)*i;
            iv += tmp * pow(Wn,k*n) / (complex<double>)N;
        }
        values->replace(n, iv.real());
    }
    return;
*/

    /*
     * Sinusoidal solution
     */

    double img, real, iv, angle;
    for ( int n =0; n<N; ++n ) {
        img = 0;
        real = 0;
        iv = 0;
        for ( int k=0; k<N; ++k ) {
            angle = -2.0*_PI*(double)k*(double)n/(double)N;
            img += freqImg.at(k) * sin(angle);
            real += freqReal.at(k) * cos(angle);
        }
        iv = (img + real)/(double)N;
        values.replace(n, iv);
    }
}
