
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
#ifndef EAQTDATA_H
#define EAQTDATA_H

#include <QtWidgets>
#include "Qcustomplot/qcustomplot.h"
#include "eaqtdatainterface.h"
#include "eaqtuiinterface.h"
#include "eaqtnetwork.h"
#include "mdir.h"
#include "mdircollection.h"
#include "curve.h"
#include "curvecollection.h"
#include "mescfg.h"
#include "eaqtsignalprocessing.h"
#include "calibrationdata.h"

/*
 * EAQtData is a singleton, which holds all data and data operations,
 * it was devided into subclasses, to make it more readable,
 * However, it is still very large class.
 */
class EAQtData : public QObject, public EAQtDataInterface
{
    Q_OBJECT
public:
    static EAQtData& getInstance()
    {
        static EAQtData instance; // Guaranteed to be destroyed.
        return instance;
    }
    void initialUpdate(EAQtUIInterface *wp);
    ~EAQtData();

/*-------------
 * FUNCTIONS :
 -------------*/
private:
    EAQtData();
    EAQtData(EAQtData const& copy);       // Don't Implement
    EAQtData& operator=(EAQtData const& copy); // Don't implement

    void setCurrentRange(int, int b = 0);
    int getCurrentRange();
    void initParam();
    void initParamLSV();
    void initEca();
    void initPtime();
    void createMatrix();
    void crmxSQW();

    void prepareParamForSQWClassic(Curve*);
    void recoverParamForSQWClassic(Curve*);
    struct SqwClassicTemp {
        bool isSQWClassic = false;
        int32_t method = 0;
        int32_t Ep = 0;
        int32_t Ek = 0;
        int32_t dE = 0;
    } _sqwClassicTemp;

    void loadMesFile();
    void seriaWait(int32_t delay_secs);

    bool sendLSVToEA();
    bool sendPVToEA();

public:
    void openFile(QString *filePath, int nrPos);
    // END FILE operations
    CurveCollection* getCurves();
    CurveCollection* getMesCurves();
    EAQtSignalProcessing* getProcessing();

    void deleteActiveCurveFromGraph();
    void deleteNonactiveCurvesFromGraph();
    void deleteAllCurvesFromGraph();

    void undoPrepare();
    void undoExecute();

    QString dispI(double); //display formated current with units
    QString dispIforTXT(double); //display formated current w/o units
    QString dispE(int); // display formated potential with units
    QString dispE(double); //display formated potential with units
    QString dispEforTXT(double); //display formated potential w/o units
    QString dispNR(int); // display number
    QString dispMATH(double); // display number
    QString dispTIME(int); // display time e.g 01,13,06
    QString dispTIME1(int); // display time in sec, argument in ms
    QString dispTIME1(double);
    QString dispTIMEforTXT(double);

    void Act(TYPES::vectorindex_t nAct);
    TYPES::vectorindex_t Act();
    int getXAxis();
    void setXAxis(int);
    void ParamPV(int32_t, int32_t);
    int32_t ParamPV(int32_t);
    void ParamLSV(int32_t, int32_t);
    int32_t ParamLSV(int32_t);
    void ParamMain(int32_t, int32_t);
    int32_t ParamMain(int32_t);

    int32_t isGasOn();
    void setGas(int32_t);
    int32_t isMixerOn();
    void setMixer(int32_t);
    void setTestHammer(bool);
    void setTestCGMDEValve(bool);
    int32_t getCGMDETestNum();

    int CurReadCurvePro(QFile &ff, QString CName);
    bool MDirReadPro(QFile &ff);
    void CurReadFilePro(QString *FileName, int PosNr);
    int CurReadCurveOld(QFile &ff, QString CName);
    void CurImportTxtFile(QString *FileName);
    void ParamReadOld(QFile &ff);
    bool MDirReadOld(QFile &ff);
    void CurReadFileOld(QString *FileName, int PosNr);
    int safeAppend(QString pFileName, Curve* CurveToAppend);
    void exportToTXT(QString path);
    void exportToCSV(QString path);
    std::vector<std::string> exportToCSVCurve(Curve* c);
    std::vector<std::string> exportToTXTCurve(Curve* c);

    void MesStart(bool isLsv = false);
    void MesStop();
    void MesClear();
    void MesAfter();
    void MesUpdate(int32_t, int32_t, bool);
    int  MesSaveAll(QString UserCName, QString UserFName, QString UserComment);
    bool isMeasurement();
    const QVector<int16_t>& getPotentialProgram();
    void setPotentialProgram(QVector<int16_t> pp);

