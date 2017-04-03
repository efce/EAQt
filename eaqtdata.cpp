
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
#include "eaqtdata.h"
#include "eaqtsavefiledialog.h"
#include "eaqtcalibrationdialog.h"

EAQtData::EAQtData() : QObject(), EAQtDataInterface()
{
    memset(_TxBuf, 0, NETWORK::TxBufLength*sizeof(char));
    memset(_RxBuf, 0, NETWORK::RxBufLength*sizeof(char));

    _act = SELECT::none;
    _wasLSVMeasurement = 0;
    _conductingMeasurement = 0;
    _performSetup = true;
    _multielectrodeNr = 0;
    _isMesSeries = false;
    _useSeriesFile = false;

    _vChannelNamesOfMultielectrode.resize(8);
    for ( int a=0; a<_vChannelNamesOfMultielectrode.size(); a++) {
        _vChannelNamesOfMultielectrode[a] = tr("%1").arg(a+1);
    }

    _accessories.gas_on = 0;
    _accessories.stirrer_on = 0;
    _accessories.drops_bm = 0;
    _accessories.test_knock = 0;
    _accessories.test_valve = 0;

    _break.currentE=0;
    _break.currentMin=0;
    _break.currentSec=0;
    _break.targetSec=0;
    _break.targetMin=0;

    _ptnrFromEss = 0;
    _prepareEstart = true;
    _currentRange = PARAM::crange_macro_1uA;
    _xaxis_type = XAXIS::potential;

    this->_calculatedNrOfPoints = 0;
    this->_displayCurveNr = 0;
    this->_currentPointOnCurve = 0;

    InitParam();
    InitParamLSV();
}

void EAQtData::InitialUpdate(EAQtUIInterface *wp)
{
    _pUI = wp;
    _curves = new CurveCollection(wp);
    _mesCurves = new CurveCollection(wp);
    _fileIndex = new MDirCollection();
    _network = new EAQtNetwork(this);
    _processing = new EAQtSignalProcessing(_curves, wp->PlotAddGraph());
}

EAQtData::~EAQtData()
{
    delete _fileIndex;
    delete _processing;
    delete _curves;
    delete _mesCurves;
    delete _network;
}

void EAQtData::InitParam()
{
    // Parametry pomiaru
    for (int j=0 ; j<PARAM::PARAMNUM ; j++)
        _PVParam[j] = 0;

    _PVParam[PARAM::aver] = 1;
    _PVParam[PARAM::Ep] = -100;
    _PVParam[PARAM::Ek] = 100;
    _PVParam[PARAM::Estep] = 1;
    _PVParam[PARAM::tp] = 100;
    _PVParam[PARAM::tw] = 100;
    _PVParam[PARAM::tk] = 500;
    _PVParam[PARAM::ptnr] = 200;
    _PVParam[PARAM::pro] = 0;
    _PVParam[PARAM::multi] = 0;
    _PVParam[PARAM::method] = PARAM::method_scv;
    _PVParam[PARAM::electr] = PARAM::electr_solid;
    _PVParam[PARAM::el23] = PARAM::el23_dummy;
    _PVParam[PARAM::mespv] = PARAM::mespv_voltamperometry;
    _PVParam[PARAM::sampl] = PARAM::sampl_single;
    _PVParam[PARAM::crange] = PARAM::crange_macro_10uA;
    _PVParam[PARAM::gtype] = 0;
    _PVParam[PARAM::impnr] = 5;
    _PVParam[PARAM::imptime] = 10;
    _PVParam[PARAM::inttime] = 200;
    _PVParam[PARAM::nonaveragedsampling] = 0;

    // Akcesoria
    _PVParam[PARAM::sti] = 20;
    _PVParam[PARAM::kp] = 10;
    _PVParam[PARAM::kpt] = 30;
}

void EAQtData::InitParamLSV()
{
    // Parametry pomiaru
    for (int j=0 ; j<=61 ; j++)
        _LSVParam[j] = 0;

    _LSVParam[PARAM::aver] = 1;
    _LSVParam[PARAM::Ep] = -100;
    _LSVParam[PARAM::Ek] = 100;
    //	m_alLsvParam[method] = 4;
    _LSVParam[PARAM::electr] = PARAM::electr_solid;
    _LSVParam[PARAM::el23] = PARAM::el23_dummy;
    _LSVParam[PARAM::crange] = PARAM::crange_macro_10uA;
    _LSVParam[PARAM::impnr] = 5;
    _LSVParam[PARAM::imptime] = 10;
    _LSVParam[PARAM::inttime] = 200;
    _LSVParam[PARAM::dEdt] = 3;
    _LSVParam[PARAM::EstartLSV] = 0;
    _LSVParam[PARAM::Escheck] = 0;
    _LSVParam[PARAM::ts] = 0;
    _LSVParam[PARAM::td] = 500;

    // Akcesoria
    _LSVParam[PARAM::sti] = 20;
    _LSVParam[PARAM::kp] = 10;
    _LSVParam[PARAM::kpt] = 30;
}

void EAQtData::InitEca()
{
    int32_t work;

    //E2I := Ep    NP
    //       Ep+dE DP
    E2I = this->getMesCurves()->get(0)->Param(PARAM::Ep);
    if (this->getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_dpv ) // DP
        E2I += this->getMesCurves()->get(0)->Param(PARAM::dE);
    if (this->getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_sqw ) // SQW
    {
        E3I = E2I - this->getMesCurves()->get(0)->Param(PARAM::dE);
        E2I += this->getMesCurves()->get(0)->Param(PARAM::dE);
    }

    work = this->getMesCurves()->get(0)->Param(PARAM::Ep);
    if (this->getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_npv)
        work = this->getMesCurves()->get(0)->Param(PARAM::E0); // NP
    E1I = work;
}

void EAQtData::InitPtime()
{
    int64_t work;
    //Count tr = tk-i*(tw+tp)  i=1,2 (sampling)
    work = this->getMesCurves()->get(0)->Param(PARAM::tw) + this->getMesCurves()->get(0)->Param(PARAM::tp);
    if (this->getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_sqw ) // SQW
        _trValue = this->getMesCurves()->get(0)->Param(PARAM::tk) - 5*work;
    else
        _trValue = this->getMesCurves()->get(0)->Param(PARAM::tk) - work;

    if ( this->getMesCurves()->get(0)->Param(PARAM::sampl) == PARAM::sampl_double )
        _trValue -= work;

    if (this->getMesCurves()->get(0)->Param(PARAM::mespv) == PARAM::mespv_polarography ) {
        _singleStepTime = _trValue; // polar.
    } else {
        if ( ! (((this->getMesCurves()->get(0)->Param(PARAM::electr) == PARAM::electr_cgmde )  // CGMDE
                 || (this->getMesCurves()->get(0)->Param(PARAM::electr) == PARAM::electr_microCgmde )) &&  // mikroelektroda CGMDE
                (this->getMesCurves()->get(0)->Param(PARAM::gtype) >= 2)) ) {
            _singleStepTime = this->getMesCurves()->get(0)->Param(PARAM::td); // woltamper.
        }
    }

    if (_singleStepTime == 0)
        _singleStepTime = this->getMesCurves()->get(0)->Param(PARAM::tw);
    if (_singleStepTime == 0)
        _singleStepTime = this->getMesCurves()->get(0)->Param(PARAM::tp);
}

void EAQtData::CreateMatrix()
{
    int32_t mxptr;
    int32_t j;
    int32_t i = 0;


    for (j=0 ; j<20 ; j++)
        _measurementMatrix[j] = -1;
    mxptr = 1;
    // if voltamper. && td <> 0
    if ( (this->getMesCurves()->get(i)->Param(PARAM::mespv) == PARAM::mespv_voltamperometry)
         &&   (this->getMesCurves()->get(i)->Param(PARAM::td) != 0) ) {
        // CGMDE and seinf == 2 - do not put PROC1
        if (!(((this->getMesCurves()->get(i)->Param(PARAM::electr) == PARAM::electr_cgmde ) ||  // CGMDE
               (this->getMesCurves()->get(i)->Param(PARAM::electr) == PARAM::electr_microCgmde )) &&  // mikroelektroda CGMDE
              (this->getMesCurves()->get(i)->Param(PARAM::gtype) >= 2))) {
            _measurementMatrix[0] = 1;
        }
    }

    _measurementMatrix[mxptr] = 0;
    mxptr++;

    //if tw=0 and (if polar. then tr=0) then ... else goto CM2
    //if wolt. and pr.poj. then ... else goto CM3
    if ((this->getMesCurves()->get(i)->Param(PARAM::tw) == 0) &&
            (((this->getMesCurves()->get(i)->Param(PARAM::mespv) == 0) && (_trValue == 0)) ||
             (this->getMesCurves()->get(i)->Param(PARAM::mespv) == 1)))
        if ((this->getMesCurves()->get(i)->Param(PARAM::mespv) == 1) && (this->getMesCurves()->get(i)->Param(PARAM::sampl) == 0))
        {
            _measurementMatrix[mxptr] = 6;  mxptr++;
            _measurementMatrix[mxptr] = 2;  mxptr++;
            _measurementMatrix[mxptr] = 8;  mxptr++;
            _measurementMatrix[mxptr] = 10; mxptr++;
        }
        else
            if (this->getMesCurves()->get(i)->Param(PARAM::sampl) == PARAM::sampl_double ) // differential sampling
            {
                _measurementMatrix[mxptr] = 6;  mxptr++;
                _measurementMatrix[mxptr] = 9;  mxptr++;
                _measurementMatrix[mxptr] = 7;  mxptr++;
                _measurementMatrix[mxptr] = 10; mxptr++;
            }
            else // single sampling
            {
                _measurementMatrix[mxptr] = 6;  mxptr++;
                _measurementMatrix[mxptr] = 10; mxptr++;
            }
    else
    {
        _measurementMatrix[mxptr] = 5; mxptr++;
        // if polar and tr<>0 then PROC3
        if ((this->getMesCurves()->get(i)->Param(PARAM::mespv) == PARAM::mespv_polarography)
                && (tr != 0)) {
            _measurementMatrix[mxptr] = 3;
            mxptr++;
        }
        // if tw<>0 then PROC2
        if (this->getMesCurves()->get(i)->Param(PARAM::tw) != 0) {
            _measurementMatrix[mxptr] = 2;
            mxptr++;
        }
        _measurementMatrix[mxptr] = 8; mxptr++;
        // if polar. and pr.poj. then PROC10
        if ((this->getMesCurves()->get(i)->Param(PARAM::mespv) == PARAM::mespv_polarography)
                && (this->getMesCurves()->get(i)->Param(PARAM::sampl) == 0)) {
            _measurementMatrix[mxptr] = 10; mxptr++;
        } else {
            _measurementMatrix[mxptr] = 9; mxptr++;
            // if pr.podw. and tw<>0 then ... else CM8
            if ((this->getMesCurves()->get(i)->Param(PARAM::sampl) == PARAM::sampl_double )
                    && (this->getMesCurves()->get(i)->Param(PARAM::tw) != 0)) {
                _measurementMatrix[mxptr] = 4;  mxptr++;
                _measurementMatrix[mxptr] = 2;  mxptr++;
                _measurementMatrix[mxptr] = 8;  mxptr++;
                _measurementMatrix[mxptr] = 10; mxptr++;
            } else {
                // if polar. then ... else goto CM9
                if (this->getMesCurves()->get(i)->Param(PARAM::mespv) == PARAM::mespv_polarography ) {
                    _measurementMatrix[mxptr] = 7;  mxptr++;
                    _measurementMatrix[mxptr] = 10; mxptr++;
                } else {
                    _measurementMatrix[mxptr] = 2;  mxptr++;
                    _measurementMatrix[mxptr] = 8;  mxptr++;
                    _measurementMatrix[mxptr] = 10; mxptr++;
                }
            }
        }
    }
}

void EAQtData::CrmxSQW()
{
    int32_t i, last, curr;

    for (i=2 ; i<20 ; i++) {
        if (_measurementMatrix[i] == -1) {
            break;
        }
    }
    last = i;
    _measurementMatrix[2] = 15;
    curr = 3;
    if (curr != (last-1)) {
        if (_measurementMatrix[last-2] == 7) {
            _measurementMatrix[last-2] = 4;
            curr = last-1;
        } else {
            curr = last-2;
        }
    }
    _measurementMatrix[curr] = 13; curr++;
    _measurementMatrix[curr] = 14; curr++;
}

