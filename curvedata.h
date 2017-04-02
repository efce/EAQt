
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
	CurveData(int);
	void addDataPoint(double, double, double, int);
	void addDataPoint(double current, int pointNumber);
	int getNrOfPoints();

	void allocateProbingData(int nLen ,double dProbingRate);
	void addProbingDataPoint(float);
    void setProbingData(QVector<double>);
    QVector<double>* getProbingData();
    uint32_t getNumberOfProbingPoints();
    QVector<double>* getProbingDataPointNumbers();

    QVector<double>* getPotentialVector();
	double getPotential(int index);
    void setPotentialVector(QVector<double>);

    QVector<double>* getTimeVector();
	double getTime(int index);
    void setTimeVector(QVector<double>);

    QVector<double>* getCurrentVector();
	double getCurrent(int index);
	void setCurrent(int index, double value);

	void setMesCurrent1Point(int, long);
	long* getMesCurrent1Point(int);
	void setMesCurrent2Point(int, long);
	long* getMesCurrent2Point(int);
	void setMesTimePoint(int, long);
	long* getMesTimePoint(int);

	void allocateMesArray(int numberOfFields, bool allocateTwoCurrent);

	long* mesResCurrent1;
	long* mesResCurrent2;
	long* mesResTime;
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