    void ProcessPacketFromEA(char* packet, bool nextPacketReady);
    void sendAccessories();
    bool sendTestCGMDE();
    void sendTestCGMDEStop();
    void NetworkError(QString);
    void updateELSV();
    void updatePause();
    void updateTestCGMDE();
    double CountResultPV(int64_t value);
    double CountResultLSV(int64_t value);
    MDirCollection* getMDir();

    bool getWasLSV();

    QVector<QString> getChannelsNames();
    std::vector<bool> getChannelsEnabled();
    void setChannelsNames(QVector<QString>);
    void setChannelsEnabled(std::vector<bool>);
    bool getUseMesFile();
    void setUseMesFile(bool);
    QString getMesSeriesFile();
    void setMesSeriesFile(QString);

    int32_t _ptnrFromEss;

   struct TestCGMDE {
       int32_t GP;				// test CGMDE - czas otwarcia zaworu
       int32_t BT;				// test CGMDE - czas przerwy
       int32_t ElPot;			// test CGMDE - potecjał elektrody podczas testu
       int32_t ChPot;			// test CGMDE - inf. czy przykładać potencjał
   } _testCGMDE;

   CalibrationData *_calibration;

/* ------------
 * VARIABLES
 ------------*/
private:
   EAQtUIInterface *_pUI;
   EAQtNetwork *_network;
   CurveCollection *_curves;
   CurveCollection *_mesCurves;
   MDirCollection *_fileIndex;
   EAQtSignalProcessing *_processing;
   QTime _fromUpdate;

   struct UndoStruct {
       bool undoReady = false;
       CurveCollection *_curves = NULL;
       int32_t _act;
       int32_t _PVParam[PARAM::PARAMNUM];
       int32_t _LSVParam[PARAM::PARAMNUM];
   } undoStruct;

   unsigned char _TxBuf[NETWORK::TxBufLength];
   unsigned char _RxBuf[NETWORK::RxBufLength];
   TYPES::vectorindex_t _act;
   int32_t _mainParam[PARAM::PARAMNUM];	   // parametry pomiaru czytanej/zapisywanej krzywej
   int32_t _PVParam[PARAM::PARAMNUM];
   int32_t _LSVParam[PARAM::PARAMNUM];
   QVector<int16_t> _PVParam_PotentialProgram {};

   int _currentRange;
   bool _prepareEstart;
   int _calculatedNrOfPoints;
   int _displayCurveNr;
   int _currentPointOnCurve;

   QVector<QString> _vChannelNamesOfMultielectrode {};

   int16_t _IUE0;          // calibration result: current in uA for E = 0 V //
   int32_t _endOfMes;       // measurmenet ended //
   int32_t _stopInfo;		// measurement was stopped //

   int32_t _EstartInfo;	   // LSV:
                           // 0 - ignore start potential;
                           // 1 - start potential within measurement range;
                           // 2 - start potential outside of measurment range;

   struct BreakInfo {
       int32_t currentSec;
       int32_t currentMin;
       int32_t currentE;
       int32_t targetMin;
       int32_t targetSec;
   } _break;

   int32_t _measurementMatrix[20], ainmat; // macierz pomiaru, wskaźnik w macierzy

   int32_t E1I, E2I, E3I; // calculated potential of first three potential steps //

   int32_t _samplingTime;   // sampling time in [ms]
   int64_t workl;    // current result from A/C
   int32_t _measurementGo;
   int32_t currentPointNr;
   int _xaxis_type;

   struct AnalyzerAccessories {
       int32_t gas_on;
       int32_t stirrer_on;
       int32_t drops_bm;
       int32_t test_knock;
       int32_t test_valve;
   } _accessories;

   int32_t _CGMDETestNr;		// _CGMDETestNr - counter for number valve openings (CGMDE)
   int64_t _EstartCurrentTime;	// time of start potential applied in LSV multicyclic
   int32_t _singleStepTime;		// time in ms of one point
   int32_t _ctnrSQW;    // Counter for Oysteryoung SQW measuremnet
   int32_t _trValue;    // Donno

   int32_t _wasLSVMeasurement;	// informacja, które parametry pomiaru były ostatnio przetwarzane
                                // 0 - techniki impulsowe, 1 - LSV
   int32_t _conductingMeasurement;
   bool _performSetup;
   int32_t _multielectrodeNr;
   std::vector<double> mdata;		// do interpretacji

    //SERIA:
    bool _isMesSeries;
    bool _useSeriesFile;
    QString _seriesFilePath;
    MesCFG *_pSeriesData;
    // END SERIA
signals:
    void actChanged(int act);
    void undoPrepared(bool);
    void undoExecuted(bool);

};
#endif // EAQTDATA_H
