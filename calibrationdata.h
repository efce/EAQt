#ifndef CALIBRATIONDATA_H
#define CALIBRATIONDATA_H

#include <QVector>
#include "curvecollection.h"

class CalibrationData
{
public:
    CalibrationData();
    void load(QFile *file);
    void save(QFile *file, bool includeCurves);

    bool wasFitted;
    bool curvesIncluded;
    double slope;
    double slopeStdDev;
    double intercept;
    double interceptStdDev;
    double x0StdDev;
    double correlationCoef;
    QVector<double> xValues;
    QString xUnits;
    QVector<double> yValues;
    QString yUnits;
    CurveCollection *curves;
};

#endif // CALIBRATIONDATA_H