CurveCollection *EAQtData::getCurves()
{
    return _curves;
}

CurveCollection *EAQtData::getMesCurves()
{
    return _mesCurves;
}

int32_t EAQtData::ParamPV(int32_t num)
{
    return _PVParam[num];
}

void EAQtData::ParamPV(int32_t num, int32_t val)
{
    _PVParam[num] = val;
}

int32_t EAQtData::ParamLSV(int32_t num)
{
    return _LSVParam[num];
}

void EAQtData::ParamLSV(int32_t num, int32_t val)
{
    _LSVParam[num] = val;
}

int EAQtData::Act()
{
    return _act;
}

void EAQtData::Act(int toAct)
{
    if ( (int)getCurves()->count() > toAct ) {
        _act = toAct;
    } else {
        _act = SELECT::none;
    }
}

// --------------------------------------------------------
// MDirReadPro - czytaj nazwy krzywych z pliku volt
// --------------------------------------------------------
bool EAQtData::MDirReadPro(QFile &ff)
{
    uint32_t iAux, iFilePos;
    char cAux[1024];
    uint32_t curvesInFile;

    ff.seek(0);
    ff.read((char*)(&iAux), sizeof(uint32_t)); // liczba krzywych w pliku

    curvesInFile = iAux;
    iFilePos=4;

    _fileIndex->clear();

    if (curvesInFile == 0) {
        return false;
    }

    for(uint32_t i = 0;i < curvesInFile;++i) {
        ff.seek(iFilePos);
        uint32_t index = _fileIndex->addNew();
        _fileIndex->get(index)->Off(iFilePos);         	// offset krzywej w pliku
        ff.read((char*)(&iAux), sizeof(uint32_t)); // długość krzywej
        iFilePos += iAux;
        ff.read(cAux, 1); // nazwa krzywej
        int ii=0;
        while (cAux[ii] != '\0') {
            ii++;
            ff.read(cAux+ii, sizeof(char));
        }
        _fileIndex->get(index)->CName(QString(cAux));
    }
    return true;
}

MDirCollection* EAQtData::getMDir()
{
    return _fileIndex;
}

// ------------------------------------------------------
// Czytaj plik volt z krzywymi
// ------------------------------------------------------
void EAQtData::CurReadFilePro(QString *FileName, int PosNr)
{
    QFile ff(*FileName);

    int j;

    if( !ff.open(QIODevice::ReadOnly) ) {
        ff.close();
        //this->ShowMessage("File is empty.");
        return; // plik jest pusty
    }

    if ( !MDirReadPro(ff) )
        return;

    if (PosNr == 0) {
        for (uint32_t i=0 ; i<_fileIndex->count() ; ++i) {
            if ( (j = CurReadCurvePro(ff, _fileIndex->get(i)->CName())) < 0) {
                ff.close();
                //AfxMessageBox(IDS_info4, MB_OK);
                return;  // niepoprawna nazwa krzywej
            } else if ( this->Act() != SELECT::all ) {
                this->Act(j);
            }
        }
    } else if ( (j = CurReadCurvePro(ff, _fileIndex->get(PosNr-1)->CName())) < 0 ) {
        ff.close();
        //AfxMessageBox(IDS_info4, MB_OK);
        return;  // niepoprawna nazwa krzywej
    } else if ( this->Act() != SELECT::all )  {
        this->Act(j);
    }

    ff.close();
    this->_pUI->updateAll();
    this->_pUI->PlotRescaleAxes();
}

// --------------------------------------------------------
// CurReadCurvePro - czytaj krzyw¹ z pliku volt
//           -1 b³¹d, w pliku nie ma krzywych
//           -2 b³¹d, niepoprawna nazwa krzywej
//            >=0 ok, numer krzywej
//
// --------------------------------------------------------
int EAQtData::CurReadCurvePro(QFile &ff, QString pCName)
{
    uint32_t j1;
    uint32_t i;
    int32_t fparam;
    int work;
    uint32_t cLen;
    char buf[256];
    double dwork;
    char cAux[1024];

    MDirReadPro(ff);

    if (_fileIndex->count() == 0)
        return -1;

    for (i=0 ; i<_fileIndex->count() ; i++) {
        if (pCName.compare(_fileIndex->get(i)->CName(),Qt::CaseSensitive) == 0) {
            break;
        }
    }
    if ( i == _fileIndex->count() ) {
        return -2;
    }


    // i numer krzywej
    ff.seek(_fileIndex->get(i)->Off());

    j1 = getCurves()->addNew(1); // TMP nie znam ilosci punktow krzywej
    ff.read((char*)(&cLen), sizeof(uint32_t));			// ilosc bajtow krzywej w pliku
    ff.read(cAux, 1); // nazwa krzywej
    int ii=0;
    while (cAux[ii] != '\0') {
        ii++;
        ff.read(cAux+ii, 1);
    }

    this->getCurves()->get(j1)->CName(QString(cAux));

    ff.read(buf,1); // komentarz
    ii=0;
    while (buf[ii] != '\0')
    {
        ii++;
        ff.read(buf+ii, 1);
    }
    this->getCurves()->get(j1)->Comment(QString(buf));

    ff.read((char*)(&fparam), sizeof(int32_t)*1);		// liczba parametrów

    if ( fparam > PARAM::PARAMNUM ) {
        //        CString	m_String;
        //        m_String.LoadString(IDS_info5);

        //        this->view->MessageBox(m_String);
        this->getCurves()->remove(j1);
        return -1;
    }
    for (ii=0 ; ii<fparam ; ii++) {
        ff.read((char*)(&work),sizeof(int32_t)*1);
        this->getCurves()->get(j1)->Param(ii, work);
    }

    this->getCurves()->get(j1)->reinitializeCurveData(this->getCurves()->get(j1)->Param(PARAM::ptnr));

    // wyniki
    double time;
    double potential;

    for (int ii=0 ; ii<this->getCurves()->get(j1)->Param(PARAM::ptnr) ; ii++) {
        ff.read((char*)(&time), sizeof(double));
        ff.read((char*)(&potential), sizeof(double));
        ff.read((char*)(&dwork), sizeof(double));
        this->getCurves()->get(j1)->addDataPoint(time, potential, dwork);
    }

    if ( this->getCurves()->get(j1)->Param(PARAM::nonaveragedsampling) != 0 ) {
        uint32_t vectorSize;
        ff.read((char*)(&vectorSize),sizeof(uint32_t));
        if ( vectorSize == 0 ) {
            this->getCurves()->get(j1)->Param(PARAM::nonaveragedsampling,0);
        } else {
            this->getCurves()->get(j1)->allocateProbingData(vectorSize);
            float data;
            for (uint32_t j=0;j<vectorSize;j++) {
                ff.read((char*)(&data),sizeof(float));
                this->getCurves()->get(j1)->addProbingDataPoint(data);
            }
        }

    }

    this->SetCurrentRange(this->getCurves()->get(j1)->Param(PARAM::crange),this->getCurves()->get(j1)->Param(PARAM::crange));
    this->getCurves()->get(j1)->FName(ff.fileName());
    this->_pUI->updateAll();

    return(j1);
}

void EAQtData::openFile(QString *filePath, int nrPos)
{
    this->CurReadFilePro(filePath, nrPos);
}

void EAQtData::deleteActiveCurveFromGraph()
{
    if ( this->Act() == SELECT::all ) {
        this->deleteAllCurvesFromGraph();
    } else if ( this->Act() >= 0 && this->getCurves()->get(this->Act()) != NULL ) {
        this->getCurves()->remove(this->Act());
        this->Act(SELECT::none);
        this->_pUI->updateAll();
    }
}

void EAQtData::deleteNonactiveCurvesFromGraph()
{
    if ( this->getCurves()->get(this->Act()) != NULL ) {
        Curve *c = this->getCurves()->get(this->Act());
        this->getCurves()->unset(this->Act());
        this->getCurves()->clear();
        uint32_t index = this->getCurves()->append(c);
        this->Act(index);
        this->_pUI->updateAll();
    }
}

void EAQtData::deleteAllCurvesFromGraph()
{
    if ( this->_pUI->showQuestionBox(
             tr("Delete all curves?"),
             tr("Are you sure")) ) {
        this->getCurves()->clear();;
        this->Act(SELECT::none);
        this->_pUI->updateAll();
    }
}

