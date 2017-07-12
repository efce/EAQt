
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
#ifndef _CURVEDATA_H_
#define _CURVEDATA_H_ 1

#include <QVector>
#include <stdint.h>
#include "const.h"

/*
 * Class containing experimental data from Curve
 */
class CurveData
{
public:
    CurveData(TYPES::VectorSize);
    void addDataPoint(double, double, double, TYPES::VectorSize);
    void addDataPoint(double current, TYPES::VectorSize pointNumber);
	int getNrOfPoints();

    void allocateProbingData(TYPES::VectorSize nLen ,double dProbingRate);
	void addProbingDataPoint(float);
    void setProbingData(QVector<double>&);
    QVector<double>* getProbingData();
    TYPES::VectorSize getNumberOfProbingPoints();
    QVector<double>* getProbingDataPointNumbers();

    QVector<double>* getPotentialVector();
    double getPotential(TYPES::VectorSize index);
    void setPotentialVector(QVector<double>);

    QVector<double>* getTimeVector();
    double getTime(TYPES::VectorSize index);
    void setTimeVector(QVector<double>);

    QVector<double>* getCurrentVector();
    double getCurrent(TYPES::VectorSize index);
    void setCurrent(TYPES::VectorSize index, double value);

    void setMesCurrent1Point(TYPES::VectorSize, int64_t);
    void addToMesCurrent1Point(TYPES::VectorSize, int64_t);
    int64_t getMesCurrent1Point(TYPES::VectorSize);
    void setMesCurrent2Point(TYPES::VectorSize, int64_t);
    void addToMesCurrent2Point(TYPES::VectorSize, int64_t);
    int64_t getMesCurrent2Point(TYPES::VectorSize);
    void setMesTimePoint(TYPES::VectorSize, int64_t);
    void addToMesTimePoint(TYPES::VectorSize, int64_t);
    int64_t getMesTimePoint(TYPES::VectorSize);

    void allocateMesArray(TYPES::VectorSize numberOfFields, bool allocateTwoCurrent);

    int64_t* mesResCurrent1;
    int64_t* mesResCurrent2;
    int64_t* mesResTime;
	bool isMesAllocated;
	bool allocatedTwoCurrent;

public:
	~CurveData(void);
private:
    QVector<double> _vCurrent;
    QVector<double> _vPotential;
    QVector<double> _vTime;
    TYPES::VectorSize _nrOfPoints;
    double _probingRate;
    QVector<double> _vProbingData;
    QVector<double> _vProbingDataPointNo;
    TYPES::VectorSize _nrOfPointsProbing;

};

#endif
