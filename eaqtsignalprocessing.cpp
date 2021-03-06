
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

EAQtSignalProcessing::EAQtSignalProcessing(QCPCurve *graph) : QObject()
{
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
    CurveCollection* _curves = EAQtData::getInstance().getCurves();
    Curve* c = _curves->get(EAQtData::getInstance().Act());
    if ( c == nullptr )
        return;
    QVector<double> values = c->getYVector();
    for ( int i = 0; i<values.size(); ++i ) {
        c->setYValue(i, values[i]+dY);
    }
}

void EAQtSignalProcessing::calibrationData(QVector<std::array<int,2>>& coords)
{
    CurveCollection* _curves = EAQtData::getInstance().getCurves();
    CalibrationData *calibration = EAQtData::getInstance()._calibration;
    static QHash<QString,QString> oldSettings;
    calibration->yValues.resize(_curves->count());
    for ( int32_t i = 0; i<_curves->count(); ++i ) {
        calibration->yValues[i] = relativeHeight(_curves->get(i), coords[i][0],coords[i][1]);
    }
    int act = EAQtData::getInstance().Act();
    if ( act < 0 ) {
        act = 0;
    }
    calibration->pointStart = _curves->get(act)->getXVector().at(coords[act][1]);
    calibration->pointEnd = _curves->get(act)->getXVector().at(coords[act][0]);
    calibration->xAxis = EAQtData::getInstance().getXAxis();
    calibration->curves = _curves;
    EAQtCalibrationDialog *cd = new EAQtCalibrationDialog(calibration, &oldSettings);
    cd->exec();
    delete cd;
}