void EAQtData::ProcessPacketFromEA(char* packet)
{
    char* RxBuf;
    RxBuf = packet;

    int Cmd, i;
    int16_t work;
    int64_t workl;
    uint32_t DataLen;
    int32_t twCounter;
    static int32_t ActSampl1, ActSampl2;
    static uint32_t previousPointNr, currentPointNr, previousCurveNr;
    uint32_t currentCurveNr = 0;

    Cmd = RxBuf[0];

    switch (Cmd) {
    case EA2PC_RECORDS::calibPV:
        this->_IUE0 = RxBuf[2] + 256*RxBuf[3];
        previousPointNr = 0;
        previousCurveNr =0;
        ActSampl1 = 0;
        ActSampl2 = 0;
        _ctnrSQW = 6;
        break;

    case EA2PC_RECORDS::calibLSV:
        this->_IUE0 = RxBuf[2] + 256*RxBuf[3];
        previousPointNr = 0;
        previousCurveNr =0;
        break;

    case EA2PC_RECORDS::recordPV:
        this->_endOfMes = (uint8_t)RxBuf[1];
        DataLen = ((uint8_t)RxBuf[2] + 256*(uint8_t)RxBuf[3])-8; // było -6
        currentPointNr = (uint8_t)RxBuf[4] + 256*(uint8_t)RxBuf[5];
        currentCurveNr = RxBuf[6] + 256*RxBuf[7];	// prox

        if ( this->_endOfMes ) {
            _measurementGo = 0;
        }

        if ( currentPointNr != previousPointNr
        || previousCurveNr != currentCurveNr ) {
            ActSampl1 = 0;
            ActSampl2 = 0;
            twCounter = 0;
            _ctnrSQW = 6;	// nr ms w 1. i 2. próbkowaniu
            this->MesUpdate(previousCurveNr, previousPointNr);
        }
        i = MEASUREMENT::PVstartData; // == 6

        if (this->getMesCurves()->get(currentCurveNr)->Param(PARAM::method) < PARAM::method_sqw ) { // IMPULSOWE (nie SQW, nie LSV)
            while (DataLen > 0) {
                twCounter++;
                if ( this->getMesCurves()->get(currentCurveNr)->Param(PARAM::nonaveragedsampling) != 0 ) {
                    // pomiar idzie dla tp i tw, robimy cos innego
                    work =  (uint8_t)RxBuf[i] + 256*(uint8_t)RxBuf[i+1];
                    workl = (long)work;
                    this->getMesCurves()->get(currentCurveNr)->addProbingDataPoint(this->CountResultPV(60L*workl));
                    if ( twCounter <= this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tw) ) {
                        // jestesmy w trakcie tw
                        i+= 2;
                        DataLen -=2 ;
                        continue;
                    }
                    if ( this->getMesCurves()->get(currentCurveNr)->Param(PARAM::sampl) == PARAM::sampl_double ) {
                        if (( twCounter <= 2*this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tw)+ this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tp))
                                && ( twCounter > this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tw)+ this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tp))) {
                            // jestesmy w trakcie tw
                            i+= 2;
                            DataLen -=2 ;
                            continue;
                        }
                    }
                    if ( (this->getMesCurves()->get(currentCurveNr)->Param(PARAM::sampl) == PARAM::sampl_double)
                         &&   (ActSampl1 < this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tp)) ) {
                        *this->getMesCurves()->get(currentCurveNr)->getMesCurrent1Point(currentPointNr) +=  workl;
                        i+= 2;
                        ActSampl1 ++;
                    } else {
                        *this->getMesCurves()->get(currentCurveNr)->getMesCurrent2Point(currentPointNr) +=  workl;
                        i+= 2;
                        ActSampl2 ++;
                        *this->getMesCurves()->get(currentCurveNr)->getMesTimePoint(currentPointNr) += 1;
                    }
                } else {
                    if ( (this->getMesCurves()->get(currentCurveNr)->Param(PARAM::sampl) == PARAM::sampl_double)
                         &&   (ActSampl1 < this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tp)) ) {
                        work =  (uint8_t)RxBuf[i] + 256*(uint8_t)RxBuf[i+1];
                        workl = (long)work;
                        *this->getMesCurves()->get(currentCurveNr)->getMesCurrent1Point(currentPointNr) +=  workl;
                        i+= 2;
                        ActSampl1 ++;
                    } else {
                        work = (uint8_t)RxBuf[i] + 256*(uint8_t)RxBuf[i+1];// ((RxBuf[i+1]<<8) | (uint8_t)RxBuf[i]);
                        workl = (long)work;
                        *this->getMesCurves()->get(currentCurveNr)->getMesCurrent2Point(currentPointNr) +=  workl;
                        i+= 2;
                        ActSampl2 ++;
                        *this->getMesCurves()->get(currentCurveNr)->getMesTimePoint(currentPointNr) += 1;
                    }
                }
                DataLen -= 2;
            }
        } else if (this->getMesCurves()->get(currentCurveNr)->Param(PARAM::method) == PARAM::method_sqw )  {
            while (DataLen > 0) {
                if ( (this->getMesCurves()->get(currentCurveNr)->Param(PARAM::sampl) == PARAM::sampl_double)
                     &&   (ActSampl1 < this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tp))  ) {
                    work =  (uint8_t)RxBuf[i] + 256*(uint8_t)RxBuf[i+1];
                    workl = (long)work;
                    *this->getMesCurves()->get(currentCurveNr)->getMesCurrent1Point(currentPointNr) +=  workl;
                    i+= 2;
                    ActSampl1 ++;
                } else {
                    if ((_ctnrSQW == 6) || (_ctnrSQW == 4) || (_ctnrSQW == 2)) {
                        work =  (uint8_t)RxBuf[i] + 256*(uint8_t)RxBuf[i+1];
                        workl = (long)work;
                        *this->getMesCurves()->get(currentCurveNr)->getMesCurrent2Point(currentPointNr) +=  workl/3;
                    } else {
                        work =  (uint8_t)RxBuf[i] + 256*(uint8_t)RxBuf[i+1];
                        workl = (long)work;
                        *this->getMesCurves()->get(currentCurveNr)->getMesCurrent2Point(currentPointNr) -=  workl/3;
                    }
                    if (_ctnrSQW == 6) {
                        *this->getMesCurves()->get(currentCurveNr)->getMesTimePoint(currentPointNr) += 1;
                    }
                    i+= 2;
                    ActSampl2 ++;
                    if ( ActSampl2 == this->getMesCurves()->get(currentCurveNr)->Param(PARAM::tp) ) {
                        ActSampl2 = 0;
                        _ctnrSQW--;
                    }
                }
                DataLen -= 2;
            }
        }

        previousPointNr = currentPointNr;
        previousCurveNr = currentCurveNr;

        if ( this->_endOfMes ) {
            this->MesAfter();
        }

        break;

    case EA2PC_RECORDS::recordLSV:
        //TODO: dlaczego tu sa klasowe zmienne ?
        this->_endOfMes = RxBuf[1];
        DataLen = (RxBuf[2] + 256*RxBuf[3])-8;
        static int32_t firstCycle = (RxBuf[4] + 256*RxBuf[5]);
        static int32_t averageCounter1 = (RxBuf[6] + 256*RxBuf[7]);
        if ( this->_endOfMes ) {
            _measurementGo = 0;
        }

        i = MEASUREMENT::LSVstartData; // == 8

        while ( DataLen > 0 ) {
            currentCurveNr = (RxBuf[i] + 256*RxBuf[i+1]);
            i+= 2;
            _mesPointNr = (RxBuf[i] + 256*RxBuf[i+1]);
            i+= 2;
            _currentResult = ((long)RxBuf[i+3]<<24) | ((long)RxBuf[i+2]<<16) | ((long)RxBuf[i+1]<<8) | ((long)RxBuf[i]);
            i += 4;
            DataLen -= 8;
            if ( firstCycle ) {
                *this->getMesCurves()->get(currentCurveNr)->getMesTimePoint(_mesPointNr) += _samplingTime;
            } else if (*this->getMesCurves()->get(currentCurveNr)->getMesTimePoint(_mesPointNr) == 0) {
                *this->getMesCurves()->get(currentCurveNr)->getMesTimePoint(_mesPointNr) = _samplingTime;
            }
            if (this->getMesCurves()->get(currentCurveNr)->Param(PARAM::messc) == PARAM::messc_multicyclic ) {
                *this->getMesCurves()->get(currentCurveNr)->getMesCurrent1Point(_mesPointNr) = _currentResult;
                //TotalCntr ++;
            } else {
                *this->getMesCurves()->get(currentCurveNr)->getMesCurrent1Point(_mesPointNr) += _currentResult;
            }
            this->MesUpdate(currentCurveNr,_mesPointNr);
        }
        if ( this->_endOfMes ) {
            this->MesAfter();
        }
        break;

    case EA2PC_RECORDS::startELSV:
        _EstartCurrentTime = ((long)RxBuf[4]<<24) | ((long)RxBuf[3]<<16) | ((long)RxBuf[2]<<8) | ((long)RxBuf[1]);
        this->updateELSV();
        break;

    case EA2PC_RECORDS::recordPause:
        work = (RxBuf[2]<<8) | RxBuf[1];
        workl = (int64_t)work;
        _break.currentE = workl;
        _break.targetMin = RxBuf[3] + 256*RxBuf[4];
        _break.targetSec = RxBuf[5] + 256*RxBuf[6];
        _break.currentMin = RxBuf[7] + 256*RxBuf[8];
        _break.currentSec = RxBuf[9] + 256*RxBuf[10];
        this->updatePause();
        break;

    case EA2PC_RECORDS::recordTestCGMDE:
        _CGMDETestNr = (RxBuf[2]<<8) | RxBuf[1];
        this->updateTestCGMDE();
        break;

    default:
        break;
    }
}

