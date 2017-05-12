
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
#ifndef EAQTDATAINTERFACE_H
#define EAQTDATAINTERFACE_H

#include "curvecollection.h"
#include "eaqtsignalprocessing.h"

class EAQtDataInterface {
public:
    virtual void ProcessPacketFromEA(char*, bool) = 0;
    virtual void NetworkError(QString) = 0;
    virtual int32_t ParamPV(int32_t) = 0;
    virtual void ParamPV(int32_t, int32_t) = 0;
    virtual int32_t ParamLSV(int32_t) = 0;
    virtual void ParamLSV(int32_t, int32_t) = 0;
    virtual void MesStart(bool isLSV) = 0;
    virtual int32_t isGasOn() = 0;
    virtual void setGas(int32_t) = 0;
    virtual int32_t isMixerOn() = 0;
    virtual void setMixer(int32_t) = 0;
    virtual void sendAccessories() = 0;
    virtual void setTestHammer(bool) = 0;
    virtual void setTestCGMDEValve(bool) = 0;
    virtual int getXAxis() = 0;
    virtual void setXAxis(int) = 0;
    virtual int Act() = 0;
    virtual CurveCollection *getCurves() = 0;
    virtual CurveCollection *getMesCurves() = 0;
    virtual QString dispI(double) = 0; //display formated current with units
    virtual QString dispE(int) = 0; // display formated potential with units
    virtual QString dispE(double) = 0; //display formated potential with units
    virtual QString dispNR(int) = 0; // display number
    virtual QString dispMATH(double) = 0; // display number
    virtual QString dispTIME(int) = 0; // display time e.g 01,13,06
    virtual QString dispTIME1(int) = 0; // display time in sec, argument in ms
    virtual QString dispTIME1(double) = 0;
    virtual QVector<QString> getChannelsNames() = 0;
    virtual std::vector<bool> getChannelsEnabled() = 0;
    virtual void setChannelsNames(QVector<QString>) = 0;
    virtual void setChannelsEnabled(std::vector<bool>) = 0;
    virtual bool getIsMesSeries() = 0;
    virtual void setIsMesSeries(bool) = 0;
    virtual QString getMesSeriesFile() = 0;
    virtual void setMesSeriesFile(QString) = 0;
    virtual EAQtSignalProcessing* getProcessing() = 0;

    struct CalibrationData {
        bool wasFitted;
        double slope;
        double slopeStdDev;
        double intercept;
        double interceptStdDev;
        double x0StdDev;
        double correlationCoef;
        QVector<double> xvalues;
        QString xUnits;
        QVector<double> yvalues;
        QString yUnits;
        CurveCollection *curves;
    } _calibration;

    static void saveCalibration(QFile *file, const CalibrationData& cd, const bool& includeCurves)
    {
        char wrt;
        int n, l;
        wrt = (char)includeCurves;
        file->write(&wrt,1);
        n = cd.yvalues.size();
        file->write((char*)&n,sizeof(int));
        file->write((char*)cd.xvalues.data(),sizeof(double)*n);
        l = cd.xUnits.toStdString().length();
        file->write((char*)&l,sizeof(int));
        file->write(cd.xUnits.toLatin1());
        file->write((char*)cd.yvalues.data(),sizeof(double)*n);
        l = cd.yUnits.toLatin1().size();
        file->write((char*)&l,sizeof(int));
        file->write(cd.xUnits.toLatin1());
        wrt = (char)cd.wasFitted;
        file->write((char*)&wrt,1);
        file->write((char*)&(cd.slope),sizeof(double));
        file->write((char*)&(cd.slopeStdDev),sizeof(double));
        file->write((char*)&(cd.intercept),sizeof(double));
        file->write((char*)&(cd.interceptStdDev),sizeof(double));
        file->write((char*)&(cd.x0StdDev),sizeof(double));
        file->write((char*)&(cd.correlationCoef),sizeof(double));
    }

    static void readCalibration(QFile *file, CalibrationData& cd, bool& curvesIncluded)
    {
        char wrt;
        int n,l;
        QByteArray ba;
        file->read((char*)&wrt,1);
        curvesIncluded = (bool)wrt;
        file->read((char*)&n,sizeof(int));
        cd.xvalues.resize(n);
        cd.yvalues.resize(n);
        file->read((char*)cd.xvalues.data(),sizeof(double)*n);
        file->read((char*)&l,sizeof(int));
        ba.resize(l);
        file->read(ba.data(),l);
        cd.xUnits.fromLatin1(ba);
        file->read((char*)cd.yvalues.data(),sizeof(double)*n);
        file->read((char*)&l,sizeof(int));
        ba.resize(l);
        file->read(ba.data(),l);
        cd.yUnits.fromLatin1(ba);
        file->read((char*)&wrt,1);
        cd.wasFitted = (bool)wrt;
        file->read((char*)&(cd.slope),sizeof(double));
        file->read((char*)&(cd.slopeStdDev),sizeof(double));
        file->read((char*)&(cd.intercept),sizeof(double));
        file->read((char*)&(cd.interceptStdDev),sizeof(double));
        file->read((char*)&(cd.x0StdDev),sizeof(double));
        file->read((char*)&(cd.correlationCoef),sizeof(double));
    }

};

#endif // EAQTDATAINTERFACE_H
