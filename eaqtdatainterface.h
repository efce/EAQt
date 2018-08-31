
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
    virtual void ProcessPacketFromEA(const char* packet) = 0;
    virtual void MesPrepareUIUpdate() = 0;
    virtual void NetworkError(QString) = 0;
    virtual int32_t ParamPV(int32_t) = 0;
    virtual void ParamPV(int32_t, int32_t) = 0;
    virtual int32_t ParamLSV(int32_t) = 0;
    virtual void ParamLSV(int32_t, int32_t) = 0;
    virtual void MesStart(bool isLSV) = 0;
    virtual int32_t getDropsBeforeMes() = 0;
    virtual void setDropsBeforeMes(int32_t) = 0;
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
    virtual void setChannelName(uint channelNum, QString name) = 0;
    virtual void setChannelEnabled(uint channelNum, bool enabled) = 0;
    virtual bool getUseMesFile() = 0;
    virtual void setUseMesFile(bool) = 0;
    virtual QString getMesSeriesFile() = 0;
    virtual void setMesSeriesFile(QString) = 0;
    virtual EAQtSignalProcessing* getProcessing() = 0;
    virtual void undoPrepare() = 0;
};

#endif // EAQTDATAINTERFACE_H
