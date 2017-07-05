
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
// save diagnostic state
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#include "./Eigen/Dense"
#include "./kiss_fft130/kissfft.hh"
#pragma GCC diagnostic pop

#include "eaqtdata.h"
#include "eaqtsignalprocessing.h"
#include "eaqtcalibrationdialog.h"

double EAQtSignalProcessing::_TINY = 1.0e-20;
double EAQtSignalProcessing::_PI = 2*asin(1.0);
int EAQtSignalProcessing::_selectedFitMethod = EAQtSignalProcessing::poly3;

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

void EAQtSignalProcessing::calibrationData(int32_t a1, int32_t a2)
{
    CalibrationData *calibration = EAQtData::getInstance()._calibration;
    static QHash<QString,QString> oldSettings;
    calibration->yValues.resize(_curves->count());
    for ( int32_t i = 0; i<_curves->count(); ++i ) {
        calibration->yValues[i] = relativeHeight(_curves->get(i), a1,a2);
    }
    calibration->pointStart = a1;
    calibration->pointEnd = a2;
    calibration->curves = _curves;
    EAQtCalibrationDialog *cd = new EAQtCalibrationDialog(calibration, &oldSettings);
    cd->exec();
    delete cd;
}

double EAQtSignalProcessing::relativeHeight(Curve *c, int32_t start, int32_t end)
{
        QVector<double> values = c->getYVector();
        if ( start > end ) {
            int32_t tmp = start;
            start = end;
            end = tmp;
        }

        double min = *std::min_element(&values[start],&values[end]);
        double max = *std::max_element(&values[start],&values[end]);
        return (max - min);
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
        inbuf[k]= cpx_type(values[k],0.0);
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

void EAQtSignalProcessing::kissIFFT(double samplingFreq,
                                    const QVector<double> &freqImg,
                                    const QVector<double> &freqReal,
                                    QVector<double> &values
                                   )
{
    int N = freqReal.size();

    typedef double T;

    typedef kissfft<T> FFT;
    typedef std::complex<T> cpx_type;

    FFT fft(N,true);
    vector<cpx_type> inbuf(N);
    vector<cpx_type> outbuf(N);
    for (int k=0;k<N;++k) {
        inbuf[k]= cpx_type(freqReal[k],freqImg[k]);
    }

    fft.transform( &inbuf[0] , &outbuf[0] );

    values.resize(N);
    double factor = N;
    for ( int i = 0; i<N; ++i ) {
        values[i] = outbuf[i].real()/factor;
    }
}

void EAQtSignalProcessing::linearRegression(const QVector<double>& x, const QVector<double>& y, double* slope, double* slopeStdDev, double* intercept, double* interceptStdDev, double* x0StdDev)
{
    Eigen::MatrixXd A;
    int n = y.size();
    int i = 0;
    A.resize(n,2);
    for ( i = 0; i<n; ++i ) {
        A(i,0) = 1;
        A(i,1) = x[i];
    }
    Eigen::VectorXd b;
    b.resize(n);
    for ( i = 0; i<n;++i ) {
        b(i) = y[i];
    }
    Eigen::VectorXd leastSquare = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
    *intercept = leastSquare(0);
    *slope = leastSquare(1);

    double sxx = 0, sr = 0, sumx2 = 0, xavg = 0, yavg = 0;
    QVector<double> ypred(n);
    for ( i = 0; i< n; ++i) {
        ypred[i] = (*slope)*x[i] + (*intercept);
        xavg += x[i];
        yavg += y[i];
        sumx2 += pow(x[i],2);
    }
    xavg/=n;
    yavg/=n;
    for ( i =0; i<n;++i ) {
        sr += pow(((y[i]-ypred[i])/(n-2)),2);
        sxx += pow((x[i]-xavg),2);
    }
    sr = sqrt(sr);

    *interceptStdDev = sqrt(pow(sr,2)/sxx);
    *slopeStdDev = sqrt((pow(sr,2)*sumx2)/(n*sxx));

    int y0Index = -1;
    for ( i=0; i<n;++i) {
        if ( x[i]==0.0 ) {
            y0Index = i;
            break;
        }
    }
    if ( y0Index > -1 ) {
        *x0StdDev = sr/(*intercept) * sqrt( 1+(1/n)+(pow(y[y0Index]-yavg,2)/(pow((*intercept),2)*sumx2)) );
    } else {
        *x0StdDev = -1;
    }
}

void EAQtSignalProcessing::polynomialFit(const QVector<double>& x, const QVector<double>& y, int order, QVector<double> *coeff)
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

void EAQtSignalProcessing::generateBackground(int32_t r1, int32_t r2, int32_t r3, int32_t r4)
{
    QPen pn = QPen(COLOR::background);
    pn.setStyle(Qt::DashLine);
    pn.setWidth(2);
    _graph->setPen(pn);
    Curve *c = _curves->get(EAQtData::getInstance().Act());
    bkg_curveX = c->getXVector();
    QVector<double> bx;
    QVector<double> by;
    int32_t rs[4];
    rs[0] = r1;
    rs[1] = r2;
    rs[2] = r3;
    rs[3] = r4;
    bool sorted = false;
    while ( !sorted ) {
        sorted = true;
        for ( int i = 0; i<3; ++i) {
            if ( rs[i] > rs[i+1] ) {
                int32_t t = rs[i];
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

#if QT_VERSION < 0x050500
    bx = c->getXVector().mid(r1,(r2-r1));
    QVector<double> tmp = c->getXVector().mid(r3,(r4-r3));
    int newsize = bx.size()+tmp.size();
    int oldsize = bx.size();
    bx.resize(newsize);
    for ( int i=oldsize; i<newsize;++i) {
        bx[i] = tmp[i-oldsize];
    }

    by=c->getYVector().mid(r1,(r2-r1));
    tmp = c->getYVector().mid(r3,(r4-r3));
    newsize = by.size()+tmp.size();
    oldsize = by.size();
    by.resize(newsize);
    for ( int i=oldsize; i<newsize;++i) {
        by[i] = tmp[i-oldsize];
    }
#else
    bx = c->getXVector().mid(r1,(r2-r1));
    bx.append(c->getXVector().mid(r3,(r4-r3)));

    by=c->getYVector().mid(r1,(r2-r1));
    by.append(c->getYVector().mid(r3,(r4-r3)));
#endif

    QVector<double> coeff;
    switch (EAQtSignalProcessing::_selectedFitMethod) {
    case poly1:
        polynomialFit(bx,by,1,&coeff);
        bkg_curveY.resize(bkg_curveX.size());
        for ( int i = 0; i<c->getXVector().size(); ++i ) {
            bkg_curveY[i] = coeff[1]*bkg_curveX[i] + coeff[0];
        }
        break;
    case poly2:
        polynomialFit(bx,by,2,&coeff);
        bkg_curveY.resize(bkg_curveX.size());
        for ( int i = 0; i<c->getXVector().size(); ++i ) {
            bkg_curveY[i] = coeff[2]*pow(bkg_curveX[i],2) + coeff[1]*bkg_curveX[i] + coeff[0];
        }
        break;
    default:
    case poly3:
        polynomialFit(bx,by,3,&coeff);
        bkg_curveY.resize(bkg_curveX.size());
        for ( int i = 0; i<c->getXVector().size(); ++i ) {
            bkg_curveY[i] = coeff[3]*pow(bkg_curveX[i],3) + coeff[2]*pow(bkg_curveX[i],2) + coeff[1]*bkg_curveX[i] + coeff[0];
        }
        break;
    case exponential:
        int nn = by.size();
        QVector<double> ylog(nn);
        for ( int i =0; i<nn;++i ) {
            ylog[i] = log(by[i]);
        }
        polynomialFit(bx,ylog,1,&coeff);
        bkg_curveY.resize(bkg_curveX.size());
        for ( int i = 0; i<c->getXVector().size(); ++i ) {
            bkg_curveY[i] = exp(coeff[0])*exp(bkg_curveX[i] * coeff[1]);
        }
        break;
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

void EAQtSignalProcessing::correlation(const QVector<double>& x, const QVector<double>& y, double* correlationCoef)
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

QVector<QString> EAQtSignalProcessing::getFitNames()
{
    QVector<QString> names(SIZE,"");
    names[poly1] = tr("y=p1*x+p2");
    names[poly2] = tr("y=p1*x^2+p2*x+p3");
    names[poly3] = tr("y=p1*x^3+p2*x^2+p3*x+p4");
    names[exponential] =   tr("y=p1*exp(x*p2)");
    return names;
}

int EAQtSignalProcessing::getFitMethod()
{
    return EAQtSignalProcessing::_selectedFitMethod;
}

void EAQtSignalProcessing::setFitMethod(int method)
{
    EAQtSignalProcessing::_selectedFitMethod = method;
}
