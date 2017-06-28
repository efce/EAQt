
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
#ifndef _C_CURVE_H_
#define _C_CURVE_H_ 1

#include <QVector>
#include "./Qcustomplot/qcustomplot.h"
#include "const.h"
#include "curvedata.h"

class Curve
{
public:
	Curve(Curve*);
    Curve(int32_t nLen);
    Curve(int32_t nLen, int32_t nLenProbing, double dProbingRate);

	~Curve();

    QByteArray serialize(bool compress);
    bool unserialize(QByteArray &ba, bool compressed);

    void Param(int32_t nParamNr, int32_t lValue);
    int32_t Param(int32_t nParamNr);
    void Result(int32_t nResultNr, double dResult);
    double Result(int32_t nResultNr);
    void addDataPoint(double time, double potential, double current, int32_t pointNumber = -1);
    void addDataPoint(double current, int32_t pointNumber = -1);
    int32_t getNrOfDataPoints();
    void reinitializeCurveData(int32_t); //this destroys already loaded data !!!
    QVector<double>* getCurrentVector();
	bool wasModified();
	void wasModified(bool);
	int lastDisplayed();
	void lastDisplayed(int);
    QVector<double>* getPotentialVector();
    void setDate();

    void setPotentialVector(QVector<double>);
    void setTimeVector(QVector<double>);

    double getPotentialPoint(int32_t);
    QVector<double>* getTimeVector();
    double getTimePoint(int32_t);
    int64_t getMesTimePoint(int32_t index);
    int64_t getMesCurrent1Point(int32_t index);
    int64_t getMesCurrent2Point(int32_t index);

    void addToMesTimePoint(int32_t index, int64_t v);
    void addToMesCurrent1Point(int32_t index, int64_t v);
    void addToMesCurrent2Point(int32_t index, int64_t v);

    void allocateMesArray(int32_t numOfFields, bool twoCurrent);
    void allocateMesArray();
	void allocateProbingData();
    void allocateProbingData(int32_t newNumber);
    void addProbingDataPoint(float fValue);
    int32_t getNumberOfProbingPoints();
    QVector<double>* getProbingData();
    QVector<double>* getProbingDataPointNumbers();
    void setProbingData(QVector<double>);

    QVector<double> getXVector();
    QVector<double> getYVector();
    void setYValue(int32_t, double);

    QCPGraph* getPlot();
    void setPlot(QCPGraph *_plot);
	bool hasPlot();
    void changeToMesPlot(int32_t electroNum = 0);
	void changeToRegularPlot();
	void clearPlot();

    void FName(QString lszText);
    QString FName();
    void CName(QString lszText);
    QString CName();
    void Comment(QString lszText);
    QString Comment();

    QString getListName();

private:
    void Initialize(int nResLen);

    CurveData *_curveData;
    QString _fileName;	   // File name where it is saved
    QString _curveName;		// Curve name in file
    int32_t _mesParam[PARAM::PARAMNUM];		// Parameters of the measurement
    QString _comment;		// Comments of the curve (usually sample informations)
    QCPGraph* _plot;				// pointer to the graph structure in the EAQtMainWindow::_plotMain
    bool _hasPlot;
    int _lastDisplayed;
    bool _wasModified;
};

#endif