// --------------------------------------------------------------------------------------
void EAQtData::MesStart(bool isLsv)
{
    //this->graphInterface->showMessageBox("Starting Measurement.");
    uint32_t actptnr;
    int32_t nrOfCurvesMeasured = 1;
    uint32_t work;
    int32_t mesCurveIndex;
    int i;

    this->_pUI->changeStartButtonText(tr("Start ") + (isLsv?"LSV":"PV"));

    this->_wasLSVMeasurement = isLsv;

    if ( !this->_network->connectToEA() ) {
        return;
    }

    if ( this->_useSeriesFile && !this->_isMesSeries ) {
        // Wczytaj, ale tylko raz
        this->loadMesFile();
        if ( this->_isMesSeries ) {
            // udało się wczytać plik
            this->_pSeriesData->Mes_Start();
        } else {
            //wczytanie się nie powiodło
            //            CString	m_String;
            //            m_String.LoadString(IDS_info12);
            //            view->MessageBoxA(m_String);
            return;
        }
    }

    //////////////// SERIA /////////////////////////////////////////////////////
    if ( _isMesSeries == true ) {
        for (int ii=0 ; ii<PARAM::PARAMNUM ; ii++) {
            if ( ii != PARAM::pro  // parametry nie ustawiane (ignorowane) w eacfg (do ustawienia w EAPro)
                 && ii != PARAM::nonaveragedsampling
                 && ii != PARAM::multi
                 && ii != PARAM::sti
                 && ii != PARAM::kp
                 && ii != PARAM::kpt ) {
                _PVParam[ii] = _pSeriesData->Mes_Param(ii);
            }
        }

        if ( this->_pSeriesData->Mes_PotentialProgramLength() > 0 ) {
            _PVParam[PARAM::pro]= 1;
        }

        if ( this->_pSeriesData->Mes_WaitForUser() == true ) {
            //			CString	m_String;
            //			m_String.LoadString(IDS_info13);

            //			view->MessageBox(m_String);
        }
        if ( this->_pSeriesData->Mes_Delay() != 0 ) {
            this->SeriaWait(this->_pSeriesData->Mes_Delay());
        }
    }
    /////////////////////////////////////////////////////////////////////////////
    // sprawdzić jaki typ pomiaru był ostatnio wybierany

    int itp = _PVParam[PARAM::tp];
    int itw = _PVParam[PARAM::tw];

    if ( _wasLSVMeasurement != 0 ) {  // DODATKOWE DLA DLA LSV
        _ptnrFromEss = 0;
        _samplingTime = MEASUREMENT::LSVtime[_LSVParam[PARAM::dEdt]]; //Very important !

        if ( _LSVParam[PARAM::Ek] == _LSVParam[PARAM::Ep] ) {
            //            CString	m_String;
            //            m_String.LoadString(IDS_info15);
            this->_pUI->showMessageBox("IDS_info15");
            return;
        }

        _LSVParam[PARAM::sti] = _PVParam[PARAM::sti];
        _LSVParam[PARAM::kp] = _PVParam[PARAM::kp];
        _LSVParam[PARAM::kpt] = _PVParam[PARAM::kpt];

        _EstartInfo = (bool)_LSVParam[PARAM::Escheck];

        actptnr = (int)fabs((double)(_LSVParam[PARAM::Ek]-_LSVParam[PARAM::Ep]) /
                MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]);

        _LSVParam[PARAM::ptnr] = actptnr;

        if ( _LSVParam[PARAM::messc] == PARAM::messc_cyclic ) {
            nrOfCurvesMeasured = 2; // cykliczny
        } else if ( _LSVParam[PARAM::messc] == PARAM::messc_multicyclic ) {
            if ( _LSVParam[PARAM::aver] == 0 ) {
                _LSVParam[PARAM::aver] = 1;
            }
            nrOfCurvesMeasured = 2*_LSVParam[PARAM::aver]; // czyli to jest zle ?
        }

        double timeCounter = 1.0;

        for ( int iii=0; iii<nrOfCurvesMeasured; iii++) { // rysowana jest jedna krzywa niezależnie
            // od tego czy jest cykliczna czy nie
            mesCurveIndex = this->getMesCurves()->addNew(_LSVParam[PARAM::ptnr]);
            QVector<double> vecMesPotential;
            QVector<double> vecMesTime;
            for (i=0 ; i<PARAM::PARAMNUM ; i++) {
                this->getMesCurves()->get(mesCurveIndex)->Param(i, _LSVParam[i]);
            }
            this->getMesCurves()->get(mesCurveIndex)->allocateMesArray();

            /*
            * !!! Obliczanie wektorów czasu i potencjału dla każdej krzywej
            */
            if ( (mesCurveIndex == 0)
                 && (_LSVParam[PARAM::messc] == PARAM::messc_multicyclic )
                 && (_LSVParam[PARAM::Escheck] == PARAM::Escheck_yes) ) {
                /*
                * Gdy interpretujemy potencjał startowy, to się dzieje :)
                * nie uwzględniono multielektr !
                */
                if ( _LSVParam[PARAM::Ep] < _LSVParam[PARAM::Ek] ) {
                    if ( _LSVParam[PARAM::EstartLSV] >= _LSVParam[PARAM::Ep]
                         &&  _LSVParam[PARAM::EstartLSV] <= _LSVParam[PARAM::Ek] ) {
                        QVector<double> vecMesPotential;
                        QVector<double> vecMesTime;
                        this->_ptnrFromEss = (int) ceil( fabs( (_LSVParam[PARAM::Ek] - _LSVParam[PARAM::EstartLSV])
                                                                / MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                        vecMesTime.resize(_LSVParam[PARAM::ptnr]);
                        int p;
                        for ( p = 0; p<(_LSVParam[PARAM::ptnr]-this->_ptnrFromEss); ++p ) {
                            vecMesPotential[p] = _LSVParam[PARAM::EstartLSV];
                            vecMesTime[p] = MEASUREMENT::LSVtime[_LSVParam[PARAM::dEdt]];
                        }
                        for ( p=p; p<_LSVParam[PARAM::ptnr]; ++p ) {
                            vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] + ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            vecMesTime[p]=( timeCounter++ * MEASUREMENT::LSVtime[_LSVParam[PARAM::dEdt]] );
                        }
                        //MesCurve(mesCurveIndex)->reinitializeCurveData(ptnrFromEss);
                        getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                        getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
                    } else {
                        //Brak potencjału startowego na wykresie -- tylko przyłożony potencjał bez mierzenia //
                        QVector<double> vecMesPotential;
                        QVector<double> vecMesTime;
                        vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                        vecMesTime.resize(_LSVParam[PARAM::ptnr]);
                        for ( int p=0; p<_LSVParam[PARAM::ptnr]; p++ ) {
                            if ( _LSVParam[PARAM::Ep] < _LSVParam[PARAM::Ek] ) {
                                vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] + ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            } else {
                                vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] - ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            }
                            vecMesTime[p]=( timeCounter++ * MEASUREMENT::LSVtime[_LSVParam[PARAM::dEdt]] );
                        }
                        getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                        getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
                    }
                } else {
                    if ( _LSVParam[PARAM::EstartLSV] <= _LSVParam[PARAM::Ep]
                         &&  _LSVParam[PARAM::EstartLSV] >= _LSVParam[PARAM::Ek] ) {
                        QVector<double> vecMesPotential;
                        QVector<double> vecMesTime;
                        this->_ptnrFromEss = (int) ceil( fabs( (_LSVParam[PARAM::Ep] - _LSVParam[PARAM::EstartLSV])
                                                                / MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                        vecMesTime.resize(_LSVParam[PARAM::ptnr]);
                        int p;
                        for ( p = 0; p<(_LSVParam[PARAM::ptnr]-this->_ptnrFromEss); ++p ) {
                            vecMesPotential[p] = _LSVParam[PARAM::EstartLSV];
                            vecMesTime[p] = MEASUREMENT::LSVtime[_LSVParam[PARAM::dEdt]];
                        }
                        for ( p=p; p<_LSVParam[PARAM::ptnr]; ++p ) {
                            vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] - ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            vecMesTime[p]=( timeCounter++ * MEASUREMENT::LSVtime[_LSVParam[PARAM::dEdt]] );
                        }
                        getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                        getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
                    } else {
                        //Brak potencjału startowego na wykresie -- tylko przyłożony potencjał bez mierzenia //
                        QVector<double> vecMesPotential;
                        QVector<double> vecMesTime;
                        vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                        vecMesTime.resize(_LSVParam[PARAM::ptnr]);
                        for ( int p=0; p<_LSVParam[PARAM::ptnr]; p++ ) {
                            if ( _LSVParam[PARAM::Ep] < _LSVParam[PARAM::Ek] ) {
                                vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] + ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            } else {
                                vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] - ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            }
                            vecMesTime[p]=( timeCounter++ * MEASUREMENT::LSVtime[_LSVParam[PARAM::dEdt]] );
                        }
                        getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                        getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
                    }
                }
            } else {
                /*
                * Gdy nie interpretujemy potencjału startowego, to nic ciekawego
                */
                vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                vecMesTime.resize(_LSVParam[PARAM::ptnr]);
                for ( int p=0; p<_LSVParam[PARAM::ptnr]; p++ ) {
                    if ( mesCurveIndex % 2 == 0 ) {
                        if ( _LSVParam[PARAM::Ep] < _LSVParam[PARAM::Ek] ) {
                            vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] + ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        } else {
                            vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] - ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        }
                    } else {
                        if ( _LSVParam[PARAM::Ep] > _LSVParam[PARAM::Ek] ) {
                            vecMesPotential[p]=(double)( _LSVParam[PARAM::Ek] + ((double)(1.0+p) * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        } else {
                            vecMesPotential[p]=(double)( _LSVParam[PARAM::Ek] - ((double)(1.0+p) * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        }
                    }
                    vecMesTime[p]=( timeCounter++ * MEASUREMENT::LSVtime[_LSVParam[PARAM::dEdt]] );
                }
                getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
            }
            /*
            * !!!
            */
        }
        SetCurrentRange(_LSVParam[PARAM::crange],_LSVParam[PARAM::electr]);
    } else { // KONIEC DODATKOWE LSV, POCZATEK DODATKOWE DLA PV

        if ( _PVParam[PARAM::nonaveragedsampling] != 0 ) {
            _PVParam[PARAM::tp]=itp+itw;
            _PVParam[PARAM::tw]=0;
        }

        int nrOfElectrodes = 1;

        if ( _PVParam[PARAM::electr] == PARAM::electr_multi ) { //Wieloelektrodowy
            nrOfCurvesMeasured = 0; //wg. okna dialogowego
            work = _PVParam[PARAM::multi] & 0x000000ff;
            for (int i=0 ; i<8; i++) {
                if ((work & 0x0080) != 0) nrOfCurvesMeasured++;
                work = work << 1;
            }
            nrOfElectrodes = nrOfCurvesMeasured;
        }
        if ( _PVParam[PARAM::messc] == 1 )
            nrOfCurvesMeasured *= 2; // cykliczny

        int stepEtmp;
        stepEtmp = _PVParam[PARAM::Estep];

        if ( _PVParam[PARAM::Ep] != _PVParam[PARAM::Ek] ) {
            if ( _PVParam[PARAM::Ek] > _PVParam[PARAM::Ep] ) {
                stepEtmp = abs(stepEtmp);
            } else {
                stepEtmp = -abs(stepEtmp);
            }
            _PVParam[PARAM::ptnr] =  (int)floor( (double)(_PVParam[PARAM::Ek] - _PVParam[PARAM::Ep]) / (double)stepEtmp);
            _PVParam[PARAM::Estep] = stepEtmp;
        } else {
            _PVParam[PARAM::Estep] = 0;
        }

        _PVParam[PARAM::Ek] = _PVParam[PARAM::Ep] + (_PVParam[PARAM::Estep]*_PVParam[PARAM::ptnr]);

        for ( int iii=0; iii<nrOfCurvesMeasured; iii++) { // rysowana jest jedna krzywa niezależnie
            // od tego czy jest cykliczna czy nie
            mesCurveIndex = this->getMesCurves()->addNew(_PVParam[PARAM::ptnr]);
            for (i=0 ; i<PARAM::PARAMNUM ; i++) {
                this->getMesCurves()->get(mesCurveIndex)->Param(i, _PVParam[i]);
            }

            if ( this->_isMesSeries
                 && _PVParam[PARAM::pro] == PARAM::pro_yes
                 && this->_pSeriesData->Mes_PotentialProgramLength() > 0 ) {
                //
                // TYLKO DLA Programowania potencjalu
                //
                getMesCurves()->get(mesCurveIndex)->reinitializeCurveData(this->_pSeriesData->Mes_PotentialProgramLength());
                QVector<double> vecP;
                QVector<double> vecT;
                if ( _PVParam[PARAM::messc] == PARAM::messc_cyclic && nrOfElectrodes <= mesCurveIndex ) {
                    //powrot cyklicznej
                    for ( int y=this->_pSeriesData->Mes_PotentialProgramLength()-1; y<=0; y-- ) {
                        vecP.append(this->_pSeriesData->Mes_PotentialProgram()[y]);
                        vecT.append( (2*this->_pSeriesData->Mes_PotentialProgramLength()-(y+1))*2*(_PVParam[PARAM::tp]+_PVParam[PARAM::tw]));
                    }
                } else {
                    // normalny pomiar
                    for (int y=0; y<this->_pSeriesData->Mes_PotentialProgramLength();y++) {
                        vecP.append(this->_pSeriesData->Mes_PotentialProgram()[y]);
                        vecT.append( (y+1)*2*(_PVParam[PARAM::tp]+_PVParam[PARAM::tw]) );
                    }
                }
                getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecP);
                getMesCurves()->get(mesCurveIndex)->setTimeVector(vecT);
                getMesCurves()->get(mesCurveIndex)->Param(PARAM::method,2);
                getMesCurves()->get(mesCurveIndex)->Param(PARAM::Ep, 0);
                getMesCurves()->get(mesCurveIndex)->Param(PARAM::Estep, 1);
                if ( getMesCurves()->get(mesCurveIndex)->Param(PARAM::sampl) == PARAM::sampl_single ) {
                    getMesCurves()->get(mesCurveIndex)->Param(PARAM::ptnr,this->_pSeriesData->Mes_PotentialProgramLength());
                    getMesCurves()->get(mesCurveIndex)->Param(PARAM::Ek,this->_pSeriesData->Mes_PotentialProgramLength());
                } else {
                    getMesCurves()->get(mesCurveIndex)->Param(PARAM::ptnr,this->_pSeriesData->Mes_PotentialProgramLength()/2);
                    getMesCurves()->get(mesCurveIndex)->Param(PARAM::Ek,this->_pSeriesData->Mes_PotentialProgramLength()/2);
                }

            } else {
                //
                // Jak bez programowania potencjalu
                //
                QVector<double> vecMesPotential;
                QVector<double> vecMesTime;
                vecMesPotential.resize(_PVParam[PARAM::ptnr]);
                vecMesTime.resize(_PVParam[PARAM::ptnr]);
                for ( int p=0; p<_PVParam[PARAM::ptnr]; p++ ) {
                    if ( nrOfElectrodes > mesCurveIndex ) {
                        vecMesTime[p] = ( (p+1.0) * 2.0 * (_PVParam[PARAM::tw]+_PVParam[PARAM::tp]) );
                        vecMesPotential[p] = _PVParam[PARAM::Ep] + (p* _PVParam[PARAM::Estep]);

                    } else { // powrót cyklicznej //
                        vecMesTime[p]= (
                                    _PVParam[PARAM::ptnr] * 2.0 * (_PVParam[PARAM::tw]+_PVParam[PARAM::tp])
                                + (p * 2.0 * (_PVParam[PARAM::tw]+_PVParam[PARAM::tp]))
                                );
                        vecMesPotential[p] = _PVParam[PARAM::Ek] - ( (1.0+p) * _PVParam[PARAM::Estep]);
                    }
                }

                getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
            }

            if ( getMesCurves()->get(mesCurveIndex)->Param(PARAM::nonaveragedsampling) != 0 ) {
                getMesCurves()->get(mesCurveIndex)->allocateProbingData();
            }
            this->getMesCurves()->get(mesCurveIndex)->allocateMesArray();
        }

        SetCurrentRange(_PVParam[PARAM::crange],_PVParam[PARAM::electr]);
        InitEca();
        InitPtime();
        //ilpw = this->getMesCurves()->get(0)->Param(PARAM::aver);
        CreateMatrix();
        ainmat = 0;
        if (_measurementMatrix[0] == -1) {
            ainmat = 1;
        }
        if ( _PVParam[PARAM::method] == PARAM::method_sqw ) {
            CrmxSQW(); // SQW
        }

    } // KONIEC DODATKOWE

    //sbuf.LoadString(IDD_off);

    //    if (!is_connect(view)) {
    //        view->MessageBox(sbuf);

    //        m_alMesParam[tp]=itp;
    //        m_alMesParam[tw]=itw;
    //        int nn=0;
    //        while (MesCurve(nn)!=NULL) {
    //            MesCurve(nn)->Param(tp,m_alMesParam[tp]);
    //            MesCurve(nn)->Param(tw,m_alMesParam[tw]);
    //            nn++;
    //        }

    //        return;
    //    }
    //    view->m_sSubtext = _T("");
    //    view->mHandler->ChangeMouseMode(MouseHandler::mouseMode::mesurement,
    //                                    MouseHandler::userFunctions::none);

    //    if (this->m_alMesParam[PARAM::Ep] == this->m_alMesParam[PARAM::Ek]) {
    //        this->setXAxis(XTIME);
    //    }

    _stopInfo = 0;
    _conductingMeasurement = 1;

    if ( _wasLSVMeasurement == 0 ){
        if ( SendPVToEA() ) {
            _PVParam[PARAM::tp]=itp;
            _PVParam[PARAM::tw]=itw;
            int nn=0;
            while (getMesCurves()->get(nn)!=NULL) {
                getMesCurves()->get(nn)->Param(PARAM::tp,_PVParam[PARAM::tp]);
                getMesCurves()->get(nn)->Param(PARAM::tw,_PVParam[PARAM::tw]);
                nn++;
            }

            return;
        } else { // m_nMesInfo=1 - pomiar techniki impulsowe
            // b³edy podczas transmisji parametrów lub EA od³aczony
            _conductingMeasurement = 0;
            //view->MessageBox(sbuf);
            _PVParam[PARAM::tp]=itp;
            _PVParam[PARAM::tw]=itw;
            this->getMesCurves()->clear();
            //view->enableAllButtons();
            //this->bIsSeriaMes = false;
            return;
        }
    } else {
        if ( SendLSVToEA() ) {
            return ;
        } // m_nMesInfo=2,3 - pomiar LSV
        else {
            _conductingMeasurement = 0;
            //view->MessageBox(sbuf);
            this->getMesCurves()->clear();
            //view->enableAllButtons();
            //this->bIsSeriaMes = false;
            return ;
        } // błedy podczas transmisji parametrów lub EA odłaczony
    }
}

