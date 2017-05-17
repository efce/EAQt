#include "calibrationdata.h"

CalibrationData::CalibrationData()
{
    wasFitted = false;
    xUnits = "";
    yUnits = "";
    xValues.resize(0);
    yValues.resize(0);
    slope = 0;
    slopeStdDev = -1;
    intercept = 0;
    interceptStdDev = -1;
    x0StdDev = -1;
    x0value = 0;
    correlationCoef = 0;
    pointEnd = 0;
    pointStart = 0;
}

void CalibrationData::save(QFile *file, bool includeCurves)
{
    char wrt;
    int n, l;
    QByteArray ba;
    wrt = (char)includeCurves;
    file->write(&wrt,1);
    n = this->yValues.size();
    file->write((char*)&n,sizeof(int));

    file->write((char*)this->xValues.data(),sizeof(double)*n);
    ba = this->xUnits.toUtf8();
    l = ba.size();
    file->write((char*)&l,sizeof(int));
    file->write(ba);
    file->write((char*)this->yValues.data(),sizeof(double)*n);

    ba = this->yUnits.toUtf8();
    l = ba.size();
    file->write((char*)&l,sizeof(int));
    file->write(ba);
    wrt = (char)this->wasFitted;
    file->write((char*)&wrt,1);
    file->write((char*)&(this->pointStart),sizeof(uint32_t));
    file->write((char*)&(this->pointEnd),sizeof(uint32_t));
    file->write((char*)&(this->slope),sizeof(double));
    file->write((char*)&(this->slopeStdDev),sizeof(double));
    file->write((char*)&(this->intercept),sizeof(double));
    file->write((char*)&(this->interceptStdDev),sizeof(double));
    file->write((char*)&(this->x0value),sizeof(double));
    file->write((char*)&(this->x0StdDev),sizeof(double));
    file->write((char*)&(this->correlationCoef),sizeof(double));
}

void CalibrationData::load(QFile *file)
{
    char wrt;
    int n,l;
    QByteArray ba;
    file->read((char*)&wrt,1);
    curvesIncluded = (bool)wrt;
    file->read((char*)&n,sizeof(int));
    this->xValues.resize(n);
    this->yValues.resize(n);
    file->read((char*)this->xValues.data(),sizeof(double)*n);
    file->read((char*)&l,sizeof(int));
    ba.resize(l);
    file->read(ba.data(),l);
    this->xUnits = QString::fromUtf8(ba);
    file->read((char*)this->yValues.data(),sizeof(double)*n);
    file->read((char*)&l,sizeof(int));
    ba.resize(l);
    file->read(ba.data(),l);
    this->yUnits = QString::fromUtf8(ba);
    file->read((char*)&wrt,1);
    this->wasFitted = (bool)wrt;
    file->read((char*)&(this->pointStart),sizeof(uint32_t));
    file->read((char*)&(this->pointEnd),sizeof(uint32_t));
    file->read((char*)&(this->slope),sizeof(double));
    file->read((char*)&(this->slopeStdDev),sizeof(double));
    file->read((char*)&(this->intercept),sizeof(double));
    file->read((char*)&(this->interceptStdDev),sizeof(double));
    file->read((char*)&(this->x0value),sizeof(double));
    file->read((char*)&(this->x0StdDev),sizeof(double));
    file->read((char*)&(this->correlationCoef),sizeof(double));
}
