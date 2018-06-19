
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
#include "curvedata.h"

CurveData::CurveData(TYPES::vectorindex_t dataSize)
{
    this->_vCurrent.resize(dataSize);
    this->_vPotential.resize(dataSize);
    this->_vTime.resize(dataSize);
	this->_nrOfPoints = 0;
	this->_nrOfPointsProbing = 0;
	this->isMesAllocated = false;
	this->allocatedTwoCurrent = false;
}

CurveData::~CurveData(void)
{
	if ( this->isMesAllocated ) {
		delete[] this->mesResCurrent1;
		delete[] this->mesResTime;
		if ( this->allocatedTwoCurrent ) {
			delete[] this->mesResCurrent2;
		}
	}
}

void CurveData::allocateProbingData(TYPES::vectorindex_t nLen, double dProbingRate)
{
	this->_probingRate = dProbingRate;
    this->_vProbingData.resize(nLen);
	this->_nrOfPointsProbing = 0;
    this->_vProbingDataPointNo.reserve(nLen);
    for ( double d = 1; d<=nLen; ++d ) {
        this->_vProbingDataPointNo.append(d);
    }
}
	
void CurveData::addDataPoint(double time, double potential, double current, TYPES::vectorindex_t pointNumber)
{
	if ( (pointNumber!= -1) && (pointNumber < this->_nrOfPoints) ) {
		this->_vCurrent[pointNumber]   = current;
		this->_vPotential[pointNumber] = potential;
		this->_vTime[pointNumber]      = time;
	} else {
		this->_vCurrent[this->_nrOfPoints]   = current;
		this->_vPotential[this->_nrOfPoints] = potential;
		this->_vTime[this->_nrOfPoints]      = time;
		this->_nrOfPoints++;
	}
}

void CurveData::addDataPoint(double current, TYPES::vectorindex_t pointNumber)
{
    if ( pointNumber >= this->getPotentialVector()->size() || pointNumber >= this->getTimeVector()->size() ) {
        throw 1;
	}
	if ( (pointNumber!= -1) && (pointNumber < this->_nrOfPoints) ) {
		this->_vCurrent[pointNumber]   = current;
	} else {
		this->_vCurrent[this->_nrOfPoints]   = current;
		this->_nrOfPoints++;
	}
}

void CurveData::addProbingDataPoint(float fValue)
{
	this->_vProbingData[this->_nrOfPointsProbing] = fValue;
	this->_nrOfPointsProbing++;
}

QVector<double>* CurveData::getProbingData()
{
    return &this->_vProbingData;
}

QVector<double>* CurveData::getProbingDataPointNumbers()
{
    return &this->_vProbingDataPointNo;
}

void CurveData::setProbingData(QVector<double>& probingData)
{
    _vProbingData = probingData;
    _nrOfPointsProbing = _vProbingData.size();
    _vProbingDataPointNo.clear();
    _vProbingDataPointNo.resize(_nrOfPointsProbing);
    TYPES::vectorindex_t i = 0;
    for ( double d = 1; d<=_nrOfPointsProbing; ++d ) {
        this->_vProbingDataPointNo[i] = d;
        ++i;
    }
}

int32_t CurveData::getNumberOfProbingPoints()
{
	return this->_nrOfPointsProbing;
}

int CurveData::getNrOfPoints()
{
    return this->_nrOfPoints;
}

QVector<double>* CurveData::getPotentialVector()
{
    return &this->_vPotential;
}

void CurveData::setPotentialVector(QVector<double> vecPot)
{
	this->_vPotential = vecPot;
}

double CurveData::getPotential(TYPES::vectorindex_t index)
{
	return this->_vPotential[index];
}

QVector<double>* CurveData::getCurrentVector()
{
    return &this->_vCurrent;
}

double CurveData::getCurrent(TYPES::vectorindex_t index)
{
	return this->_vCurrent[index];
}

void CurveData::setCurrent(TYPES::vectorindex_t index, double value)
{
	this->_vCurrent[index] = value;
}

QVector<double>* CurveData::getTimeVector()
{
    return &this->_vTime;
}

void CurveData::setTimeVector(QVector<double> vecTime)
{
	this->_vTime = vecTime;
}


double CurveData::getTime(TYPES::vectorindex_t index)
{
	return this->_vTime[index];
}

void CurveData::setMesCurrent1Point(TYPES::vectorindex_t index, int64_t value)
{
    this->mesResCurrent1[index] = value;
}

void CurveData::addToMesCurrent1Point(TYPES::vectorindex_t index, int64_t value)
{
    this->mesResCurrent1[index] += value;
}

void CurveData::setMesCurrent2Point(TYPES::vectorindex_t index, int64_t value)
{
    this->mesResCurrent2[index] = value;
}

void CurveData::addToMesCurrent2Point(TYPES::vectorindex_t index, int64_t value)
{
    this->mesResCurrent2[index] += value;
}

void CurveData::setMesTimePoint(TYPES::vectorindex_t index, int64_t value)
{
    this->mesResTime[index] = value;
}

void CurveData::addToMesTimePoint(TYPES::vectorindex_t index, int64_t value)
{
    this->mesResTime[index] += value;
}

int64_t CurveData::getMesTimePoint(TYPES::vectorindex_t index)
{
    return this->mesResTime[index];
}

int64_t CurveData::getMesCurrent1Point(TYPES::vectorindex_t index)
{
    return this->mesResCurrent1[index];
}

int64_t CurveData::getMesCurrent2Point(TYPES::vectorindex_t index)
{
    return this->mesResCurrent2[index];
}

void CurveData::allocateMesArray(TYPES::vectorindex_t numberOfFields, bool allocateTwoCurrent)
{
	this->isMesAllocated = true;
	this->allocatedTwoCurrent = allocateTwoCurrent;
    this->mesResCurrent1 = new int64_t[numberOfFields];
	for ( int i=0; i<numberOfFields; i++) {
        this->mesResCurrent1[i] = 0;
	}

    this->mesResTime = new int64_t[numberOfFields];
	for ( int i=0; i<numberOfFields; i++) {
        this->mesResTime[i] = 0;
	}

	if ( allocateTwoCurrent ) {
        this->mesResCurrent2 = new int64_t[numberOfFields];
		for ( int i=0; i<numberOfFields; i++) {
            this->mesResCurrent2[i] = 0;
		}
	}

}