void EAQtSignalProcessing::curvesStats(QVector<std::array<int,2>>& coords)
{
    CurveCollection* _curves = EAQtData::getInstance().getCurves();
    int n = _curves->count();
    QVector<double> bkg_std_vec;
    QVector<double> minmax(n);
    double stdDev = 0;
    double peakMean = 0;
    for ( int i = 0; i < n; ++i ) {
        QVector<double> v_tmp = _curves->get(i)->getYVector().mid(coords[i][0], coords[i][1]-coords[i][0]);
        stdDev += calcStdDev(v_tmp);
        minmax[i] = relativeHeight(_curves->get(i), coords[i][0], coords[i][1]);
        peakMean += minmax[i];
    }
    stdDev /= n;
    peakMean /= n;
    double peakStdDev = calcStdDev(minmax);
    QString text;
    text += tr("Peak height: %1 ± %2 µA <br>").arg(peakMean,0,'f',4).arg(peakStdDev,0,'f',4);
    text += tr("Baseline Standard Deviation: %1 µA <br>").arg(stdDev,0,'f',4);
    QMessageBox mb;
    mb.setWindowTitle(tr("Stats"));
    mb.setText(text);
    mb.exec();
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

void EAQtSignalProcessing::linearRegression(
        const QVector<double>& x,
        const QVector<double>& y,
        double* slope,
        double* slopeStdDev,
        double* intercept,
        double* interceptStdDev,
        double* x0StdDev
    )
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
    xavg = xavg / n;
    yavg = yavg / n;
    for ( i =0; i<n;++i ) {
        sr += pow((y[i]-ypred[i]),2);
        sxx += pow((x[i]-xavg),2);
    }
    sr = sqrt(sr/(n-2));

    *slopeStdDev = sr/sqrt(sxx);
    *interceptStdDev = sr*sqrt(sumx2/(n*sxx));

    int y0Index = -1;
    for ( i=0; i<n;++i) {
        if ( x[i]==0.0 ) {
            y0Index = i;
            break;
        }
    }
    if ( y0Index > -1 ) {
        *x0StdDev = sr / *slope * sqrt(1 + (1/n) + (pow(y[y0Index]-yavg, 2) / (pow(*slope, 2) * sxx)));
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

void EAQtSignalProcessing::medianfilter(
        const QVector<double>& signal,
        QVector<double>& result,
        int windowSize
    )
{
    // CODE BASED ON: http://www.librow.com/articles/article-1
    QVector<double> result_temp = signal;
    result = QVector<double>(signal);
    const int N = signal.size();
    int halfWindow = (int)floor(windowSize/2);
    //   Move window through all elements of the signal
    for (int i = halfWindow; i<N-2; ++i) {
        //   Pick up window elements
        double window[windowSize];
        for (int j = 0; j < windowSize; ++j) {
            window[j] = signal[i-halfWindow+j];
        }
        //   Order elements (only half of them)
        for (int j=0; j<halfWindow+1; ++j) {
            //   Find position of minimum element
            int min = j;
            for (int k = j+1; k<windowSize; ++k) {
                if (window[k] < window[min]) {
                    min = k;
                }
            }
            //   Put found minimum element in its place
            const double temp = window[j];
            window[j] = window[min];
            window[min] = temp;
        }
        //   Get result - the middle element
        result_temp[i-halfWindow] = window[halfWindow];
    }
    for ( int i=halfWindow; i<N-halfWindow; ++i) {
        result[i] = result_temp[i-halfWindow];
    }
}

void EAQtSignalProcessing::showBackground()
{
    this->showBackground(this->bkg_curveX, this->bkg_curveY);
}

void EAQtSignalProcessing::showBackground(QVector<double> x, QVector<double> y)
{
    QPen pn = QPen(COLOR::background);
    pn.setStyle(Qt::DashLine);
    pn.setWidth(2);
    _graph->setPen(pn);
    _graph->setData(x, y);
    _graph->setVisible(true);
}

QVector<double> EAQtSignalProcessing::generateBackground(Curve* c, int32_t r1, int32_t r2, int32_t r3, int32_t r4)
{
    this->_cursorIndex[0] = r1;
    this->_cursorIndex[1] = r2;
    this->_cursorIndex[2] = r3;
    this->_cursorIndex[3] = r4;
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
        if (bx.size()<2) {
            throw(2);
        }
        polynomialFit(bx,by,1,&coeff);
        bkg_curveY.resize(bkg_curveX.size());
        for ( int i = 0; i<c->getXVector().size(); ++i ) {
            bkg_curveY[i] = coeff[1]*bkg_curveX[i] + coeff[0];
        }
        break;
    case poly2:
        if (bx.size()<3) {
            throw(3);
        }
        polynomialFit(bx,by,2,&coeff);
        bkg_curveY.resize(bkg_curveX.size());
        for ( int i = 0; i<c->getXVector().size(); ++i ) {
            bkg_curveY[i] = coeff[2]*pow(bkg_curveX[i],2) + coeff[1]*bkg_curveX[i] + coeff[0];
        }
        break;
    default:
    case poly3:
        if (bx.size()<4) {
            throw(4);
        }
        polynomialFit(bx,by,3,&coeff);
        bkg_curveY.resize(bkg_curveX.size());
        for ( int i = 0; i<c->getXVector().size(); ++i ) {
            bkg_curveY[i] = coeff[3]*pow(bkg_curveX[i],3) + coeff[2]*pow(bkg_curveX[i],2) + coeff[1]*bkg_curveX[i] + coeff[0];
        }
        break;
    case exponential:
        if (bx.size()<4) {
            throw(4);
        }
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
    return this->bkg_curveY;
}

void EAQtSignalProcessing::subtractBackground(Curve* c, QVector<double> bkg_y)
{
    QVector<double> values = c->getYVector();
    for (int i = 0 ; i<values.size(); ++i) {
        c->setYValue(i,values[i] - bkg_y[i]);
    }
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

double EAQtSignalProcessing::tinv0975(uint degreesOfFreedom)
{
    switch ( degreesOfFreedom ) {
    case 0:
        return 0.0;
    case 1:
        return 12.706;
    case 2:
        return 4.303;
    case 3:
        return 3.182;
    case 4:
        return 2.7764;
    case 5:
        return 2.5706;
    default:
        return (exp(2.378/(double)degreesOfFreedom) + 0.96);
    }
}

double EAQtSignalProcessing::calcStdDev(QVector<double> yvals)
{
    int n = yvals.size();
    double sum = std::accumulate(yvals.begin(), yvals.end(), 0.0);
    double mean = sum / n;
    double wrk = 0;
    for ( int i = 0; i<n; ++i ) {
        wrk += pow((mean - yvals[i]),2);
    }
    return sqrt(wrk/(static_cast<double>(n)-1.0));
}

QVector<uint> EAQtSignalProcessing::findPeaks(QVector<double> y)
{
    QVector<uint> peaks;
    QVector<double> tmp;
    medianfilter(y, tmp);
    sgSmooth(&tmp, 3, 11);
    //sgSmooth(&tmp, 3, 15);
    if (tmp.length() < 10) {
        return peaks;
    }
    for (int i=4; i<tmp.length()-4; ++i) {
        if ( (tmp[i-2] < tmp[i-1])
        && (tmp[i-1] < tmp[i])
        && (tmp[i] > tmp[i+1])
        && (tmp[i+1] > tmp[i+2]) ) {
            peaks.append(i);
        }
        if ( (tmp[i-2] > tmp[i-1])
        && (tmp[i-1] > tmp[i])
        && (tmp[i] < tmp[i+1])
        && (tmp[i+1] < tmp[i+2]) ) {
            peaks.append(i);
        }
    }
    return peaks;
    /*
    QVector<double> ydiff(tmp.size()-1);
    for (uint i=0; i<(tmp.size()-1); ++i) {
        ydiff[i] = tmp[i+1] - tmp[i];
    }
    const int test_req[3] = {2, 7, 2};
    int test_count[4] = {0, 0, 0, 0};
    bool test_positive_slope = true;
    int test_step = -1;
    for (int i=0; i<(ydiff.size()-1); ++i) {
        if (test_step == 3) { // Peak was found //
            peaks.append(i - test_count[1] - ceil((double)test_count[2]/2.0));
            test_count[0] = 0;
            test_count[1] = 0;
            test_count[2] = 0;
            test_step = -1;
            i = (i - test_count[2]) + 1;
        }
        if ( test_step == -1 ) {
            if (ydiff[i] > ydiff[i+1]) {
                test_positive_slope = false;
                test_step = 0;
            } else if (ydiff[i] < ydiff[i+1]) {
                test_positive_slope = true;
                test_step = 0;
            }
        } else {
            bool grow = (ydiff[i] < ydiff[i+1]);
            if ( (grow && test_positive_slope)
            || (!grow && !test_positive_slope)
            || (ydiff[i] == ydiff[i+1]) ) {
                test_count[test_step] += 1;
            } else {
                if (test_count[test_step] >= test_req[test_step]) {
                    test_step += 1;
                    test_count[test_step] = 1;
                    test_positive_slope = !test_positive_slope;
                } else {
                    int go_back = test_count[0] + test_count[1] + test_count[2] - 2;
                    if (go_back > 0) {
                        if ((i - go_back) > 0) {
                            i -= go_back;
                        }
                    }
                    test_step = -1;
                    test_count[0] = 0;
                    test_count[1] = 0;
                    test_count[2] = 0;
                    test_count[3] = 0;
                }
            }
        }
    }
    return peaks;
    */
}

int EAQtSignalProcessing::secondSignificantDigitDecimalPlace(double value)
{
    double l = floor(log10(value));
    if(l>0.0) {
        return 0;
    } else {
        return int(-(l-1.0));
    }
}

double EAQtSignalProcessing::calcLOD(double slope, double slopeStdDev, double intercept, double interceptStdDev, double blankStdDev, double k)
{
    return (k/slope) * sqrt(pow(blankStdDev, 2) + pow(interceptStdDev, 2) + pow(slopeStdDev, 2)*pow((intercept/slope), 2));
}
