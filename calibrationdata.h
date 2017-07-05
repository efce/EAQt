
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
    double x0value;
    double x0StdDev;
    double correlationCoef;
    int32_t pointStart;
    int32_t pointEnd;
    QVector<double> xValues;
    QString xUnits;
    QVector<double> yValues;
    QString yUnits;
    int32_t xAxis;
    CurveCollection *curves;
};

#endif // CALIBRATIONDATA_H