bool EAQtData::SendPVToEA()
{
    int32_t i, lenEact;
    uint32_t TxN;

    char TxBufAlt[320];

    // Pomiar - techniki impulsowe
    // Z komputera: Param, matrix, E1I, E2I, E3I, stepE, ptime, tr(long), drops_bm
    TxN = 0;
    _TxBuf[0] = PC2EA_RECORODS::recordPV;
    TxN = 3;

    int16_t lenE;
    int16_t E[1200];
    if ( this->_isMesSeries && this->getMesCurves()->get(0)->Param(PARAM::pro) == 1 ) {
        lenE = this->_pSeriesData->Mes_PotentialProgramLength();
        for ( int i=0; i<lenE; i++ ) {
            //E[i] = i*10;
            E[i]=this->_pSeriesData->Mes_PotentialProgram()[i];
        }
        E1I = E[0];
        E2I = E[1];
        E3I = E2I - this->getMesCurves()->get(0)->Param(PARAM::dE);
    }

    for (i=0 ; i<PARAM::PARAMNUM ; i++) {
        _TxBuf[TxN] = (unsigned char)(this->getMesCurves()->get(0)->Param(i) & 0x000000ff); TxN++;
        _TxBuf[TxN] = (unsigned char)((this->getMesCurves()->get(0)->Param(i) >> 8) & 0x000000ff); TxN++;
        _TxBuf[TxN] = (unsigned char)((this->getMesCurves()->get(0)->Param(i) >> 16) & 0x000000ff); TxN++;
        _TxBuf[TxN] = (unsigned char)((this->getMesCurves()->get(0)->Param(i) >> 24) & 0x000000ff); TxN++;
    }

    for (i=0 ; i<20 ; i++) {
        _TxBuf[TxN] = (unsigned char)(_measurementMatrix[i] & 0x00ff); TxN++;
        _TxBuf[TxN] = (unsigned char)((_measurementMatrix[i] >> 8) & 0x00ff); TxN++;
    }

    _TxBuf[TxN] = (unsigned char)(E1I & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((E1I >> 8) & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)(E2I & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((E2I >> 8) & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)(E3I & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((E3I >> 8) & 0x00ff); TxN++;

    int32_t stepE = abs(this->getMesCurves()->get(0)->Param(PARAM::Estep));

    _TxBuf[TxN] = (unsigned char)(stepE & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((stepE >> 8) & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)(_singleStepTime & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((_singleStepTime >> 8) & 0x00ff); TxN++;

    _TxBuf[TxN] = (unsigned char)(_trValue & 0x000000ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((_trValue >> 8) & 0x000000ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((_trValue >> 16) & 0x000000ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((_trValue >> 24) & 0x000000ff); TxN++;

    _TxBuf[TxN] = (unsigned char)(_accessories.drops_bm & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((_accessories.drops_bm >> 8) & 0x00ff); TxN++;

    _TxBuf[1] = (unsigned char)(TxN & 0x00ff);
    _TxBuf[2] = (unsigned char)((TxN >> 8) & 0x00ff);

    if ( this->_network->sendToEA(&_TxBuf[0], TxN) > 0 ) {
        return true;
    } else {
        return false;
    }

    if ( this->getMesCurves()->get(0)->Param(PARAM::pro) == 1 ) {
        int32_t TxNAlt=0;
        TxBufAlt[TxNAlt] = PC2EA_RECORODS::recordPVE;


        int32_t userEi=2;
        while (userEi < lenE) {
            TxNAlt=4;
            lenEact = lenE - userEi;
            TxBufAlt[3] = 1;
            if (lenEact >= 100) {
                lenEact = 100;
                TxBufAlt[3] = 0;
            }
            TxBufAlt[TxNAlt] = (unsigned char)(lenEact & 0x00ff); TxNAlt++;
            TxBufAlt[TxNAlt] = (unsigned char)((lenEact >> 8) & 0x00ff); TxNAlt++;

            for (int i=0 ; i<lenEact ; i++) {
                TxBufAlt[TxNAlt] = (unsigned char)(E[userEi + i] & 0x00ff); TxNAlt++;
                TxBufAlt[TxNAlt] = (unsigned char)((E[userEi + i] >> 8) & 0x00ff); TxNAlt++;
            }
            userEi += lenEact;

            for (int i =0;i<320;i++) {
                _TxBuf[i]=TxBufAlt[i];
            }
            _TxBuf[1] = (unsigned char)(TxNAlt & 0x00ff);
            _TxBuf[2] = (unsigned char)((TxNAlt >> 8) & 0x00ff);

            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);// sleep

            this->_network->sendToEA(&_TxBuf[0], TxNAlt);

        }

    }
}

void EAQtData::MesStop()
{
    _stopInfo = 1;
    if (_conductingMeasurement == 0) {
        this->_pUI->showMessageBox("IDS_info20"); // Pomiar nie został zainicjowany
    } else {
        _conductingMeasurement = 0;
        // wyczyszczenie pola textowego
        //view->m_sSubtext = _T("");
        //view->UpdateData( FALSE );
        _TxBuf[0] = PC2EA_RECORODS::recordStop;
        //TxN = TxBufLgth;
        this->_network->sendToEA(&_TxBuf[0],1);

        //		if ( this->bIsSeriaMes == true ) {
        //			this->pMesData->~MesCFG();
        //			this->pMesData = new MesCFG();
        //			this->bIsSeriaMes = false;
        //		}

        //		view->enableAllButtons();
    }
}

bool EAQtData::SendLSVToEA()
{
    int i;
    uint16_t TxN;

    // CGMDE
    //m_alLsvParam[impnr] = m_alMesParam[impnr];
    //m_alLsvParam[inttime] = m_alMesParam[inttime];
    //m_alLsvParam[imptime] = m_alMesParam[imptime];


    // Pomiar - LSV
    // Z komputera do EA: LSVParam, axisx, start_info, drops_bm
    TxN = 0;
    _TxBuf[0] = PC2EA_RECORODS::recordLSV;
    TxN = 3;

    for (i=0 ; i<62 ; i++) {
        _TxBuf[TxN] = (unsigned char)(_LSVParam[i] & 0x000000ff); TxN++;
        _TxBuf[TxN] = (unsigned char)((_LSVParam[i] >> 8) & 0x000000ff); TxN++;
        _TxBuf[TxN] = (unsigned char)((_LSVParam[i] >> 16) & 0x000000ff); TxN++;
        _TxBuf[TxN] = (unsigned char)((_LSVParam[i] >> 24) & 0x000000ff); TxN++;
    }
    int32_t work = 10240; // TODO:
    _TxBuf[TxN] = (unsigned char)(work & 0x000000ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((work >> 8) & 0x000000ff); TxN++;
    _TxBuf[TxN] = (unsigned char)(_EstartInfo & 0x000000ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((_EstartInfo >> 8) & 0x000000ff); TxN++;
    _TxBuf[TxN] = (unsigned char)(_accessories.drops_bm & 0x000000ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((_accessories.drops_bm >> 8) & 0x000000ff); TxN++;

    _TxBuf[1] = (unsigned char)(TxN & 0x00ff);
    _TxBuf[2] = (unsigned char)((TxN >> 8) & 0x00ff);


    if ( this->_network->sendToEA(&_TxBuf[0], TxN) > 0 ) {
        return true;
    } else {
        return false;
    }
}

void EAQtData::MesUpdate(uint32_t nNrOfMesCurve, uint32_t nPointFromDevice)
{
    ///////////////// SETUP //////////////////
    if ( this->_performSetup == true && !this->_endOfMes ) {
        this->_performSetup = false;
        /*
        * zliczanie ilości elektrod w wieloelektrodowym
        */

        if ( this->_PVParam[PARAM::electr] == PARAM::electr_multi ) {
            this->_multielectrodeNr = 0;
            uint32_t work;
            work = this->_PVParam[PARAM::multi];
            for (int i=0 ; i<8 ; i++) {
                if ((work & 0x00000080) != 0) {
                    this->_multielectrodeNr++;
                }
                work = work << 1;
            }
        } else {
            this->_multielectrodeNr = 1;
        }

        this->_pUI->MeasurementSetup();

        int ic = 0;
        while ( getMesCurves()->get(ic) != NULL ) {
            if ( !getMesCurves()->get(ic)->hasPlot() ) {
                getMesCurves()->get(ic)->setPlot(this->_pUI->PlotAddGraph());
                getMesCurves()->get(ic)->changeToMesPlot();
            }
            ic++;
        }
        if ( getMesCurves()->get(0)->Param(PARAM::Escheck) == PARAM::Escheck_yes
             && this->_ptnrFromEss != 0 ) {
            this->_prepareEstart = true;
        }
    }
    //////////////// END SETUP ////////////////

    ///////////////// CHECKS ///////////////////*
    if ( *this->getMesCurves()->get(nNrOfMesCurve)->getMesTimePoint(nPointFromDevice) == 0 ) {
        return;
    }
    ////////////// END CHECKS /////////////////////////

    ///////////// CALC RESULT ////////////////////
    if ( _wasLSVMeasurement == 0 ) {
        if ( nNrOfMesCurve >= this->_multielectrodeNr ) {
            // powrot cyklicznej ...
            this->getMesCurves()->get(nNrOfMesCurve)->addDataPoint(
                        this->CountResultPV(
                            (60L*(*getMesCurves()->get(nNrOfMesCurve)->getMesCurrent2Point(nPointFromDevice)
                                  - *getMesCurves()->get(nNrOfMesCurve)->getMesCurrent1Point(nPointFromDevice))
                             / *getMesCurves()->get(nNrOfMesCurve)->getMesTimePoint(nPointFromDevice))
                            )
                        , nPointFromDevice
                        );
        } else {
            this->getMesCurves()->get(nNrOfMesCurve)->addDataPoint(
                        this->CountResultPV(
                            (60L*(*getMesCurves()->get(nNrOfMesCurve)->getMesCurrent2Point(nPointFromDevice)
                                  - *getMesCurves()->get(nNrOfMesCurve)->getMesCurrent1Point(nPointFromDevice))
                             / *getMesCurves()->get(nNrOfMesCurve)->getMesTimePoint(nPointFromDevice))
                            )
                        , nPointFromDevice
                        );
        }
    } else {
        if ( this->_prepareEstart ) {
            /*
             * When LSV start from Es (not Ep), we need to fillin the fields in potential vector anyway
             */
            _prepareEstart = false;
            for ( int i = 0; i<(getMesCurves()->get(0)->Param(PARAM::ptnr) - this->_ptnrFromEss); ++i ) {
                this->getMesCurves()->get(nNrOfMesCurve)->addDataPoint(
                            this->CountResultLSV(
                                (60L * *getMesCurves()->get(nNrOfMesCurve)->getMesCurrent1Point(nPointFromDevice)
                                 / *getMesCurves()->get(nNrOfMesCurve)->getMesTimePoint(nPointFromDevice))
                                )
                            , i
                            );
            }
        }
        if ( nNrOfMesCurve % 2 == 0 ) {
            this->getMesCurves()->get(nNrOfMesCurve)->addDataPoint(
                        this->CountResultLSV(
                            (60L * *getMesCurves()->get(nNrOfMesCurve)->getMesCurrent1Point(nPointFromDevice)
                             / *getMesCurves()->get(nNrOfMesCurve)->getMesTimePoint(nPointFromDevice))
                            )
                        , nPointFromDevice
                        );
        } else {
            this->getMesCurves()->get(nNrOfMesCurve)->addDataPoint(
                        this->CountResultLSV(
                            (60L * *getMesCurves()->get(nNrOfMesCurve)->getMesCurrent1Point(nPointFromDevice)
                             / *getMesCurves()->get(nNrOfMesCurve)->getMesTimePoint(nPointFromDevice))
                            )
                        , nPointFromDevice
                        );
        }
    }

    this->_pUI->MeasurementUpdate();
}

/*
 *  Calculate final result for LSV
 */
double EAQtData::CountResultLSV(long ResI)
{
    static double dResI;

    ResI -= this->_IUE0;

    if ( getMesCurves()->get(0)->Param(PARAM::electr) < PARAM::electr_micro ) { //MACRO ELEKTRODA
        dResI = (double)ResI * MEASUREMENT::scale_micro[getMesCurves()->get(0)->Param(PARAM::crange)] * MEASUREMENT::multiply;
    } else {
        dResI = (double)ResI * MEASUREMENT::scale_micro[getMesCurves()->get(0)->Param(PARAM::crange)] * MEASUREMENT::multiply;
    }

    return dResI;
}

/*
 * Calculate final result for PV
 */
double EAQtData::CountResultPV(long ResI)
{
    static double dResI;

    if (getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_sqw ) {
        if (getMesCurves()->get(0)->Param(PARAM::sampl) == PARAM::sampl_double) {
            ResI -= this->_IUE0;
        }
    } else { // not SQW
        if (getMesCurves()->get(0)->Param(PARAM::sampl) == PARAM::sampl_single) {
            ResI -= this->_IUE0;
        }
    }

    if ( getMesCurves()->get(0)->Param(PARAM::electr) < PARAM::electr_micro ) { //MACRO ELEKTRODA
        dResI = (double)ResI * MEASUREMENT::scale_micro[getMesCurves()->get(0)->Param(PARAM::crange)] * MEASUREMENT::multiply;
    } else {
        dResI = (double)ResI * MEASUREMENT::scale_micro[getMesCurves()->get(0)->Param(PARAM::crange)] * MEASUREMENT::multiply;
    }

    return dResI;
}

/*
 * E start is applied, we need to display it
 */
void EAQtData::updateELSV()
{
    _pUI->setLowLabelText(0,tr("Applying Estart: %1 mV, %2 s").arg(_mesCurves->get(0)->Param(PARAM::EstartLSV)).arg(_EstartCurrentTime));
    return;
}

/*
 * Pause (break) is applied, we need to display it
 */
void EAQtData::updatePause()
{
    _pUI->setLowLabelText(0,tr("Break: %1 mV  %2:%3 of %3:%4 ").arg(_break.currentE).arg(_break.currentMin).arg(_break.currentSec).arg(_break.targetMin).arg(_break.targetSec));
    return;
}

/*
 * CGMDE test is performed, we need to display it
 */
void EAQtData::updateTestCGMDE()
{
    //TODO dialog window
    //_pUI->setLowLabelText(0,tr("CGMDE test: %1").arg(_testCGMDE));
    return;
}

/*
 * After the measurement is completed, we need to:
 * -ask if user want to save curves
 * -save them
 * -clear the plots from mes curves
 * -free mes curves
 */
void EAQtData::MesAfter()
{
    int work_act, err;

    _conductingMeasurement = 0;
    work_act = this->Act();
    // nie wiem czy trzeba: m_nStopInfo = 1;

    this->_pUI->MeasurementUpdate();

    if (this->_isMesSeries != true ) // to nie pomiar seryjny
    {
        if ( this->_pUI->showQuestionBox( tr("Save dialog"),
                                                 tr("Do you want to save the result?")) ) {
            err = -1;
            while (err < 0)
            {
                EAQtSaveFiledialog::SaveDetails saveDetails = this->_pUI->DialogSaveInFile();
                if ( saveDetails.wasCanceled ) {
                    break;
                }
                if ( (err = this->MesSaveAll(saveDetails.curveName, saveDetails.fileName, saveDetails.curveComment)) == 1 ) {
                    // anulowano zapis do pliku
                    this->MesClear();
                }
            }
        } else {
            this->MesClear();
        }
    }
    else // pomiar seryjny
    {
        err = -1;
        while (err < 0) {
            if ( this->_pSeriesData->getFileName() != ""
                 && (err = this->MesSaveAll(QString::fromStdString(this->_pSeriesData->getCurveName()),
                                            QString::fromStdString(this->_pSeriesData->getFileName()),
                                            QString::fromStdString(this->_pSeriesData->getCurveComment()))) == 1)
            {
                this->Act(work_act);
            }
        }
    }
    this->MesClear();

    /////// WYCZYSZCZENIE WYKRSU ///////
    this->_calculatedNrOfPoints = 0;
    this->_displayCurveNr = 0;
    this->_currentPointOnCurve = 0;
    this->_performSetup = true;
    /////// WYCZYSZCZENIE WYKRSU ///////

    this->_pUI->updateAll();

    if (this->_isMesSeries == true )
    {
        if ( this->_pSeriesData->Mes_Next() ) {
            this->MesStart();
            return;
        }
        //        Beep((DWORD)70,(DWORD)2000);
        //        PlaySound("C:\\EALab\\tada.wav", 0, SND_FILENAME|SND_LOOP|SND_ASYNC);
        //        Sleep(2000);
        //        PlaySound(0,0,0);
        delete _pSeriesData;
        _pSeriesData = new MesCFG();
        _isMesSeries = false;
    }

    this->_pUI->MeasurementAfter();
    return;
}

/*
 * Clears the measurement curves from memory
 */
void EAQtData::MesClear()
{
    this->getMesCurves()->clear();
    this->_pUI->updateAll();
}

/*
 * Save all registered curves
 */
int EAQtData::MesSaveAll(QString UserCName, QString UserFName, QString UserComment)
{
    //Zachowuje wszystkie zmierzone krzywe po zakończonym pomiarze
    int err;

    if ( UserCName.isEmpty() ) {
        UserCName = "USER";
    }

    if ( UserFName.right(5).compare(".volt",Qt::CaseInsensitive) != 0 ) {
        UserFName += ".volt";
    }

    /*
    *
    */
    QString orderedPrefixes[8];
    int cntElectrodes = 0;
    if ( this->_PVParam[PARAM::electr] == PARAM::electr_multi ) {
        uint32_t work;
        work = this->_PVParam[PARAM::multi];
        for (int i=0 ; i<8 ; i++) {
            if ((work & 0x00000080) != 0) {
                orderedPrefixes[cntElectrodes++]= _vChannelNamesOfMultielectrode[i];
                this->_multielectrodeNr++;
            }
            work = work << 1;
        }
    } else {
        cntElectrodes = 1;
    }


    int num = 0;
    while ( this->getMesCurves()->get(num) != NULL ) {
        this->getMesCurves()->get(num)->setDate();
        if ( this->getMesCurves()->get(num)->Param(PARAM::electr) == PARAM::electr_multi ) {
            // wieloelektrodowy
            if ( num >= cntElectrodes) {
                // Dla cyklicznych ustawiane sa wartości równocześnie
                break;
            }
            if ( this->getMesCurves()->get(num)->Param(PARAM::messc) == PARAM::messc_single ) { // pojedycza
                // wieloelektrodowy pojedynczy
                this->getMesCurves()->get(num)->CName(orderedPrefixes[num] + ":" + UserCName);
                this->getMesCurves()->get(num)->FName(UserFName);
                this->getMesCurves()->get(num)->Comment(UserComment);
            } else { // cykliczna (wielocykliczna)
                this->getMesCurves()->get(num)->CName(orderedPrefixes[num] + ":" + UserCName);
                this->getMesCurves()->get(num)->FName(UserFName);
                this->getMesCurves()->get(num)->Comment(UserComment);
                this->getMesCurves()->get(num+cntElectrodes)->CName("r" + orderedPrefixes[num] + ":" + UserCName);
                this->getMesCurves()->get(num+cntElectrodes)->FName(UserFName);
                this->getMesCurves()->get(num+cntElectrodes)->Comment(UserComment);
                this->getMesCurves()->get(num+cntElectrodes)->setDate();
            }
            num++;
        } else if ( getMesCurves()->get(num)->Param(PARAM::messc) == PARAM::messc_multicyclic ) {
            this->getMesCurves()->get(num)->CName(UserCName);
            this->getMesCurves()->get(num)->FName(UserFName);
            this->getMesCurves()->get(num)->Comment(UserComment);
            this->getMesCurves()->get(num+1)->CName("r" + UserCName);
            this->getMesCurves()->get(num+1)->FName(UserFName);
            this->getMesCurves()->get(num+1)->Comment(UserComment);
            this->getMesCurves()->get(num+1)->setDate();
            num+=2;
        } else {
            if ( num >= cntElectrodes ) {
                // Dla cyklicznych ustawiane sa wartości równocześnie z pojedynczymi
                break;
            }
            if ( getMesCurves()->get(num)->Param(PARAM::messc) == PARAM::messc_single ) { // pojedycza
                this->getMesCurves()->get(num)->CName(UserCName);
                this->getMesCurves()->get(num)->FName(UserFName);
                this->getMesCurves()->get(num)->Comment(UserComment);
            } else { // cykliczna
                this->getMesCurves()->get(num)->CName(UserCName);
                this->getMesCurves()->get(num)->FName(UserFName);
                this->getMesCurves()->get(num)->Comment(UserComment);
                this->getMesCurves()->get(num+1)->CName("r"+UserCName);
                this->getMesCurves()->get(num+1)->FName(UserFName);
                this->getMesCurves()->get(num+1)->Comment(UserComment);
                this->getMesCurves()->get(num+1)->setDate();
            }
            num++;
        }
    }

    num=0;
    while ( getMesCurves()->get(num) != NULL ) {
        if ( (err = SafeAppend(getMesCurves()->get(num)->FName(), getMesCurves()->get(num))) < 0 ) {
            //AfxMessageBox(IDS_info6, MB_OK);
            return -1;
        }
        num++;
    }


    while ( getMesCurves()->get(0) != NULL ) {
        int index = this->getCurves()->append(this->getMesCurves()->get(0));
        this->getMesCurves()->unset((uint32_t)0);
        this->getCurves()->get(index)->changeToRegularPlot();
        this->Act(index);
    }
    this->MesClear();
    this->_pUI->updateAll();
    return 0;
}

/*
 * Append Curve structure to file, this does some validation if operation was successfull
 * (we cannot lose any measurement data)
 */
int EAQtData::SafeAppend(QString pFileName, Curve* CurveToAppend)
{
    /// 0 - All OK
    /// -1 - Destination file Inaccessible or corrupted
    /// -2 - Destination file larger than 2MB
    /// -3 - Unable to create/read temp file
    /// -4 - Too many Curves in the file
    /// -5 - memory allocation error
    /// -6 - tmp file corrupted
    /// -7 - could not replace destination file
    /// -8 - wrong curve name

    char *buffer;
    char *curveBuffer;
    char *validTemp;
    unsigned long fileLen;
    uint32_t nCurveCntr; // licznik krzywych
    int const maxFileSizeBytes = 1024*1024*1024;

    QFile *file = new QFile(pFileName);


    if ( !file->exists() )
    {
        if( file->open(QIODevice::ReadWrite) ) // jezeli nie ma proba utworzenia
        {
            nCurveCntr = 0;
            file->write((char*)&nCurveCntr,sizeof(uint32_t));
            file->close();
            if ( !file->open(QIODevice::ReadOnly) ) {
                this->_pUI->showMessageBox("IDS_info6");
                return(-1);
            }
        } else {
            this->_pUI->showMessageBox("IDS_info6");
            return(-1); // brak dostepu do pliku
        }
    } else {
        if ( !file->open(QIODevice::ReadOnly) ) {
            this->_pUI->showMessageBox("IDS_info6");
            return(-1);
        }
    }

    //Get file length
    file->seek(0);
    fileLen=file->size();

    // Sprawdzanie czy plik nie jest za duzy do zaladowania do pamieci lub pusty
    if ( fileLen > maxFileSizeBytes  ||  fileLen < 4 )
    {
        this->_pUI->showMessageBox("IDS_info6");
        file->close();
        return(-2);
    }

    QByteArray byteArray = file->readAll();
    buffer = byteArray.data();
    file->close();

    // Verify the number of curves in file
    uint32_t *CurvInFile;
    CurvInFile=(uint32_t*)buffer;

    int a=4;

    QString* CurveNames = new QString[*CurvInFile];
    char appNumber=0;
    int nComperator=0;
    int CurvInFileLen;


    for (uint i=0;i<*CurvInFile;i++)
    {
        // a - start of curve
        CurvInFileLen=*(uint32_t*)(buffer+a); // length of curve with bytes representing the length

        int o=0;
        while ((char)*(buffer+a+4+o) != '\0') // name of curve -- +4 bytes of its size
        {
            CurveNames[i].append((char)*(buffer+a+4+o));
            o++;
        }

        if( CurveNames[i].compare(CurveToAppend->CName(),Qt::CaseSensitive) == 0 ) // does curve exists in file
            nComperator = nComperator + 1;  //if does increment its name

        a += CurvInFileLen;
    }

    QString appCurveName = "";
    if( nComperator == 0 )
    {
        appCurveName = CurveToAppend->CName();
    } else {
        short SafetySwitch = 0;
        while(true) //change the curve name
        {
            char bufo[2];
            nComperator=0;
            appCurveName = CurveToAppend->CName();
            sprintf(bufo, "%02d", appNumber++);

            appCurveName.append(bufo[0]);
            appCurveName.append(bufo[1]);

            for ( uint p=0;p<*CurvInFile;p++ )
            {
                if( CurveNames[p].compare(appCurveName,Qt::CaseSensitive) == 0 )
                    nComperator = nComperator + 1;
            }

            if( nComperator==0 )
            {
                break;
            }

            if ( SafetySwitch > 98 )
            {
                byteArray.clear();
                this->_pUI->showMessageBox("IDS_info6");
                return(-8);
            }
            SafetySwitch++;
        }
    }

    *CurvInFile=*CurvInFile+1;	// number of curves +1 (new one) -- 4 bytes

    char* pI = (char*)CurvInFile;
    *(buffer)=*pI;
    pI++;
    *(buffer+1)=*pI;
    pI++;
    *(buffer+2)=*pI;
    pI++;
    *(buffer+3)=*pI;

    int addLen, addLen1;
    addLen = 4 + appCurveName.size() + 1 + CurveToAppend->Comment().size() + 1;
    curveBuffer = (char *)malloc(addLen);
    // number of params + params + time vecotr + potential vecotr + current vecotr
    addLen1 = addLen + 4 + PARAM::PARAMNUM*sizeof(int32_t) + 3*CurveToAppend->getNrOfDataPoints()*sizeof(double);
    if ( CurveToAppend->Param(PARAM::nonaveragedsampling) != 0 ) {
        // nonaveraged length
        addLen1 += CurveToAppend->getNumberOfProbingPoints()*sizeof(float)+sizeof(uint32_t); //+zapisanie ile tych punktow jest
    }

    char *PointInBuffer;

    PointInBuffer=curveBuffer;

    char* pI2 = (char*)&addLen1; // 4 bajty - długość krzywej
    for (uint iii=0; iii<sizeof(int32_t); iii++)
    {
        *(PointInBuffer)=*pI2;
        pI2++;
        PointInBuffer++;
    }

    for (int ii=0;ii<appCurveName.toLocal8Bit().size();ii++)
    {
        *(PointInBuffer)=appCurveName.toLocal8Bit()[ii];
        PointInBuffer++;
    }
    *(PointInBuffer)='\0';
    PointInBuffer++;

    for (int ii=0;ii<CurveToAppend->Comment().toLocal8Bit().size();ii++)
    {
        *(PointInBuffer)=CurveToAppend->Comment().toLocal8Bit()[ii];
        PointInBuffer++;
    }
    *(PointInBuffer)='\0';
    PointInBuffer++;

    QString TempFileName = pFileName + ".tmp";

    QFile *outFile = new QFile(TempFileName);
    if ( outFile->open(QIODevice::ReadWrite) )
    {
        outFile->write(buffer,fileLen);
        outFile->write(curveBuffer,addLen);

        int32_t nParNr=PARAM::PARAMNUM;
        outFile->write((char*)&nParNr,sizeof(int32_t));
        for (short ii=0;ii<PARAM::PARAMNUM;ii++) {
            int nParVal=CurveToAppend->Param(ii);
            outFile->write((char*)&nParVal,sizeof(int32_t));
        }

        double dAux;
        for (uint ii=0; ii<CurveToAppend->getNrOfDataPoints(); ii++) {
            dAux = CurveToAppend->getTimePoint(ii);       // time
            outFile->write((char*)&dAux,sizeof(double));
            dAux = CurveToAppend->getPotentialPoint(ii);   // potential
            outFile->write((char*)&dAux,sizeof(double));
            dAux=CurveToAppend->Result(ii);
            outFile->write((char*)&dAux,sizeof(double));   // current

        }
        if ( CurveToAppend->Param(PARAM::nonaveragedsampling) != 0 )  {
            // nonaveraged:
            uint32_t vectorSize = CurveToAppend->getNumberOfProbingPoints();
            vector<float> vFloat(CurveToAppend->getProbingData()->begin(), CurveToAppend->getProbingData()->end());
            outFile->write((char*)&vectorSize,sizeof(uint32_t));
            outFile->write((char*)&vFloat[0],vectorSize*sizeof(float));
        }
        outFile->close();
    } else {
        byteArray.clear();
        free(curveBuffer);
        this->_pUI->showMessageBox("IDS_info6");
        return(-3);
    }

    byteArray.clear();
    free(curveBuffer);
    PointInBuffer=NULL;

    ////////////////////////////////////////////////////////////////////////////
    //////////////////// walidacja pliku TEMP //////////////////////////////////

    unsigned long tmpFileLen;

    if( !outFile->open(QIODevice::ReadOnly) ) {
        this->_pUI->showMessageBox("IDS_info6");
        return(-6);
    }

    outFile->seek(0);
    tmpFileLen=outFile->size();

    if ( tmpFileLen > maxFileSizeBytes  ||  tmpFileLen < 240 )
    {
        outFile->close();
        this->_pUI->showMessageBox("IDS_info6");
        return(-6);
    }

    QByteArray qba2 = outFile->readAll();
    validTemp = qba2.data();
    outFile->close();

    uint32_t CurvsInTmpFile;
    CurvsInTmpFile=*(uint32_t*)validTemp;

    if ( CurvsInTmpFile<1 )
    {
        qba2.clear();
        this->_pUI->showMessageBox("IDS_info6");
        return(-6);
    }

    unsigned long nOffsetWholeSum = sizeof(int32_t);

    for(uint II=0;II<CurvsInTmpFile;II++)
    {
        uint32_t* tmpCurOffset;
        tmpCurOffset=(uint32_t*)(validTemp+nOffsetWholeSum);
        nOffsetWholeSum+=*tmpCurOffset;
        if ( tmpFileLen < nOffsetWholeSum ) {
            qba2.clear();
            this->_pUI->showMessageBox("IDS_info6");
            return (-6);
        }
    }


    if( nOffsetWholeSum<=4 || (tmpFileLen!=nOffsetWholeSum) ) // verify offsets and file size
    {
        qba2.clear();
        this->_pUI->showMessageBox("IDS_info6");
        return(-6);
    }

    qba2.clear();

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    if ( !file->remove() ) {
        this->_pUI->showMessageBox("IDS_info6");
        return(-7);
    }

    if ( !outFile->rename(pFileName) ) {
        this->_pUI->showMessageBox("IDS_info6");
        return(-7);
    }

    CurveToAppend->CName( appCurveName );
    CurveToAppend->FName( pFileName );

    return 0;
}


void EAQtData::loadMesFile()
{
    int res = -1;
    try {
        res = this->_pSeriesData->LoadFromFile(this->_seriesFilePath.toStdString());
    } catch (int a) {
        this->_pUI->showMessageBox(tr("error loading file: ") + this->dispNR(a));
    }

    if ( res == -1 ) {
        QString b = this->_pSeriesData->lasterroron.c_str();
        this->_pUI->showMessageBox( "IDS_info10" + b );
        this->_isMesSeries = false;
        return;
    } else if ( res == -2 ) {
        this->_pUI->showMessageBox("IDS_info25");
        this->_isMesSeries = false;
        return;
    } else if ( res == -3 ) {
        this->_pUI->showMessageBox("IDS_info4");
        this->_isMesSeries = false;
        return;
    } else if ( res == 0 ) {
        this->_pUI->showMessageBox("IDS_info16"); // Wczytano 0 krzywych, nie kontynuuję
        this->_isMesSeries = false;
        return;
    } else if ( res > 0 ) {

        //this->uiInterface->showMessageBox("IDS_info17"); // Wczytano
        //this->uiInterface->showMessageBox("IDS_info18"); // serii krzywych, rozpoczynam serię pomiarów

        this->_pUI->showMessageBox("IDS_info17" + this->dispNR(res) + "IDS_info18");
        this->_isMesSeries = true;
    } else {
        this->_pUI->showMessageBox("IDS_info19"); // Błąd krytyczny, nie kontynuuję
        this->_isMesSeries = false;
        return;
    }
}

/*
 * When there is delay between measurements is MeasurementSeries, this should be done
 */
void EAQtData::SeriaWait(uint32_t delay)
{
    QTime dieTime= QTime::currentTime().addSecs(delay*1000);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    return;
}

/*
 * Display information of any network errors (this is connected with this->socket)
 */
void EAQtData::NetworkError(QString text)
{
    this->_pUI->showMessageBox(text);
}

int32_t EAQtData::isGasOn()
{
    return this->_accessories.gas_on;
}

void EAQtData::setGas(int32_t v)
{
    this->_accessories.gas_on = v;
}

int32_t EAQtData::isMixerOn()
{
    return this->_accessories.stirrer_on;
}

void EAQtData::setMixer(int32_t v)
{
    this->_accessories.stirrer_on = v;
}

/*
 * When user want accessories state to change (stirrer, deaeration, etc.)
 * poll for settings and send to the analyzer
 */
void EAQtData::sendAccessories()
{
    if ( !this->_network->connectToEA() ) {
        return;
    }

    int work;
    int32_t TxN;

    TxN = 0;
    _TxBuf[0] = PC2EA_RECORODS::recordAcces;
    TxN = 1;

    // stirrer
    work = _PVParam[PARAM::sti];
    _TxBuf[TxN] = (unsigned char)(work & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((work >> 8) & 0x00ff); TxN++;

    // knock
    work = _PVParam[PARAM::kp];
    _TxBuf[TxN] = (unsigned char)(work & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((work >> 8) & 0x00ff); TxN++;

    // knock - time
    work = _PVParam[PARAM::kpt];
    _TxBuf[TxN] = (unsigned char)(work & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((work >> 8) & 0x00ff); TxN++;

    // odtlenianie
    _TxBuf[TxN] = _accessories.gas_on; TxN++;

    // mieszanie
    _TxBuf[TxN] = _accessories.stirrer_on; TxN++;

    // test mlotka
    _TxBuf[TxN] = _accessories.test_knock; TxN++;
    _accessories.test_knock = 0;

    // test zaworu 10 ms
    _TxBuf[TxN] = _accessories.test_valve; TxN++;
    _accessories.test_valve = 0;

    this->_network->sendToEA(&_TxBuf[0], TxN);

    return;
}

void EAQtData::setTestHammer(bool v)
{
    this->_accessories.test_knock = v;
}

void EAQtData::setTestCGMDEValve(bool v)
{
    this->_accessories.test_valve = v;
}

int EAQtData::getXAxis()
{
    return this->_xaxis_type;
}

void EAQtData::setXAxis(int newtype)
{
    this->_xaxis_type = newtype;
    this->_pUI->updateAll();
}

QString EAQtData::dispE(int nNumber) {
    if ( nNumber == 0 ) {
        return tr("%1 mV").arg(0.0, 0,'f', 1);
    }
    return tr("%1 mV").arg((double)nNumber, 0,'f', 1);
}

QString EAQtData::dispE(double nNumber) {
    if ( nNumber == 0 ) {
        return "0.0";
    }
    return tr("%1 mV").arg(nNumber,0,'f',1);
}

QString EAQtData::dispEforTXT(double nNumber) {
    if ( nNumber == 0 ) {
        return "0.0";
    }
    return tr("%1").arg(nNumber,0,'f',1);
}

QString EAQtData::dispI(double dNumber) {
    QString ret;
    switch (this->_currentRange) {
    case PARAM::crange_micro_5uA + PARAM::crange_macro_100nA + 1:
        ret = tr("%1 µA").arg(dNumber,0,'f', 4);
        break;
    case PARAM::crange_micro_500nA + PARAM::crange_macro_100nA + 1:
        ret = tr("%1 nA").arg(dNumber*1000,0,'f', 2);
        break;
    case PARAM::crange_micro_50nA + PARAM::crange_macro_100nA + 1:
        ret = tr("%1 nA").arg(dNumber*1000,0,'f', 3);
        break;
    case PARAM::crange_micro_5nA + PARAM::crange_macro_100nA + 1:
        ret = tr("%1 nA").arg(dNumber*1000,0,'f', 4);
        break;
    case PARAM::crange_macro_100mA:
        ret = tr("%1 mA").arg(dNumber/1000,0,'f', 2);
        break;
    case PARAM::crange_macro_10mA:
        ret = tr("%1 mA").arg(dNumber/1000,0,'f', 3);
        break;
    case PARAM::crange_macro_1mA:
        ret = tr("%1 mA").arg(dNumber/1000,0,'f', 4);
        break;
    case PARAM::crange_macro_100uA:
        ret = tr("%1 µA").arg(dNumber,0,'f', 2);
        break;
    case PARAM::crange_macro_10uA:
        ret = tr("%1 µA").arg(dNumber,0,'f', 3);
        break;
    case PARAM::crange_macro_1uA:
        ret = tr("%1 µA").arg(dNumber,0,'f', 4);
        break;
    case PARAM::crange_macro_100nA:
        ret = tr("%1 nA").arg(dNumber*1000,0,'f', 2);
        break;
    default:
        ret = tr("%1 µA").arg(dNumber,0,'f', 4);
        break;
    }
    return ret;
}

QString EAQtData::dispIforTXT(double dNumber) {
    static QString ret;
    switch (this->_currentRange) {
    case PARAM::crange_micro_5uA + PARAM::crange_macro_100nA + 1:
        ret = QString("%1").arg(dNumber,0,'f', 4);
        break;
    case PARAM::crange_micro_500nA + PARAM::crange_macro_100nA + 1:
        ret = QString("%1").arg(dNumber*1000,0,'f', 2);
        break;
    case PARAM::crange_micro_50nA + PARAM::crange_macro_100nA + 1:
        ret = QString("%1").arg(dNumber*1000,0,'f', 3);
        break;
    case PARAM::crange_micro_5nA + PARAM::crange_macro_100nA + 1:
        ret = QString("%1").arg(dNumber*1000,0,'f', 4);
        break;
    case PARAM::crange_macro_100mA:
        ret = QString("%1").arg(dNumber/1000,0,'f', 2);
        break;
    case PARAM::crange_macro_10mA:
        ret = QString("%1").arg(dNumber/1000,0,'f', 3);
        break;
    case PARAM::crange_macro_1mA:
        ret = QString("%1").arg(dNumber/1000,0,'f', 4);
        break;
    case PARAM::crange_macro_100uA:
        ret = QString("%1").arg(dNumber,0,'f', 2);
        break;
    case PARAM::crange_macro_10uA:
        ret = QString("%1").arg(dNumber,0,'f', 3);
        break;
    case PARAM::crange_macro_1uA:
        ret = QString("%1").arg(dNumber,0,'f', 4);
        break;
    case PARAM::crange_macro_100nA:
        ret = QString("%1").arg(dNumber*1000,0,'f', 2);
        break;
    default:
        ret = QString("%1").arg(dNumber,0,'f', 4);
        break;
    }
    return ret;
}


QString EAQtData::dispNR(int nNumber) {
    if ( nNumber == 0 )
        return "0";
    return QString("%1").arg(nNumber);
}

QString EAQtData::dispTIME(int nNumber) {
    return QString("%1").arg(nNumber, 2, 10, QChar('0'));
}

QString EAQtData::dispTIME1(int nNumber) {
    return QString("%1 ms").arg(nNumber);
}

QString EAQtData::dispTIME1(double dNumber) {
    return QString("%1 ms").arg(static_cast<int>(dNumber));
}

QString EAQtData::dispTIMEforTXT(double dNumber) {
    return QString("%1").arg(dNumber,0,'f',2);
}

QString EAQtData::dispMATH(double dNumber) {
    if ( dNumber == 0 )
        return "0";
    double dLog;
    dLog= floor(log10(abs(dNumber)));
    int nDispAferComma;
    dLog+=-2;
    if ( dLog > 0 )
        nDispAferComma=0;
    else if ( dLog < -6 )
        nDispAferComma=6;
    else
        nDispAferComma=static_cast<int>(abs(dLog));
    return QString("%1").arg(dNumber,0,'f',nDispAferComma);
}

void EAQtData::SetCurrentRange(int nNewRange, int nElek) {
    int add;
    if ( nElek >= PARAM::electr_micro ) {
        add = PARAM::crange_macro_100nA + 1;
    } else {
        add = 0;
    }
    this->_currentRange = nNewRange + add;
}

int EAQtData::GetCurrentRange()
{
    return _currentRange;
}

QVector<QString> EAQtData::getChannelsNames()
{
    return _vChannelNamesOfMultielectrode;
}

std::vector<bool> EAQtData::getChannelsEnabled()
{
    std::vector<bool> enabled;
    enabled.resize(_vChannelNamesOfMultielectrode.size());
    uint32_t multi = _PVParam[PARAM::multi];
    for ( uint i = 0 ; i<enabled.size(); ++i ) {
        enabled[i] = (bool)(multi & 1<<i);
    }
    return enabled;
}

void EAQtData::setChannelsNames(QVector<QString> names)
{
    _vChannelNamesOfMultielectrode = names;
}

void EAQtData::setChannelsEnabled(std::vector<bool> enabled)
{
    uint32_t multi = 0;
    for ( uint i = 0 ; i<enabled.size(); ++i ) {
        multi |= ( (uint32_t)enabled[i]<<i);
    }
    _PVParam[PARAM::multi] = multi;
}

bool EAQtData::getIsMesSeries()
{
   return _isMesSeries;
}

void EAQtData::setIsMesSeries(bool is)
{
    _isMesSeries = is;
}

QString EAQtData::getMesSeriesFile()
{
    return _seriesFilePath;
}

void EAQtData::setMesSeriesFile(QString fp)
{
    _seriesFilePath = fp;
}

EAQtSignalProcessing* EAQtData::getProcessing()
{
    return _processing;
}

void EAQtData::exportToCSV(QString path)
{
    QFile *ff = new QFile(path);
    uint32_t i, k;
    int* blen;
    blen = new int[_curves->count()];
    int blenm;
    char* endOfLine = new char[2]{ '\r', '\n' };
    if (this->Act() != SELECT::all) {
        i = this->Act();
    } else {
        for (i = 0 ; i < _curves->count(); i++)
            if (_curves->get(i) != NULL)
                break;
    }
    if ( !ff->open(QIODevice::ReadWrite) ) {
        _pUI->showMessageBox(tr("Could not open file for writing."), tr("Error"));
    }

    std::string tmp;
    int pt;
    if (this->Act() == SELECT::all)  // wszystkie aktywne
    {

        QVector<double>* workE=_curves->get(0)->getPotentialVector();
        int savedCurrentRange = this->GetCurrentRange();

        for ( int i = 0; i<_curves->count(); ++i) {
            this->SetCurrentRange(_curves->get(i)->Param(PARAM::crange)
                                  , _curves->get(i)->Param(PARAM::electr));
            if ( this->getXAxis() == XAXIS::potential ) {
                for ( pt = 0; pt < _curves->get(i)->Param(PARAM::ptnr)-1; ++pt ) {
                    tmp = this->dispEforTXT(_curves->get(i)->getPotentialPoint(pt)).toStdString();
                    tmp.append(",");
                    ff->write(tmp.data(),tmp.size());
                }
                tmp = this->dispEforTXT(_curves->get(i)->getPotentialPoint(pt)).toStdString();
                ff->write(tmp.data(),tmp.size());
            } else {
                for ( pt = 0; pt < _curves->get(i)->Param(PARAM::ptnr)-1; ++pt ) {
                    tmp = this->dispTIMEforTXT(_curves->get(i)->getTimeVector()->at(pt)).toStdString();
                    tmp.append(",");
                    ff->write(tmp.data(),tmp.size());
                }
                tmp = this->dispTIMEforTXT(_curves->get(i)->getTimeVector()->at(pt)).toStdString();
                ff->write(tmp.data(),tmp.size());
            }
            ff->write(endOfLine,2);

            for ( pt = 0; pt < _curves->get(i)->Param(PARAM::ptnr)-1; ++pt ) {
                tmp = this->dispIforTXT(_curves->get(i)->Result(pt)).toStdString();
                tmp.append(",");
                ff->write(tmp.data(),tmp.size());
            }
            tmp = this->dispIforTXT(_curves->get(i)->Result(pt)).toStdString();
            ff->write(tmp.data(),tmp.size());
            ff->write(endOfLine,2);
        }
        this->SetCurrentRange(savedCurrentRange);
    } else { // jedna aktywna
        i=this->Act();
        if ( this->getXAxis() == XAXIS::potential ) {
            int pt = _curves->get(i)->Param(PARAM::ptnr);
            for ( pt = 0; pt<(_curves->get(i)->Param(PARAM::ptnr)-1); ++pt ) {
                tmp = this->dispEforTXT(_curves->get(i)->getPotentialPoint(pt)).toStdString();
                tmp.append(",");
                ff->write(tmp.data(),tmp.size());
            }
            tmp = this->dispEforTXT(_curves->get(i)->getPotentialPoint(pt)).toStdString();
            ff->write(tmp.data(),tmp.size());
        } else {
            for ( pt = 0; pt < (_curves->get(i)->Param(PARAM::ptnr)-1); ++pt ) {
                tmp = this->dispTIMEforTXT(_curves->get(i)->getTimeVector()->at(pt)).toStdString();
                tmp.append(",");
                ff->write(tmp.data(),tmp.size());
            }
            tmp = this->dispTIMEforTXT(_curves->get(i)->getTimeVector()->at(pt)).toStdString();
            ff->write(tmp.data(),tmp.size());
        }
        ff->write(endOfLine,2);

        for ( pt = 0; pt < _curves->get(i)->Param(PARAM::ptnr)-1; ++pt ) {
            tmp = this->dispIforTXT(_curves->get(i)->Result(pt)).toStdString();
            tmp.append(",");
            ff->write(tmp.data(),tmp.size());
        }
        tmp = this->dispIforTXT(_curves->get(i)->Result(pt)).toStdString();
        ff->write(tmp.data(),tmp.size());
        if ( i != _curves->count() -1 ) {
            // Ostatnia linia bez crlf
            ff->write(endOfLine,2);
        }
    }
    ff->close();
    delete ff;
}

void EAQtData::exportToTXT(QString path)
{
    uint32_t i, k;
    char buf[256];
    int* blen;
    blen = new int[_curves->count()];
    int blenm;

    QFile *ff = new QFile(path);
    if ( !ff->open(QIODevice::ReadWrite) ) {
        _pUI->showMessageBox(tr("Failed to open file for writing"),tr("Error"));
    }

    if (this->Act() == SELECT::all ) {
        for (i = 0 ; i < _curves->count() ; i++) {
            if ( _curves->get(i) != NULL ) {
                blen[i] = _curves->get(i)->Param(PARAM::ptnr);
            } else {
                blen[i] = 0;
            }
        }
        blenm = 0;
        for (i=0 ; i<_curves->count() ; i++) {
            if (blen[i] > blenm) {
                blenm = blen[i];
            }
        }
        QVector<double> *workE = _curves->get(0)->getPotentialVector();

        for (k=0 ; k<blenm ; k++) {
            sprintf(buf, "%10.5lf   ", workE->at(k));
            ff->write(buf, strlen(buf));
            for (i = 0 ; i < _curves->count() ; i++)
                if ( _curves->get(i) != NULL ) {
                    if (k < blen[i]) {
                        sprintf(buf,"%10.5lf   ", _curves->get(i)->Result(k));
                        ff->write(buf, strlen(buf));
                    } else {
                        sprintf(buf, "             ");
                        ff->write(buf, strlen(buf));
                    }
                }
            sprintf(buf, "\n");
            ff->write(buf, 1);
        }
    } else {
        i = this->Act();
        QVector<double> *workE = _curves->get(i)->getPotentialVector();
        for (k=0 ; k < _curves->get(i)->Param(PARAM::ptnr) ; k++)
        {
            sprintf(buf,"%10.5lf   %10.5lf\n", workE->at(k), _curves->get(i)->Result(k));
            ff->write(buf, strlen(buf));
        }
    }
    ff->close();
    delete[] blen;
    delete ff;
}
