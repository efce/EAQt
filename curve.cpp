
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
#include <QVector>
#include "curve.h"
#include "curvedata.h"
#include "eaqtdata.h"

Curve::Curve(int nLen)
{
    Initialize(nLen);
}

Curve::Curve(int nLen, uint32_t nLenProbing, double dProbingRate)
{
    Initialize(nLen);
	this->_curveData->allocateProbingData(nLenProbing, dProbingRate);
}

Curve::Curve(Curve* toCopy)
{
	this->CName(toCopy->CName());
	this->Comment(toCopy->Comment());
	this->FName(toCopy->FName());
	this->_hasPlot = false;

    for (int32_t i=0; i<PARAM::PARAMNUM; ++i) {
		this->_mesParam[i] = toCopy->_mesParam[i];
	}
    this->_curveData = new CurveData(this->_mesParam[PARAM::ptnr]);

    this->setPotentialVector(QVector<double>(*toCopy->getPotentialVector()));

    this->setTimeVector(QVector<double>(*toCopy->getTimeVector()));

    if ( toCopy->Param(PARAM::nonaveragedsampling) != 0 ) {
        this->setProbingData(QVector<double>(*toCopy->getProbingData()));
	}

    uint32_t len = toCopy->getCurrentVector()->size();
    for ( uint32_t i=0; i<len; ++i) {
        this->addDataPoint(toCopy->getCurrentVector()->at(i), i);
	}
}

Curve::~Curve()
{
	delete this->_curveData;
}

void Curve::Initialize(int nResLen)
{
	_wasModified = true;
    _lastDisplayed = XAXIS::potential;

    _fileName.clear();
    _curveName.clear();
    _comment.clear();
	_hasPlot = false;
    this->_curveData = new CurveData((uint32_t) nResLen);
    for (int i=0; i<PARAM::PARAMNUM; i++ ) {
		this->_mesParam[i] = 0;
	}
}

void Curve::reinitializeCurveData(uint32_t newSize)
{
	delete this->_curveData;
	this->_curveData = new CurveData(newSize);
}

void Curve::Param(int nParamNr, int lValue)
{
	_mesParam[nParamNr]= lValue;
}

int32_t Curve::Param(int32_t nParamNr)
{
	return _mesParam[nParamNr];
}

void Curve::clearPlot()
{
	if ( this->hasPlot() ) {
        this->getPlot()->data()->clear();
	}
}

void Curve::addDataPoint(double time, double potential, double current, int pointNumber)
{
	_wasModified = true;
	this->_curveData->addDataPoint(time, potential, current, pointNumber);
}

void Curve::addDataPoint(double current, int pointNumber)
{
	_wasModified = true;
	this->_curveData->addDataPoint(current, pointNumber);
}

QVector<double>* Curve::getCurrentVector()
{
    return this->_curveData->getCurrentVector();
}

QVector<double>* Curve::getPotentialVector()
{
    return this->_curveData->getPotentialVector();
}

void Curve::setPotentialVector(QVector<double> vecPot)
{
	_wasModified = true;
	this->_curveData->setPotentialVector(vecPot);
}

double Curve::getPotentialPoint(int index)
{
	return this->_curveData->getPotential(index);
}

QVector<double>* Curve::getTimeVector()
{
    return this->_curveData->getTimeVector();
}

void Curve::setTimeVector(QVector<double> vecTime)
{
	_wasModified = true;
	this->_curveData->setTimeVector(vecTime);
}

double Curve::getTimePoint(int index)
{
	return this->_curveData->getTime(index);
}

double Curve::Result(int nResultNr)
{
	return this->_curveData->getCurrent(nResultNr);
}

void Curve::Result(int nResultNr, double dResult)
{
	_wasModified = true;
	this->_curveData->setCurrent(nResultNr, dResult);
}

QCPGraph* Curve::getPlot()
{
	return this->_plot;
}

void Curve::setPlot(QCPGraph* plot)
{
	this->_plot = plot;
	this->_hasPlot = true;
}

bool Curve::hasPlot()
{
	return this->_hasPlot;
}


void Curve::FName(QString lszText)
{
	_fileName = lszText;
}

QString Curve::FName()
{
	return _fileName;
}

void Curve::CName(QString lszText)
{
	_curveName = lszText;
}

QString Curve::CName()
{
	return _curveName;
}

void Curve::Comment(QString lszText)
{
	_comment = lszText;
}

QString Curve::Comment()
{
	return _comment;
}

void Curve::allocateMesArray(int numOfFields, bool twoCurrent)
{
	this->_curveData->allocateMesArray(numOfFields, twoCurrent);
}

void Curve::allocateMesArray()
{
    if ( this->Param(PARAM::method) == PARAM::method_lsv ) {
        this->allocateMesArray( (2*MEASUREMENT::LSVtime[this->Param(PARAM::dEdt)]* this->Param(PARAM::ptnr)), false);
    } else {
        this->allocateMesArray( (2*this->Param(PARAM::tp)* this->Param(PARAM::ptnr)), true);
    }
}

