
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

/*
 * Class containing experimental data from Curve
 */
class CurveData
{
public:
    CurveData(uint32_t);
    void addDataPoint(double, double, double, uint32_t);
    void addDataPoint(double current, uint32_t pointNumber);
	int getNrOfPoints();

    void allocateProbingData(uint32_t nLen ,double dProbingRate);
	void addProbingDataPoint(float);
    void setProbingData(QVector<double>);
    QVector<double>* getProbingData();
    uint32_t getNumberOfProbingPoints();
    QVector<double>* getProbingDataPointNumbers();

    QVector<double>* getPotentialVector();
    double getPotential(uint32_t index);
    void setPotentialVector(QVector<double>);

    QVector<double>* getTimeVector();
    double getTime(uint32_t index);
    void setTimeVector(QVector<double>);

    QVector<double>* getCurrentVector();
    double getCurrent(uint32_t index);
    void setCurrent(uint32_t index, double value);

    void setMesCurrent1Point(uint32_t, int64_t);
    void addToMesCurrent1Point(uint32_t, int64_t);
    int64_t getMesCurrent1Point(uint32_t);
    void setMesCurrent2Point(uint32_t, int64_t);
    void addToMesCurrent2Point(uint32_t, int64_t);
    int64_t getMesCurrent2Point(uint32_t);
    void setMesTimePoint(uint32_t, int64_t);
    void addToMesTimePoint(uint32_t, int64_t);
    int64_t getMesTimePoint(uint32_t);

    void allocateMesArray(uint32_t numberOfFields, bool allocateTwoCurrent);

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
    int _nrOfPoints;
    double _probingRate;
    QVector<double> _vProbingData;
    QVector<double> _vProbingDataPointNo;
    int _nrOfPointsProbing;

};

#endif