void Curve::allocateProbingData()
{
    uint32_t nrofsamples;
    if (this->Param(PARAM::sampl) == PARAM::sampl_single ) { //pojedyncze
        nrofsamples = (this->Param(PARAM::tp)+this->Param(PARAM::tw))*this->Param(PARAM::ptnr);
	} else { // podwojne
        nrofsamples = (this->Param(PARAM::tp)+this->Param(PARAM::tw))*this->Param(PARAM::ptnr)*2;
	}
    this->_curveData->allocateProbingData(nrofsamples * this->Param(PARAM::aver), this->Param(PARAM::nonaveragedsampling));
}

void Curve::allocateProbingData(uint32_t newNumber)
{
    this->_curveData->allocateProbingData(newNumber, this->Param(PARAM::nonaveragedsampling));
}

void Curve::addProbingDataPoint(float fValue)
{
	_wasModified = true;
	this->_curveData->addProbingDataPoint(fValue);
}

void Curve::setProbingData(QVector<double> data)
{
	_wasModified = true;
	this->_curveData->setProbingData(data);
}

uint32_t Curve::getNumberOfProbingPoints()
{
	return this->_curveData->getNumberOfProbingPoints();
}

QVector<double>* Curve::getProbingData()
{
	return this->_curveData->getProbingData();
}

QVector<double>* Curve::getProbingDataPointNumbers()
{
    return this->_curveData->getProbingDataPointNumbers();
}

int64_t* Curve::getMesTimePoint(int index)
{
	return this->_curveData->getMesTimePoint(index);
}

int64_t* Curve::getMesCurrent1Point(int index)
{
	return this->_curveData->getMesCurrent1Point(index);
}

int64_t* Curve::getMesCurrent2Point(int index)
{
	return this->_curveData->getMesCurrent2Point(index);
}

uint32_t Curve::getNrOfDataPoints()
{
	return this->_curveData->getNrOfPoints();
}

void Curve::changeToMesPlot(int electroNum/*=0*/)
{
	if ( this->hasPlot() ) {
        QPen qp = this->getPlot()->pen();
		switch ( electroNum ) {
			case 0:
                qp.setColor(COLOR::measurement);
				break;
			case 1:
                qp.setColor(QColor( 0, 100, 0 ));
				break;
			case 2:
                qp.setColor(QColor( 100, 0, 0 ));
				break;
			case 3:
                qp.setColor(QColor( 30, 30, 100 ));
				break;
			case 4:
                qp.setColor(QColor( 100, 100, 100 ));
				break;
			case 5:
                qp.setColor(QColor( 0, 100, 100 ));
				break;
			case 6:
                qp.setColor(QColor( 100, 100, 0 ));
				break;
			case 7:
                qp.setColor(QColor( 100, 0, 100 ));
				break;
			default:
                qp.setColor(COLOR::measurement);
				break;
		}
        this->getPlot()->setPen(qp);
    } else {
        throw 1;
	}
}

void Curve::changeToRegularPlot()
{
	if ( this->hasPlot() ) {
        QPen qp = this->getPlot()->pen();
        qp.setColor(COLOR::regular);
        this->getPlot()->setPen(qp);
    } else {
        throw 1;
	}
}

void Curve::wasModified(bool md)
{
	this->_wasModified = md;
}

bool Curve::wasModified()
{
	return this->_wasModified;
}

void Curve::lastDisplayed(int ld)
{
	this->_lastDisplayed = ld;
}

int Curve::lastDisplayed()
{
	return this->_lastDisplayed;
}

QString Curve::getListName()
{
    return this->FName().append(": " + this->CName());
}

void Curve::setDate()
{
    Param(PARAM::date_day,QDate::currentDate().day());
    Param(PARAM::date_month,QDate::currentDate().month());
    Param(PARAM::date_year,QDate::currentDate().year());
    Param(PARAM::date_hour,QTime::currentTime().hour());
    Param(PARAM::date_minutes,QTime::currentTime().minute());
    Param(PARAM::date_seconds,QTime::currentTime().second());
}

QVector<double> Curve::getXVector()
{
    switch ( EAQtData::getInstance().getXAxis() ) {
    default:
    case XAXIS::potential:
        return this->getPotentialVector()->mid(0,getNrOfDataPoints());
    case XAXIS::time:
        return this->getTimeVector()->mid(0,getNrOfDataPoints());
    case XAXIS::nonaveraged:
        return this->getProbingDataPointNumbers()->mid(0,getNumberOfProbingPoints());
    }
}

QVector<double> Curve::getYVector()
{
    switch ( EAQtData::getInstance().getXAxis() ) {
    default:
    case XAXIS::potential:
    case XAXIS::time:
        return this->getCurrentVector()->mid(0,getNrOfDataPoints());
    case XAXIS::nonaveraged:
        return this->getProbingData()->mid(0,this->getNumberOfProbingPoints());
    }
}

void Curve::setYValue(uint32_t index, double value)
{
    switch ( EAQtData::getInstance().getXAxis() ) {
    default:
    case XAXIS::potential:
    case XAXIS::time:
        this->Result(index,value);
        return;
    case XAXIS::nonaveraged:
        this->getProbingData()->replace(index, value);
        return;
    }
}
