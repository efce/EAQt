
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

//#include <QSound>
#include "eaqtdata.h"
#include "eaqtsavefiledialog.h"
#include "eaqtcalibrationdialog.h"

EAQtData::EAQtData() : QObject(), EAQtDataInterface()
{
    memset(_TxBuf, 0, NETWORK::TxBufLength*sizeof(char));
    memset(_RxBuf, 0, NETWORK::RxBufLength*sizeof(char));

    _IUE0 = 0;
    _act = SELECT::none;
    _wasLSVMeasurement = 0;
    _conductingMeasurement = 0;
    _performSetup = true;
    _multielectrodeNr = 0;
    _isMesSeries = false;
    _useSeriesFile = false;
    _fromUpdate.start();

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
    _prepareEstart = false;
    _currentRange = PARAM::crange_macro_1uA;
    _xaxis_type = XAXIS::potential;

    this->_calculatedNrOfPoints = 0;
    this->_displayCurveNr = 0;
    this->_currentPointOnCurve = 0;

    _pSeriesData = new MesCFG();

    initParam();
    initParamLSV();
}

void EAQtData::initialUpdate(EAQtUIInterface *wp)
{
    _pUI = wp;
    _curves = new CurveCollection(wp);
    _mesCurves = new CurveCollection(wp);
    _fileIndex = new MDirCollection();
    _network = new EAQtNetwork(this);
    _processing = new EAQtSignalProcessing(wp->PlotAddQCPCurve());
    _calibration = new CalibrationData();
}

EAQtData::~EAQtData()
{
    delete _fileIndex;
    delete _processing;
    delete _curves;
    delete _mesCurves;
    delete _network;
}

void EAQtData::initParam()
{
    // Parametry pomiaru
    for (int j=0 ; j<PARAM::PARAMNUM ; j++)
        _PVParam[j] = 0;

    _PVParam[PARAM::aver] = 1;
    _PVParam[PARAM::Ep] = -100;
    _PVParam[PARAM::Ek] = 100;
    _PVParam[PARAM::Estep] = 1;
    _PVParam[PARAM::tp] = 10;
    _PVParam[PARAM::tw] = 10;
    _PVParam[PARAM::tk] = 500;
    _PVParam[PARAM::ptnr] = 200;
    _PVParam[PARAM::pro] = 0;
    _PVParam[PARAM::multi] = 0;
    _PVParam[PARAM::method] = PARAM::method_scv;
    _PVParam[PARAM::electr] = PARAM::electr_solid;
    _PVParam[PARAM::el23] = PARAM::el23_dummy;
    _PVParam[PARAM::mespv] = PARAM::mespv_voltammetry;
    _PVParam[PARAM::sampl] = PARAM::sampl_single;
    _PVParam[PARAM::crange] = PARAM::crange_macro_10uA;
    _PVParam[PARAM::cgmdeMode] = 0;
    _PVParam[PARAM::valveCntr] = 5;
    _PVParam[PARAM::valveTime] = 10;
    _PVParam[PARAM::valveDelay] = 200;
    _PVParam[PARAM::nonaveragedsampling] = 1;

    // Akcesoria
    _PVParam[PARAM::stirrerSpeed] = 20;
    _PVParam[PARAM::knockPower] = 10;
    _PVParam[PARAM::knockTime] = 30;
}

void EAQtData::initParamLSV()
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
    _LSVParam[PARAM::valveCntr] = 5;
    _LSVParam[PARAM::valveTime] = 10;
    _LSVParam[PARAM::valveDelay] = 200;
    _LSVParam[PARAM::dEdt] = 3;
    _LSVParam[PARAM::EstartLSV] = 0;
    _LSVParam[PARAM::Escheck] = 0;
    _LSVParam[PARAM::ts] = 0;
    _LSVParam[PARAM::td] = 500;

    // Akcesoria
    _LSVParam[PARAM::stirrerSpeed] = 20;
    _LSVParam[PARAM::knockPower] = 10;
    _LSVParam[PARAM::knockTime] = 30;
}

void EAQtData::initEca()
{
    int32_t work;

    //E2I := Ep    NP
    //       Ep+dE DP
    E2I = this->getMesCurves()->get(0)->Param(PARAM::Ep);
    if (this->getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_dpv ) // DP
        E2I += this->getMesCurves()->get(0)->Param(PARAM::dE);
    if (this->getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_sqw_osteryoung ) // SQW
    {
        E3I = E2I - this->getMesCurves()->get(0)->Param(PARAM::dE);
        E2I += this->getMesCurves()->get(0)->Param(PARAM::dE);
    }

    work = this->getMesCurves()->get(0)->Param(PARAM::Ep);
    if (this->getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_npv)
        work = this->getMesCurves()->get(0)->Param(PARAM::E0); // NP
    E1I = work;
}

void EAQtData::initPtime()
{
    int64_t work;
    //Count tr = tk-i*(tw+tp)  i=1,2 (sampling)
    work = this->getMesCurves()->get(0)->Param(PARAM::tw) + this->getMesCurves()->get(0)->Param(PARAM::tp);
    if (this->getMesCurves()->get(0)->Param(PARAM::method) == PARAM::method_sqw_osteryoung ) // SQW
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
                (this->getMesCurves()->get(0)->Param(PARAM::cgmdeMode) >= 2)) ) {
            _singleStepTime = this->getMesCurves()->get(0)->Param(PARAM::td); // woltamper.
        }
    }

    if (_singleStepTime == 0)
        _singleStepTime = this->getMesCurves()->get(0)->Param(PARAM::tw);
    if (_singleStepTime == 0)
        _singleStepTime = this->getMesCurves()->get(0)->Param(PARAM::tp);
}

void EAQtData::createMatrix()
{
    int32_t mxptr;
    int32_t j;
    int32_t i = 0;


    for (j=0 ; j<20 ; j++)
        _measurementMatrix[j] = -1;
    mxptr = 1;
    // if voltamper. && td <> 0
    if ( (this->getMesCurves()->get(i)->Param(PARAM::mespv) == PARAM::mespv_voltammetry)
         &&   (this->getMesCurves()->get(i)->Param(PARAM::td) != 0) ) {
        // CGMDE and seinf == 2 - do not put PROC1
        if (!(((this->getMesCurves()->get(i)->Param(PARAM::electr) == PARAM::electr_cgmde ) ||  // CGMDE
               (this->getMesCurves()->get(i)->Param(PARAM::electr) == PARAM::electr_microCgmde )) &&  // mikroelektroda CGMDE
              (this->getMesCurves()->get(i)->Param(PARAM::cgmdeMode) >= 2))) {
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
                && (_trValue != 0)) {
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

void EAQtData::crmxSQW()
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

void EAQtData::prepareParamForSQWClassic(Curve* c)
{
    if ( c->Param(PARAM::method) != PARAM::method_sqw_classic ) {
        return;
    }
    _sqwClassicTemp.isSQWClassic = true;
    _sqwClassicTemp.method = c->Param(PARAM::method);
    _sqwClassicTemp.Ep = c->Param(PARAM::Ep);
    _sqwClassicTemp.Ek = c->Param(PARAM::Ek);
    _sqwClassicTemp.dE = c->Param(PARAM::dE);
    c->Param(PARAM::method, PARAM::method_dpv);
    c->Param(PARAM::Ep, (c->Param(PARAM::Ep) + c->Param(PARAM::dE)) );
    c->Param(PARAM::Ek, (c->Param(PARAM::Ek) + c->Param(PARAM::dE)) );
    c->Param(PARAM::dE, (-2*c->Param(PARAM::dE)) );
}

void EAQtData::recoverParamForSQWClassic(Curve* c)
{
    if ( _sqwClassicTemp.isSQWClassic == false ) {
        return;
    }
    c->Param(PARAM::method, _sqwClassicTemp.method);
    c->Param(PARAM::Ep, _sqwClassicTemp.Ep);
    c->Param(PARAM::Ek, _sqwClassicTemp.Ek);
    c->Param(PARAM::dE, _sqwClassicTemp.dE);
    _sqwClassicTemp.isSQWClassic = false;
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

int32_t EAQtData::ParamMain(int32_t num)
{
    return _mainParam[num];
}

void EAQtData::ParamMain(int32_t num, int32_t val)
{
    _mainParam[num] = val;
}

TYPES::vectorindex_t EAQtData::Act()
{
    return _act;
}

void EAQtData::Act(TYPES::vectorindex_t toAct)
{
    if ( getCurves()->count() > toAct
    || toAct == SELECT::all ) {
        _act = toAct;
        Curve* c = getCurves()->get(_act);
        if ( c != nullptr ) {
            setCurrentRange(c->Param(PARAM::crange), c->Param(PARAM::electr));
            if ( getXAxis() == XAXIS::potential ) {
                _pUI->PlotSetInverted( (c->Param(PARAM::Ek) < c->Param(PARAM::Ep)) );
            } else {
                _pUI->PlotSetInverted(false);
            }
        }
    } else {
        _act = SELECT::none;
    }
    emit actChanged(_act);
}

// --------------------------------------------------------
// MDirReadPro - czytaj nazwy krzywych z pliku volt
// --------------------------------------------------------
bool EAQtData::MDirReadPro(QFile &ff)
{
    uint32_t uiAux;
    TYPES::filesize_t iFilePos = 0;
    char cAux[1024];
    TYPES::vectorindex_t curvesInFile =0;

    ff.seek(0);
    ff.read((char*)(&curvesInFile), sizeof(TYPES::vectorindex_t)); // liczba krzywych w pliku

    iFilePos+=sizeof(TYPES::vectorindex_t);
    _fileIndex->clear();

    if (curvesInFile == 0) {
        return false;
    }

    for(TYPES::vectorindex_t i = 0;i < curvesInFile;++i) {
        ff.seek(iFilePos);
        TYPES::vectorindex_t index = _fileIndex->addNew();
        _fileIndex->get(index)->Off(iFilePos);         	// offset krzywej w pliku
        ff.read((char*)(&uiAux), sizeof(TYPES::curvefileindex_t)); // długość krzywej
        iFilePos += uiAux;
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

bool EAQtData::MDirReadOld(QFile &ff)
{
    int16_t iAux;
    char cAux[10];

    _fileIndex->clear();

    bool hascurves = false;
    ff.seek(0);
    ff.read((char*)&iAux, sizeof(int16_t));
    int16_t curvesInFiles = iAux;
    if ( curvesInFiles <= PARAM::VOL_CMAX ) {
        for(int16_t i = 0;i < curvesInFiles;++i) // nie zmieniać cmax (słownik vol)
        {
            ff.read(cAux, 10);
            ff.read((char*)&iAux, sizeof(int16_t));
            int32_t index = _fileIndex->addNew();
            _fileIndex->get(index)->Off(iAux);
            _fileIndex->get(index)->CName(QString(cAux));
            if ( !_fileIndex->get(index)->CName().isEmpty() ) {
                hascurves = true;
            }
        }
    }
    return hascurves;
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
        _pUI->showMessageBox(tr("File is empty"),tr("Error"));
        return;
    }

    if ( !MDirReadPro(ff) )
        return;

    if (PosNr == 0) {
        for (int32_t i=0 ; i<_fileIndex->count() ; ++i) {
            if ( (j = CurReadCurvePro(ff, _fileIndex->get(i)->CName())) < 0) {
                ff.close();
                _pUI->showMessageBox(tr("Error while reading the file"), tr("Error"));
                //AfxMessageBox(IDS_info4, MB_OK);
                return;  // niepoprawna nazwa krzywej
            } else if ( this->Act() != SELECT::all ) {
                this->Act(j);
            }
        }
    } else if ( (j = CurReadCurvePro(ff, _fileIndex->get(PosNr-1)->CName())) < 0 ) {
        ff.close();
        _pUI->showMessageBox(tr("Error while reading the file"), tr("Error"));
        return;  // niepoprawna nazwa krzywej
    } else if ( this->Act() != SELECT::all )  {
        this->Act(j);
    }

    ff.close();
    this->_pUI->updateAll();
    this->_pUI->PlotRescaleAxes(false);
}

void EAQtData::CurImportTxtFile(QString* FileName)
{
    QFile ff(*FileName);
    if( !ff.open(QIODevice::ReadOnly) ) {
        ff.close();
        _pUI->showMessageBox(tr("File is empty"),tr("Error"));
        return;
    }
    QVector<QVector<double>> curves;
    QByteArray line = ff.readLine();
    char separator;
    if ( line.contains(';') ) {
        separator = ';';
    } else if ( line.contains('\t') ) {
        separator = '\t';
    } else if ( line.contains(',') ) {
        separator = ',';
    } else {
        separator = ' ';
    }
    QList<QByteArray> splitline = line.split(separator);
    int nrcurves = splitline.size();
    curves.resize(nrcurves);
    for ( int i=0; i<nrcurves; ++i) {
        curves[i].append(QString(splitline[i]).toDouble());
    }

    while (!ff.atEnd()) {
        line = ff.readLine();
        splitline = line.split(separator);
        for ( int i=0; i<nrcurves; ++i) {
            curves[i].append(QString(splitline[i]).toDouble());
        }
    }
    ff.close();
    int ptnr = curves[0].size();
    QVector<double> incVec(ptnr);
    double step = curves[0][1] - curves[0][0];
    bool firstIsPotential = true;
    if (step != 0) {
        for ( int ii = 1; ii<ptnr; ++ii ) {
            if ( (curves[0][ii]-curves[0][ii-1]) != step ) {
                firstIsPotential = false;
                break;
            }
        }
    } else {
        firstIsPotential = false;
    }
    QVector<double> potential;
    if ( firstIsPotential && nrcurves > 1 ) {
        --nrcurves;
        potential = QVector<double>(curves[0]);
        curves.remove(0);
    }
    for ( int i = 0; i<nrcurves; ++i ) {
        TYPES::vectorindex_t index = this->_curves->addNew(ptnr);
        Curve* c = _curves->get(index);
        for ( int ii = 0; ii < ptnr; ++ii ) {
            c->addDataPoint(curves[i][ii]);
            incVec[ii] = ii;
        }
        if ( !firstIsPotential ) {
            c->setPotentialVector(incVec);
            c->setTimeVector(incVec);
            c->Param(PARAM::Ep, 0);
            c->Param(PARAM::Ek, ptnr);
            c->Param(PARAM::Estep, 1);
        } else {
            c->setPotentialVector(potential);
            c->setTimeVector(incVec);
            c->Param(PARAM::Ep, potential[0]);
            c->Param(PARAM::Ek, potential[0]+step*ptnr);
            c->Param(PARAM::Estep, step);
        }
        c->Param(PARAM::ptnr, ptnr);
        c->Param(PARAM::tp, 1);
        c->Param(PARAM::tw, 0);
        c->Param(PARAM::mespv, PARAM::mespv_voltammetry);
        c->Param(PARAM::crange, PARAM::crange_macro_1uA);
        //TODO: find the smallest difference between values and use it as a range ?
        c->CName(tr("#%1").arg(i));
        c->FName(*FileName);
    }

    this->_pUI->updateAll();
    this->_pUI->PlotRescaleAxes(false);
}

void EAQtData::CurReadFileOld(QString *FileName, int PosNr)
{
    QFile ff(*FileName);

    int j;

    if( !ff.open(QIODevice::ReadOnly) ) {
        ff.close();
        _pUI->showMessageBox(tr("File is empty"),tr("Error"));
        return;
    }

    if ( !MDirReadOld(ff) )
        return;

    if (PosNr == 0) {
        for (int32_t i=0 ; i<_fileIndex->count() ; ++i) {
            if ( (j = CurReadCurveOld(ff, _fileIndex->get(i)->CName())) < 0) {
                ff.close();
                _pUI->showMessageBox(tr("Error while reading the file"), tr("Error"));
                //AfxMessageBox(IDS_info4, MB_OK);
                return;  // niepoprawna nazwa krzywej
            } else if ( this->Act() != SELECT::all ) {
                this->Act(j);
            }
        }
    } else if ( (j = CurReadCurveOld(ff, _fileIndex->get(PosNr-1)->CName())) < 0 ) {
        ff.close();
        _pUI->showMessageBox(tr("Error while reading the file"), tr("Error"));
        return;  // niepoprawna nazwa krzywej
    } else if ( this->Act() != SELECT::all )  {
        this->Act(j);
    }

    ff.close();
    this->_pUI->updateAll();
    this->_pUI->PlotRescaleAxes(false);
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
    if ( _measurementGo ) {
        return 0;
    }
    TYPES::vectorindex_t j1;
    int32_t i;
    TYPES::curvefileindex_t cLen;

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

    try {
        j1 = getCurves()->addNew(1); // TMP nie znam ilosci punktow krzywej
    } catch (int e) {
        _pUI->showMessageBox(tr("Could not add new curve. Maximum reached?"), tr("Error"));
        return -1;
    }

    getCurves()->get(j1)->getPlot()->setLayer(_pUI->PlotGetLayers()->NonActive);
    ff.read((char*)(&cLen), sizeof(TYPES::curvefileindex_t));			// ilosc bajtow krzywej w pliku
    if (ff.fileName().right(FILES::saveCompressExt.size()).compare(FILES::saveCompressExt,Qt::CaseInsensitive) == 0 ) {
        QByteArray ba = ff.read(cLen);
        if ( !getCurves()->get(j1)->unserialize(ba,true) ) {
            getCurves()->remove(j1);
            this->_pUI->updateAll();
            return -1;
        }
    } else {
        QByteArray ba = ff.read(cLen);
        if ( !getCurves()->get(j1)->unserialize(ba,false) ) {
            getCurves()->remove(j1);
            this->_pUI->updateAll();
            return -1;
        }
    }
    this->setCurrentRange(this->getCurves()->get(j1)->Param(PARAM::crange),this->getCurves()->get(j1)->Param(PARAM::crange));
    this->getCurves()->get(j1)->FName(ff.fileName());
    return(j1);
}

int EAQtData::CurReadCurveOld(QFile &ff, QString CName)
{
    //TODO: na razie bez wsparcia potencjału startowego LSV ! //
    TYPES::vectorindex_t j1, j2;
    TYPES::vectorindex_t i;
    int16_t cntr;
    int16_t fparam, ix;
    int startadr, work;
    char buf[256];
    double dwork;

    MDirReadOld(ff);
    if (_fileIndex->count() == 0)
        return -1;

    for (i=0 ; i<_fileIndex->count() ; i++) {
        if (CName.compare(_fileIndex->get(i)->CName(),Qt::CaseSensitive) == 0) {
            break;
        }
    }
    if ( i == _fileIndex->count() ) {
        return -2;
    }

    // i numer krzywej
    ff.seek(_fileIndex->get(i)->Off());
    for (i=0 ; i<_fileIndex->count() ; ++i) {
        if (CName.compare(_fileIndex->get(i)->CName(),Qt::CaseSensitive) == 0) {
            break;
        }
    }
    if ( i == _fileIndex->count() ) {
        return -2;
    }

    ParamReadOld(ff);

    startadr = sizeof(int16_t) + PARAM::VOL_CMAX*12 + (PARAM::VOL_PMAX-2)*sizeof(int32_t); // nie zmieniać cmax i pmax (słownik vol)
    for (TYPES::vectorindex_t ii=0; ii<i; ii++) {
        startadr += _fileIndex->get(ii)->Off();
    }
    ff.seek(startadr);

    try {
        j1 = getCurves()->addNew(_mainParam[PARAM::ptnr]); // TMP nie znam ilosci punktow krzywej
    } catch (int e) {
        _pUI->showMessageBox(tr("Could not add new curve. Maximum reached?"), tr("Error"));
        return -1;
    }

    getCurves()->get(j1)->getPlot()->setLayer(_pUI->PlotGetLayers()->NonActive);

    for (int32_t ii=0 ; ii<(PARAM::VOL_PMAX-2) ; ii++) { // nie zmieniać pmax (słownik vol)
        getCurves()->get(j1)->Param(ii, _mainParam[ii]);
    }

    ff.read((char*)&cntr, sizeof(int16_t)*1);			// numer krzywej
    ff.read(buf, sizeof(char)*10);		// nazwa krzywej
    getCurves()->get(j1)->CName(QString(buf));
    ff.read(buf, sizeof(char)*50);	// komentarz
    getCurves()->get(j1)->Comment(QString(buf));
    ff.read((char*)&fparam, sizeof(int16_t)*1);		// liczba ró¿nych parametrów
    if (fparam>0) {
        for (i=0 ; i<fparam ; i++) {
            ff.read((char*)&ix, sizeof(int16_t)*1); // numer parametru
            ff.read((char*)&work,sizeof(int32_t)*1); // wartoœæ
            getCurves()->get(j1)->Param(ix, work);
        }
    }

    getCurves()->get(j1)->reinitializeCurveData(getCurves()->get(j1)->Param(PARAM::ptnr));
    getCurves()->get(j1)->Param(PARAM::multi, 0);
    getCurves()->get(j1)->Param(PARAM::nonaveragedsampling, 0);
    getCurves()->get(j1)->Param(PARAM::pro, 0);

    // wyniki
    QVector<double> vecT = this->generateTimeVector(getCurves()->get(j1)->Params(), 0);
    double potential = this->getCurves()->get(j1)->Param(PARAM::Ep);
    double estep = ( (double)this->getCurves()->get(j1)->Param(PARAM::Ek) - (double)this->getCurves()->get(j1)->Param(PARAM::Ep) ) / (double)this->getCurves()->get(j1)->Param(PARAM::ptnr);
    for (TYPES::vectorindex_t ii=0 ; ii<this->getCurves()->get(j1)->Param(PARAM::ptnr) ; ++ii) {
        ff.read((char*)&dwork, sizeof(double));
        this->getCurves()->get(j1)->addDataPoint(vecT[ii], potential, dwork);
        potential += estep;
    }
    this->setCurrentRange(getCurves()->get(j1)->Param(PARAM::crange),this->getCurves()->get(j1)->Param(PARAM::electr));
    this->getCurves()->get(j1)->FName(ff.fileName());
    if ( this->getCurves()->get(j1)->Param(PARAM::messc) >= PARAM::messc_cyclic ) { // krzywa cykliczna
        try {
            j2 = this->getCurves()->addNew(getCurves()->get(j1)->Param(PARAM::ptnr)); // TMP nie znam ilosci punktow krzywej
        } catch (int e) {
            _pUI->showMessageBox(tr("Could not add new curve. Maximum reached?"), tr("Error"));
            return -1;
        }
        this->getCurves()->get(j2)->CName(this->getCurves()->get(j1)->CName());
        this->getCurves()->get(j2)->Comment(this->getCurves()->get(j1)->Comment());
        this->getCurves()->get(j2)->FName(ff.fileName());

        for (i=0 ; i<(PARAM::VOL_PMAX-2) ; i++) { // nie zmieniać pmax (słownik vol)
            this->getCurves()->get(j2)->Param(i, this->getCurves()->get(j1)->Param(i));
        }

        double potential = this->getCurves()->get(j1)->Param(PARAM::Ep);
        vecT = this->generateTimeVector(getCurves()->get(j2)->Params(), 1);
        for (int ii=0 ; ii<this->getCurves()->get(j2)->Param(PARAM::ptnr) ; ++ii) {
            ff.read((char*)&dwork, sizeof(double));
            this->getCurves()->get(j2)->addDataPoint(vecT[ii], potential, dwork);
            potential +=estep;
        }
    }
    if (this->getCurves()->get(j1)->Param(PARAM::messc) >= PARAM::messc_cyclic) {
        return(2);
    } else {
        return(1);
    }
    return(1);
}

// --------------------------------------------------------
// Czytaj g³ówne parametry pomiaru z pliku ff
// --------------------------------------------------------
void EAQtData::ParamReadOld(QFile &ff)
{
    ff.seek(sizeof(int16_t) + PARAM::VOL_CMAX*12); // nie zmieniać cmax i pmax (słownik vol)
    ff.read((char*)_mainParam, sizeof(int32_t)*(PARAM::VOL_PMAX-2));
}

void EAQtData::openFile(QString *filePath, int nrPos)
{
    if ( filePath->right(4).compare(".txt", Qt::CaseInsensitive) == 0 ) {
        this->CurImportTxtFile(filePath);
    } else if ( filePath->right(4).compare(".vol", Qt::CaseInsensitive) == 0 ) {
        this->CurReadFileOld(filePath,nrPos);
    } else {
        this->CurReadFilePro(filePath, nrPos);
    }
}

void EAQtData::deleteActiveCurveFromGraph()
{
    if ( this->Act() == SELECT::all ) {
        this->undoPrepare();
        this->deleteAllCurvesFromGraph();
    } else if ( this->Act() >= 0 && this->getCurves()->get(this->Act()) != nullptr ) {
        this->undoPrepare();
        this->getCurves()->remove(this->Act());
        this->Act(SELECT::none);
        this->_pUI->updateAll();
    }
}

void EAQtData::deleteNonactiveCurvesFromGraph()
{
    if ( this->getCurves()->get(this->Act()) != nullptr ) {
        this->undoPrepare();
        Curve *c = this->getCurves()->get(this->Act());
        this->getCurves()->unset(this->Act());
        this->getCurves()->clear();
        int32_t index;
        try {
            index = this->getCurves()->append(c);
        } catch (int e) {
            _pUI->showMessageBox(tr("Could not reappend curve."), tr("Error"));
            Act(SELECT::none);
            this->_pUI->updateAll();
            return;
        }

        this->Act(index);
        this->_pUI->updateAll();
    }
}

void EAQtData::deleteAllCurvesFromGraph()
{
    if ( this->_pUI->showQuestionBox(
             tr("Delete all curves?"),
             tr("Are you sure"),
             false) ) {
        this->undoPrepare();
        this->getCurves()->clear();;
        this->Act(SELECT::none);
        this->_pUI->updateAll();
    }
}

void EAQtData::ProcessPacketFromEA(const char* packet)
{
    uint8_t* RxBuf = (uint8_t*)packet;

    int Cmd, i;
    int16_t work;
    int32_t workl;
    int32_t DataLen;
    static int32_t twCounter;
    static int32_t ActSampl1, ActSampl2;
    static int32_t previousPointNum, currentPointNum, previousCurveNum;
    Curve* mesCurve = nullptr;
    int32_t currentCurveNum = 0;

    Cmd = RxBuf[0];

    switch (Cmd) {
    case EA2PC_RECORDS::calibPV:
        _IUE0 = ((uint16_t)RxBuf[2] | ((uint16_t)RxBuf[3]<<8));
        previousPointNum = 0;
        previousCurveNum = 0;
        ActSampl1 = 0;
        ActSampl2 = 0;
        _ctnrSQW = 6;
        break;

    case EA2PC_RECORDS::calibLSV:
        _IUE0 = ((uint16_t)RxBuf[2] | ((uint16_t)RxBuf[3]<<8));
        previousPointNum = 0;
        previousCurveNum =0;
        break;

    case EA2PC_RECORDS::recordPV:
        this->_endOfMes = RxBuf[1];
        DataLen = ((uint16_t)RxBuf[2] | ((uint16_t)RxBuf[3]<<8))-8;
        currentPointNum = ((uint16_t)RxBuf[4] | ((uint16_t)RxBuf[5]<<8));
        currentCurveNum = ((uint16_t)RxBuf[6] | ((uint16_t)RxBuf[7]<<8));

        if ( this->_endOfMes ) {
            _measurementGo = 0;
        }

        if ( currentPointNum != previousPointNum
        || previousCurveNum != currentCurveNum ) {
            ActSampl1 = 0;
            ActSampl2 = 0;
            twCounter = 0;
            _ctnrSQW = 6;	// nr ms w 1. i 2. próbkowaniu
            this->MesUpdate(previousCurveNum, previousPointNum);
        }
        i = MEASUREMENT::PVstartData; // == 6
        mesCurve = this->getMesCurves()->get(currentCurveNum);

        if (mesCurve->Param(PARAM::method) != PARAM::method_sqw_osteryoung
        && mesCurve->Param(PARAM::method) != PARAM::method_lsv ) { // IMPULSOWE (nie SQW, nie LSV)
            while (DataLen > 0) {
                work = ((uint16_t)RxBuf[i] | ((uint16_t)RxBuf[i+1]<<8));
                workl = work;
                twCounter++;
                if (mesCurve->Param(PARAM::nonaveragedsampling) != 0 ) {
                    // pomiar idzie dla tp i tw, robimy cos innego
                    mesCurve->addProbingDataPoint(this->CountResultPV(workl));
                    if ( twCounter <= mesCurve->Param(PARAM::tw) ) {
                        // jestesmy w trakcie tw
                        i+= 2;
                        DataLen -=2 ;
                        continue;
                    }
                    if (mesCurve->Param(PARAM::sampl) == PARAM::sampl_double ) {
                        if (( twCounter <= 2*mesCurve->Param(PARAM::tw)+ mesCurve->Param(PARAM::tp))
                        && ( twCounter > mesCurve->Param(PARAM::tw)+ mesCurve->Param(PARAM::tp))) {
                            // jestesmy w trakcie tw
                            i+= 2;
                            DataLen -=2 ;
                            continue;
                        }
                    }
                    if ( (mesCurve->Param(PARAM::sampl) == PARAM::sampl_double)
                    &&   (ActSampl1 < mesCurve->Param(PARAM::tp)) ) {
                        mesCurve->addToMesCurrent1Point(currentPointNum, workl);
                        i+= 2;
                        ActSampl1 ++;
                    } else {
                        mesCurve->addToMesCurrent2Point(currentPointNum, workl);
                        i+= 2;
                        ActSampl2 ++;
                        mesCurve->addToMesTimePoint(currentPointNum, 1);
                    }
                } else {
                    if ( (mesCurve->Param(PARAM::sampl) == PARAM::sampl_double)
                    &&   (ActSampl1 < mesCurve->Param(PARAM::tp)) ) {
                        mesCurve->addToMesCurrent1Point(currentPointNum, workl);
                        i+= 2;
                        ActSampl1 ++;
                    } else {
                        mesCurve->addToMesCurrent2Point(currentPointNum,  workl);
                        i+= 2;
                        ActSampl2 ++;
                        mesCurve->addToMesTimePoint(currentPointNum, 1);
                    }
                }
                DataLen -= 2;
            }
        } else if (mesCurve->Param(PARAM::method) == PARAM::method_sqw_osteryoung )  {
            while (DataLen > 0) {
                work = ((uint16_t)RxBuf[i] | ((uint16_t)RxBuf[i+1]<<8));
                workl = work;
                if ( (mesCurve->Param(PARAM::sampl) == PARAM::sampl_double)
                &&   (ActSampl1 < mesCurve->Param(PARAM::tp))  ) {
                    mesCurve->addToMesCurrent1Point(currentPointNum,  workl);
                    i+= 2;
                    ActSampl1 ++;
                } else {
                    if ((_ctnrSQW == 6) || (_ctnrSQW == 4) || (_ctnrSQW == 2)) {
                        mesCurve->addToMesCurrent2Point(currentPointNum,  workl/3);
                    } else {
                        mesCurve->addToMesCurrent1Point(currentPointNum,  workl/3);
                    }
                    if (_ctnrSQW == 6) {
                        mesCurve->addToMesTimePoint(currentPointNum, 1);
                    }
                    i+= 2;
                    ActSampl2 ++;
                    if ( ActSampl2 == mesCurve->Param(PARAM::tp) ) {
                        ActSampl2 = 0;
                        _ctnrSQW--;
                    }
                }
                DataLen -= 2;
            }
        }

        previousPointNum = currentPointNum;
        previousCurveNum = currentCurveNum;

        if ( this->_endOfMes ) {
            this->MesAfter();
        }

        break;

    case EA2PC_RECORDS::recordLSV:
        this->_endOfMes = RxBuf[1];
        DataLen = ((uint16_t)RxBuf[2] | ((uint16_t)RxBuf[3]<<8))-8;
        static int32_t firstCycle = ((uint16_t)RxBuf[6] | ((uint16_t)RxBuf[7]<<8));
        if ( this->_endOfMes ) {
            _measurementGo = 0;
        }

        i = MEASUREMENT::LSVstartData; // == 8

        while ( DataLen > 0 ) {
            currentCurveNum = ((uint16_t)RxBuf[i] | ((uint16_t)RxBuf[i+1]<<8));
            mesCurve = this->getMesCurves()->get(currentCurveNum);
            i+= 2;
            currentPointNum = ((uint16_t)RxBuf[i] | ((uint16_t)RxBuf[i+1]<<8));
            i+= 2;
            workl = ((uint32_t)RxBuf[i] | ((uint32_t)RxBuf[i+1]<<8) | ((uint32_t)RxBuf[i+2]<<16) | ((uint32_t)RxBuf[i+3]<<24));
            i += 4;
            DataLen -= 8;
            if ( firstCycle ) {
                mesCurve->addToMesTimePoint(currentPointNum, _samplingTime);
            } else if (mesCurve->getMesTimePoint(currentPointNum) == 0) {
                mesCurve->addToMesTimePoint(currentPointNum, _samplingTime);
            }
            mesCurve->addToMesCurrent1Point(currentPointNum, workl);
            this->MesUpdate(currentCurveNum, currentPointNum);
        }
        previousPointNum = currentPointNum;
        previousCurveNum = currentCurveNum;
        if ( this->_endOfMes ) {
            this->MesAfter();
        }
        break;

    case EA2PC_RECORDS::startELSV:
        _EstartCurrentTime = ((int32_t)RxBuf[1] | ((int32_t)RxBuf[2]<<8) | ((int32_t)RxBuf[3]<<16) | ((int32_t)RxBuf[4]<<24));
        this->updateELSV();
        break;

    case EA2PC_RECORDS::recordPause:
        work = ((uint16_t)RxBuf[1] | ((uint16_t)RxBuf[2]<<8));
        _break.currentE = work;
        _break.targetMin = ((uint16_t)RxBuf[3] | ((uint16_t)RxBuf[4]<<8));
        _break.targetSec = ((uint16_t)RxBuf[5] | ((uint16_t)RxBuf[6]<<8));
        _break.currentMin = ((uint16_t)RxBuf[7] | ((uint16_t)RxBuf[8]<<8));
        _break.currentSec = ((uint16_t)RxBuf[9] | ((uint16_t)RxBuf[10]<<8));
        this->updatePause();
        break;

    case EA2PC_RECORDS::recordTestCGMDE:
        _CGMDETestNr = ((uint16_t)RxBuf[1] | ((uint16_t)RxBuf[2]<<8));
        this->updateTestCGMDE();
        break;

    default:
        break;
    }
}

bool EAQtData::isMeasurement()
{
    return static_cast<bool>(_measurementGo);
}

// --------------------------------------------------------------------------------------
void EAQtData::MesStart(bool isLsv)
{
    int32_t actptnr;
    int32_t nrOfCurvesMeasured = 1;
    int32_t work;
    int32_t mesCurveIndex;
    int i;
    _mesReadyForUI = false;

    this->_pUI->changeStartButtonText(tr("Start ") + (isLsv?"LSV":"PV"));

    this->_wasLSVMeasurement = isLsv;

    if ( !this->_network->connectToEA() ) {
        return;
    }
    _pUI->MeasurementSetup();

    if ( this->_useSeriesFile && !this->_isMesSeries ) {
        // Wczytaj, ale tylko raz
        this->loadMesFile();
        if ( this->_isMesSeries ) {
            // udało się wczytać plik
            this->_pSeriesData->Mes_Start();
        } else {
            return;
        }
    }

    //////////////// SERIA /////////////////////////////////////////////////////
    if ( _isMesSeries == true ) {
        for (int ii=0 ; ii<PARAM::PARAMNUM ; ii++) {
            if ( ii != PARAM::pro  // parametry nie ustawiane (ignorowane) w eacfg (do ustawienia w EAPro)
                 && ii != PARAM::nonaveragedsampling
                 && ii != PARAM::multi
                 && ii != PARAM::stirrerSpeed
                 && ii != PARAM::knockPower
                 && ii != PARAM::knockTime ) {
                _PVParam[ii] = _pSeriesData->Mes_Param(ii);
            }
        }

        if ( this->_pSeriesData->Mes_PotentialProgramLength() > 0 ) {
            _PVParam[PARAM::pro]= 1;
            setPotentialProgram(QVector<int16_t>::fromStdVector(this->_pSeriesData->Mes_PotentialProgram()));
        }

        if ( this->_pSeriesData->Mes_WaitForUser() == true ) {
            _pUI->showMessageBox(tr("Press OK to continue the measurement series."), tr("Measurement Series"));
        }
        if ( this->_pSeriesData->Mes_Delay() != 0 ) {
            this->seriaWait(this->_pSeriesData->Mes_Delay());
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
            this->_pUI->showMessageBox(tr("Start and end potential cannot be equal in LSV"));
            return;
        }

        _LSVParam[PARAM::stirrerSpeed] = _PVParam[PARAM::stirrerSpeed];
        _LSVParam[PARAM::knockPower] = _PVParam[PARAM::knockPower];
        _LSVParam[PARAM::knockTime] = _PVParam[PARAM::knockTime];

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
            try {
                mesCurveIndex = this->getMesCurves()->addNew(_LSVParam[PARAM::ptnr]); // TMP nie znam ilosci punktow krzywej
            } catch (int e) {
                _pUI->showMessageBox(tr("Could not add new curve. Maximum reached?"), tr("Error"));
                throw e;
            }

            getMesCurves()->get(mesCurveIndex)->getPlot()->setLayer(_pUI->PlotGetLayers()->Measurement);
            getMesCurves()->get(mesCurveIndex)->changeToMesPlot();
            QVector<double> vecMesPotential;
            QVector<double> vecMesTime;
            for (i=0 ; i<PARAM::PARAMNUM ; i++) {
                this->getMesCurves()->get(mesCurveIndex)->Param(i, _LSVParam[i]);
            }
            getMesCurves()->get(mesCurveIndex)->Param(PARAM::method,PARAM::method_lsv);
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
                if ( _LSVParam[PARAM::Ep] < _LSVParam[PARAM::Ek] ) { // not inverted
                    if ( _LSVParam[PARAM::EstartLSV] >= _LSVParam[PARAM::Ep]
                    &&  _LSVParam[PARAM::EstartLSV] <= _LSVParam[PARAM::Ek] ) {
                        QVector<double> vecMesPotential;
                        QVector<double> vecMesTime;
                        this->_ptnrFromEss = (int) ceil( fabs( (double)(_LSVParam[PARAM::Ek] - _LSVParam[PARAM::EstartLSV])
                                                                / MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                        int32_t p;
                        for ( p = 0; p<(_LSVParam[PARAM::ptnr]-this->_ptnrFromEss); ++p ) {
                            vecMesPotential[p] = _LSVParam[PARAM::EstartLSV];
                        }
                        for ( ; p<_LSVParam[PARAM::ptnr]; ++p ) {
                            vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] + ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        }
                        //MesCurve(mesCurveIndex)->reinitializeCurveData(ptnrFromEss);
                        vecMesTime = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), mesCurveIndex);
                        getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                        getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
                    } else {
                        //Brak potencjału startowego na wykresie -- tylko przyłożony potencjał bez mierzenia //
                        QVector<double> vecMesPotential;
                        QVector<double> vecMesTime;
                        vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                        for ( int p=0; p<_LSVParam[PARAM::ptnr]; p++ ) {
                            if ( _LSVParam[PARAM::Ep] < _LSVParam[PARAM::Ek] ) {
                                vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] + ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            } else {
                                vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] - ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            }
                        }
                        vecMesTime = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), mesCurveIndex);
                        getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                        getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
                    }
                } else { // inverted
                    if ( _LSVParam[PARAM::EstartLSV] <= _LSVParam[PARAM::Ep]
                    &&  _LSVParam[PARAM::EstartLSV] >= _LSVParam[PARAM::Ek] ) {
                        QVector<double> vecMesPotential;
                        QVector<double> vecMesTime;
                        this->_ptnrFromEss = (int) ceil( fabs( (double)(_LSVParam[PARAM::Ek] - _LSVParam[PARAM::EstartLSV])
                                                                / MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                        int p;
                        for ( p = 0; p<(_LSVParam[PARAM::ptnr]-this->_ptnrFromEss); ++p ) {
                            vecMesPotential[p] = _LSVParam[PARAM::EstartLSV];
                        }
                        for ( ; p<_LSVParam[PARAM::ptnr]; ++p ) {
                            vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] - ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                        }
                        vecMesTime = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), mesCurveIndex);
                        getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                        getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
                    } else {
                        //Brak potencjału startowego na wykresie -- tylko przyłożony potencjał bez mierzenia //
                        QVector<double> vecMesPotential;
                        QVector<double> vecMesTime;
                        vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
                        for ( int p=0; p<_LSVParam[PARAM::ptnr]; p++ ) {
                            if ( _LSVParam[PARAM::Ep] < _LSVParam[PARAM::Ek] ) {
                                vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] + ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            } else {
                                vecMesPotential[p]=(double)( _LSVParam[PARAM::Ep] - ((double)p * MEASUREMENT::LSVstepE[_LSVParam[PARAM::dEdt]]) );
                            }
                        }
                        vecMesTime = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), mesCurveIndex);
                        getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                        getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
                    }
                }
            } else {
                /*
                * Gdy nie interpretujemy potencjału startowego, to nic ciekawego
                */
                vecMesPotential.resize(_LSVParam[PARAM::ptnr]);
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
                }
                vecMesTime = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), mesCurveIndex);
                getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
            }
            /*
            * !!!
            */
        }
        setCurrentRange(_LSVParam[PARAM::crange],_LSVParam[PARAM::electr]);
    } else { // KONIEC DODATKOWE LSV, POCZATEK DODATKOWE DLA PV

        if ( _PVParam[PARAM::nonaveragedsampling] != 0 ) {
            _PVParam[PARAM::tp]=itp+itw;
            _PVParam[PARAM::tw]=0;
        }

        int nrOfElectrodes = 1;
        std::vector<int> channel_num {};

        if ( _PVParam[PARAM::electr] == PARAM::electr_multi ) { //Wieloelektrodowy
            nrOfCurvesMeasured = 0; //wg. okna dialogowego
            work = _PVParam[PARAM::multi] & 0x000000ff;
            for (int i=0 ; i<8; i++) {
                if ((work & 0x0080) != 0) {
                    channel_num.push_back(i);
                    nrOfCurvesMeasured++;
                }
                work = work << 1;
            }
            nrOfElectrodes = nrOfCurvesMeasured;
        } else {
            channel_num.push_back(0);
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
            try {
                mesCurveIndex = this->getMesCurves()->addNew(_PVParam[PARAM::ptnr]); // TMP nie znam ilosci punktow krzywej
            } catch (int e) {
                _pUI->showMessageBox(tr("Could not add new curve. Maximum reached?"), tr("Error"));
                throw e;
            }
            getMesCurves()->get(mesCurveIndex)->getPlot()->setLayer(_pUI->PlotGetLayers()->Measurement);
            getMesCurves()->get(mesCurveIndex)->changeToMesPlot(channel_num[mesCurveIndex % nrOfElectrodes]);
            for (i=0 ; i<PARAM::PARAMNUM ; i++) {
                this->getMesCurves()->get(mesCurveIndex)->Param(i, _PVParam[i]);
            }

            if ( _PVParam[PARAM::pro] == PARAM::pro_yes
                 && this->_PVParam_PotentialProgram.size() > 2 ) {
                //
                // TYLKO DLA Programowania potencjalu
                //
                int prsize = _PVParam_PotentialProgram.size();
                QVector<double> vecP;
                QVector<double> vecT;
                if ( getMesCurves()->get(mesCurveIndex)->Param(PARAM::sampl) == PARAM::sampl_single ) {
                    getMesCurves()->get(mesCurveIndex)->reinitializeCurveData(prsize);
                    getMesCurves()->get(mesCurveIndex)->Param(PARAM::ptnr,prsize);
                    getMesCurves()->get(mesCurveIndex)->Param(PARAM::Ek,prsize);
                    if ( _PVParam[PARAM::messc] == PARAM::messc_cyclic && nrOfElectrodes <= mesCurveIndex ) {
                        //powrot cyklicznej
                        for ( int y=prsize-1; y<=0; y-- ) {
                            vecP.append(_PVParam_PotentialProgram[y]);
                        }
                        vecT = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), 1);
                    } else {
                        // normalny pomiar
                        for (int y=0; y<prsize;y++) {
                            vecP.append(this->_PVParam_PotentialProgram[y]);
                        }
                        vecT = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), 0);
                    }
                } else {
                    getMesCurves()->get(mesCurveIndex)->reinitializeCurveData(prsize/2);
                    getMesCurves()->get(mesCurveIndex)->Param(PARAM::ptnr,prsize/2);
                    getMesCurves()->get(mesCurveIndex)->Param(PARAM::Ek,prsize/2);
                    if ( _PVParam[PARAM::messc] == PARAM::messc_cyclic && nrOfElectrodes <= mesCurveIndex ) {
                        //powrot cyklicznej
                        for ( int y=prsize-1; y<=0; y-=2 ) {
                            vecP.append(_PVParam_PotentialProgram[y/2]);
                        }
                        vecT = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), 1);
                    } else {
                        // normalny pomiar
                        for (int y=0; y<prsize;y+=2) {
                            vecP.append(this->_PVParam_PotentialProgram[y/2]);
                        }
                        vecT = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), 0);
                    }
                }
                getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecP);
                getMesCurves()->get(mesCurveIndex)->setTimeVector(vecT);
                getMesCurves()->get(mesCurveIndex)->Param(PARAM::method,2);
                getMesCurves()->get(mesCurveIndex)->Param(PARAM::Ep, 0);
                getMesCurves()->get(mesCurveIndex)->Param(PARAM::Estep, 1);
            } else {
                //
                // Jak bez programowania potencjalu
                //
                QVector<double> vecMesPotential;
                QVector<double> vecMesTime;
                vecMesPotential.resize(_PVParam[PARAM::ptnr]);
                if ( nrOfElectrodes > mesCurveIndex ) {
                    for ( int p=0; p<_PVParam[PARAM::ptnr]; p++ ) {
                        vecMesPotential[p] = _PVParam[PARAM::Ep] + (p* _PVParam[PARAM::Estep]);
                    }
                    vecMesTime = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), 0);

                } else { // powrót cyklicznej //
                    for ( int p=0; p<_PVParam[PARAM::ptnr]; p++ ) {
                        vecMesPotential[p] = _PVParam[PARAM::Ek] - ( (1.0+p) * _PVParam[PARAM::Estep]);
                    }
                    vecMesTime = this->generateTimeVector(getMesCurves()->get(mesCurveIndex)->Params(), 1);
                }

                getMesCurves()->get(mesCurveIndex)->setPotentialVector(vecMesPotential);
                getMesCurves()->get(mesCurveIndex)->setTimeVector(vecMesTime);
            }

            if ( getMesCurves()->get(mesCurveIndex)->Param(PARAM::nonaveragedsampling) != 0 ) {
                getMesCurves()->get(mesCurveIndex)->allocateProbingData();
            }
            this->getMesCurves()->get(mesCurveIndex)->allocateMesArray();
        }

        {
            // HACK -- very ugly code to allow for reasingment of variables
            // only required for method_sqwclassic. It needs two functions
            // to be executed prepare and recover !!
            if ( _PVParam[PARAM::method] == PARAM::method_sqw_classic ) {
                this->prepareParamForSQWClassic(getMesCurves()->get(0));
            }
            initEca();
            initPtime();
            //ilpw = this->getMesCurves()->get(0)->Param(PARAM::aver);
            createMatrix();
            this->recoverParamForSQWClassic(getMesCurves()->get(0));
        }

        ainmat = 0;
        if (_measurementMatrix[0] == -1) {
            ainmat = 1;
        }
        if ( _PVParam[PARAM::method] == PARAM::method_sqw_osteryoung ) {
            crmxSQW(); // SQW
        }

    } // KONIEC DODATKOWE

    _stopInfo = 0;
    _conductingMeasurement = 1;
    setCurrentRange(getMesCurves()->get(0)->Param(PARAM::crange)
                    ,getMesCurves()->get(0)->Param(PARAM::electr) );

    if ( getXAxis() == XAXIS::potential ) {
        _pUI->PlotSetInverted( (getMesCurves()->get(0)->Param(PARAM::Ek) < getMesCurves()->get(0)->Param(PARAM::Ep)) );
    } else {
        _pUI->PlotSetInverted(false);
    }

    if ( _wasLSVMeasurement == 0 ){
        if ( sendPVToEA() ) {
            _PVParam[PARAM::tp]=itp;
            _PVParam[PARAM::tw]=itw;
            int nn=0;
            while (getMesCurves()->get(nn) != nullptr) {
                getMesCurves()->get(nn)->Param(PARAM::tp,_PVParam[PARAM::tp]);
                getMesCurves()->get(nn)->Param(PARAM::tw,_PVParam[PARAM::tw]);
                nn++;
            }

            return;
        } else { // m_nMesInfo=1 - pomiar techniki impulsowe
            _conductingMeasurement = 0;
            _PVParam[PARAM::tp]=itp;
            _PVParam[PARAM::tw]=itw;
            this->getMesCurves()->clear();
            return;
        }
    } else {
        if ( sendLSVToEA() ) {
            return ;
        } // m_nMesInfo=2,3 - pomiar LSV
        else {
            _conductingMeasurement = 0;
            this->getMesCurves()->clear();
            return ;
        } // błedy podczas transmisji parametrów lub EA odłaczony
    }

}

bool EAQtData::sendPVToEA()
{
    int32_t i, lenEact;
    int32_t TxN;

    char TxBufAlt[320];

    // Pomiar - techniki impulsowe
    // Z komputera: Param, matrix, E1I, E2I, E3I, stepE, ptime, tr(long), drops_bm
    TxN = 0;
    _TxBuf[0] = PC2EA_RECORODS::recordPV;
    TxN = 3;

    int16_t lenE;
    int16_t E[1200];

    {
        // HACK -- very ugly code to allow for reasingment of variables
        // only required for method_sqwclassic. It needs two functions
        // to be executed prepare and recover !!
        this->prepareParamForSQWClassic(getMesCurves()->get(0));

        if ( this->getMesCurves()->get(0)->Param(PARAM::pro) == 1 ) {
            if ( _PVParam_PotentialProgram.size() < 3 ) {
                _pUI->showMessageBox(tr("Potential program has to have more than 2 points"), tr("Error"));
                return false;
            }
            lenE = _PVParam_PotentialProgram.size();
            for ( int i=0; i<lenE; i++ ) {
                //E[i] = i*10;
                E[i]=_PVParam_PotentialProgram[i];
            }
            E1I = E[0];
            E2I = E[1];
            E3I = E[2];
            //E3I = E2I - this->getMesCurves()->get(0)->Param(PARAM::dE);
        }

        for (i=0 ; i<PARAM::PARAMNUM ; i++) {
            _TxBuf[TxN] = (unsigned char)(this->getMesCurves()->get(0)->Param(i) & 0x000000ff); TxN++;
            _TxBuf[TxN] = (unsigned char)((this->getMesCurves()->get(0)->Param(i) >> 8) & 0x000000ff); TxN++;
            _TxBuf[TxN] = (unsigned char)((this->getMesCurves()->get(0)->Param(i) >> 16) & 0x000000ff); TxN++;
            _TxBuf[TxN] = (unsigned char)((this->getMesCurves()->get(0)->Param(i) >> 24) & 0x000000ff); TxN++;
        }

        this->recoverParamForSQWClassic(getMesCurves()->get(0));
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

    if ( this->_network->sendToEA((char*)&_TxBuf[0]) <= 0 ) {
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

            this->_network->sendToEA((char*)&_TxBuf[0]);

        }

    }
    return true;
}

void EAQtData::MesStop()
{
    _stopInfo = 1;
    _ptnrFromEss = 0;
    if (_conductingMeasurement == 0) {
        this->_pUI->showMessageBox(tr("Measurement was not initiated.")); // Pomiar nie został zainicjowany
    } else {
        _conductingMeasurement = 0;
        _TxBuf[0] = PC2EA_RECORODS::recordStop;
        this->_network->sendToEA((char*)&_TxBuf[0]);
        if ( this->_isMesSeries ) {
            delete this->_pSeriesData;
            this->_pSeriesData = new MesCFG();
            this->_isMesSeries = false;
            this->_useSeriesFile = true;
        }
        _pUI->MeasurementAfter();
    }
}

bool EAQtData::sendLSVToEA()
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


    if ( this->_network->sendToEA((char*)&_TxBuf[0]) > 0 ) {
        return true;
    } else {
        return false;
    }
}

void EAQtData::MesUpdate(int32_t nNrOfMesCurve, int32_t nPointFromDevice)
{
    ///////////////// SETUP //////////////////
    if ( this->_performSetup == true && !this->_endOfMes ) {
        this->_performSetup = false;
        /*
        * zliczanie ilości elektrod w wieloelektrodowym
        */
        _fromUpdate.restart();
        if ( this->_PVParam[PARAM::electr] == PARAM::electr_multi ) {
            this->_multielectrodeNr = 0;
            int32_t work;
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
        while ( getMesCurves()->get(ic) != nullptr ) {
            if ( !getMesCurves()->get(ic)->hasPlot() ) {
                getMesCurves()->get(ic)->setPlot(this->_pUI->PlotAddQCPCurve());
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
    if ( this->getMesCurves()->get(nNrOfMesCurve)->getMesTimePoint(nPointFromDevice) == 0 ) {
        return;
    }
    ////////////// END CHECKS /////////////////////////

    ///////////// CALC RESULT ////////////////////
    double res;
    Curve* mesCurve = getMesCurves()->get(nNrOfMesCurve);
    if ( _wasLSVMeasurement == 0 ) {

        if ( nNrOfMesCurve >= this->_multielectrodeNr ) {
            // powrot cyklicznej ...
            if ( mesCurve->Param(PARAM::method) == PARAM::method_sqw_classic ) {
                res =  this->CountResultPV(
                                ((mesCurve->getMesCurrent1Point(nPointFromDevice)
                                      - mesCurve->getMesCurrent2Point(nPointFromDevice))
                                 / mesCurve->getMesTimePoint(nPointFromDevice))
                                );
            } else {
                res =  this->CountResultPV(
                                ((mesCurve->getMesCurrent2Point(nPointFromDevice)
                                      - mesCurve->getMesCurrent1Point(nPointFromDevice))
                                 / mesCurve->getMesTimePoint(nPointFromDevice))
                                );
            }
            mesCurve->addDataPoint(
                res
                , nPointFromDevice
            );
        } else {
            if ( mesCurve->Param(PARAM::method) == PARAM::method_sqw_classic ) {
                res =  this->CountResultPV(
                                ((mesCurve->getMesCurrent1Point(nPointFromDevice)
                                      - mesCurve->getMesCurrent2Point(nPointFromDevice))
                                 / mesCurve->getMesTimePoint(nPointFromDevice))
                                );
            } else {
                res =  this->CountResultPV(
                                ((mesCurve->getMesCurrent2Point(nPointFromDevice)
                                      - mesCurve->getMesCurrent1Point(nPointFromDevice))
                                 / mesCurve->getMesTimePoint(nPointFromDevice))
                                );
            }
            mesCurve->addDataPoint(
                res
                , nPointFromDevice
            );
        }
    } else {
        if ( this->_prepareEstart ) {
            /*
             * When LSV start from Es (not Ep), we need to fillin the fields in potential vector anyway
             */
            _prepareEstart = false;
            double res1 =  this->CountResultLSV(
                                (mesCurve->getMesCurrent1Point(nPointFromDevice)
                                 / mesCurve->getMesTimePoint(nPointFromDevice))
                            );
            for ( int i = 0; i<(mesCurve->Param(PARAM::ptnr) - this->_ptnrFromEss); ++i ) {
                mesCurve->addDataPoint(
                    res1
                    , i
                );
            }
        }
        if ( nNrOfMesCurve % 2 == 0 ) {
            res =  this->CountResultLSV(
                            (mesCurve->getMesCurrent1Point(nPointFromDevice)
                             / mesCurve->getMesTimePoint(nPointFromDevice))
                            );
            mesCurve->addDataPoint(
                res
                , nPointFromDevice
            );
        } else {
            res =  this->CountResultLSV(
                            (mesCurve->getMesCurrent1Point(nPointFromDevice)
                             / mesCurve->getMesTimePoint(nPointFromDevice))
                            );
            mesCurve->addDataPoint(
                res
                , nPointFromDevice
            );
        }
    }
    _lastCurve = nNrOfMesCurve;
    _lastPoint = nPointFromDevice;
    _mesReadyForUI = true;
    _pUI->setLowLabelText(2, ""); // Cleanup break info etc. //
}

void EAQtData::MesPrepareUIUpdate()
{
    int msecnow = _fromUpdate.elapsed();
    if (_mesReadyForUI) {
        _pUI->setLowLabelText(1,tr(" [FPS: %1]").arg( 1.0 / ((double)msecnow/1000.0) ));
        this->_pUI->MeasurementUpdate(_lastCurve, _lastPoint);
        _fromUpdate.restart();
    }
}

/*
 *  Calculate final result for LSV
 */
double EAQtData::CountResultLSV(int64_t ResI)
{
    static double dResI;
    ResI *= 60L;

    Curve* c = getMesCurves()->get(0);

    ResI -= this->_IUE0;

    if ( c->Param(PARAM::electr) < PARAM::electr_micro ) { //MACRO ELEKTRODA
        dResI = (double)ResI * MEASUREMENT::scale_macro[c->Param(PARAM::crange)] * MEASUREMENT::multiply;
    } else {
        dResI = (double)ResI * MEASUREMENT::scale_micro[c->Param(PARAM::crange)] * MEASUREMENT::multiply;
    }

    return dResI;
}

/*
 * Calculate final result for PV
 */
double EAQtData::CountResultPV(int64_t ResI)
{
    static double dResI;
    ResI *= 60L;

    Curve* c = getMesCurves()->get(0);

    if (c->Param(PARAM::method) == PARAM::method_sqw_osteryoung ) {
        if (c->Param(PARAM::sampl) == PARAM::sampl_double) {
            ResI -= this->_IUE0;
        }
    } else { // not SQW
        if (c->Param(PARAM::sampl) == PARAM::sampl_single) {
            ResI -= this->_IUE0;
        }
    }

    if ( c->Param(PARAM::electr) < PARAM::electr_micro ) { //MACRO ELEKTRODA
        dResI = (double)ResI * MEASUREMENT::scale_macro[c->Param(PARAM::crange)] * MEASUREMENT::multiply;
    } else {
        dResI = (double)ResI * MEASUREMENT::scale_micro[c->Param(PARAM::crange)] * MEASUREMENT::multiply;
    }

    return dResI;
}

/*
 * E start is applied, we need to display it
 */
void EAQtData::updateELSV()
{
    _pUI->setLowLabelText(2, tr("Applying Estart: %1 mV, %2 s").arg(_mesCurves->get(0)->Param(PARAM::EstartLSV)).arg(_EstartCurrentTime));
    return;
}

/*
 * Pause (break) is applied, we need to display it
 */
void EAQtData::updatePause()
{
    _pUI->setLowLabelText(2, tr("Break: %1 mV    %2:%3 of %4:%5 ").arg(_break.currentE)
                                                                 .arg(_break.currentMin)
                                                                 .arg(_break.currentSec,2,10,QChar('0'))
                                                                 .arg(_break.targetMin)
                                                                 .arg(_break.targetSec,2,10,QChar('0')) );
    return;
}

/*
 * CGMDE test is performed, we need to display it
 */
void EAQtData::updateTestCGMDE()
{
    //TODO dialog window
    //_pUI->setLowLabelText(0,tr("CGMDE test: %1").arg(_testCGMDE));
    _pUI->updateCGMDETest();
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

    _mesReadyForUI = false;
    _conductingMeasurement = 0;
    work_act = this->Act();
    // nie wiem czy trzeba: m_nStopInfo = 1;

    if ( _mesCurves->count() > 0 && _mesCurves->get(0)->getNrOfDataPoints() > 0 ) {
        this->_pUI->MeasurementUpdate(0,_mesCurves->get(0)->getNrOfDataPoints()-1);
    }

    EAQtSaveFiledialog::SaveDetails saveDetails;
    if (this->_isMesSeries != true ) // to nie pomiar seryjny
    {
        if ( this->_pUI->showQuestionBox( tr("Save dialog"),
                                          tr("Do you want to save the result?"),
                                          true ) ) {
            err = -1;
            while (err < 0)
            {
                saveDetails = this->_pUI->DialogSaveInFile();
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
        //QSound::play("");
        delete _pSeriesData;
        _pSeriesData = new MesCFG();
        _isMesSeries = false;
        _useSeriesFile = true;
        _pUI->showMessageBox(tr("Measurement series completed."), tr("Measurement series"));
    }
    QFileInfo fi(saveDetails.fileName);
    this->_pUI->MeasurementAfter();
    _pUI->setPathInUse(fi.absoluteDir().canonicalPath());
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

    /*
    if ( UserFName.right(5).compare(".volt",Qt::CaseInsensitive) != 0 ) {
        UserFName += ".volt";
    }
    */

    /*
    *
    */
    QString orderedPrefixes[8];
    int cntElectrodes = 0;
    if ( this->_PVParam[PARAM::electr] == PARAM::electr_multi ) {
        int32_t work;
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
    while ( this->getMesCurves()->get(num) != nullptr ) {
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
                this->getMesCurves()->get(num)->FName(UserFName);
                this->getMesCurves()->get(num)->Comment(UserComment);
                this->getMesCurves()->get(num+1)->FName(UserFName);
                this->getMesCurves()->get(num+1)->Comment(UserComment);
                this->getMesCurves()->get(num+1)->setDate();
                if ( UserCName.at(0) != QChar(0x25EE) ) { // 0x25EE = "◭"
                    this->getMesCurves()->get(num)->CName("◭" + UserCName);
                    this->getMesCurves()->get(num+1)->CName("◮" + UserCName);
                } else {
                    this->getMesCurves()->get(num)->CName(UserCName);
                    this->getMesCurves()->get(num+1)->CName(UserCName);
                    getMesCurves()->get(num+1)->CName().replace(0,1,QChar(0x25EE));// 0x25EE = "◭"
                }
            }
            num++;
        }
    }

    num=0;
    while ( getMesCurves()->get(num) != nullptr ) {
        if ( (err = safeAppend(getMesCurves()->get(num)->FName(), getMesCurves()->get(num))) < 0 ) {
            //AfxMessageBox(IDS_info6, MB_OK);
            return -1;
        }
        num++;
    }


    while ( getMesCurves()->get(0) != nullptr ) {
        TYPES::vectorindex_t index;
        try {
            index = this->getCurves()->append(this->getMesCurves()->get(0));
        } catch (int e) {
            _pUI->showMessageBox(tr("Could not append curve."), tr("Error"));
            return -1;
        }

        this->getMesCurves()->unset((TYPES::vectorindex_t)0);
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
int EAQtData::safeAppend(QString pFileName, Curve* CurveToAppend)
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
    char *validTemp;
    TYPES::filesize_t fileLen;
    TYPES::vectorindex_t nCurveCntr; // licznik krzywych

    QFile *file = new QFile(pFileName);

    if ( !file->exists() )
    {
        if( file->open(QIODevice::ReadWrite) ) // jezeli nie ma proba utworzenia
        {
            nCurveCntr = 0;
            file->write((char*)&nCurveCntr,sizeof(TYPES::vectorindex_t));
            file->close();
            if ( !file->open(QIODevice::ReadOnly) ) {
                this->_pUI->showMessageBox(tr("Could not create file"));
                return(-1);
            }
        } else {
            this->_pUI->showMessageBox(tr("Could not access file created file"));
            return(-1); // brak dostepu do pliku
        }
    } else {
        if ( !file->open(QIODevice::ReadOnly) ) {
            this->_pUI->showMessageBox(tr("Could not access existing file"));
            return(-1);
        }
    }

    //Get file length
    file->seek(0);
    fileLen=file->size();

    // Sprawdzanie czy plik nie jest za duzy do zaladowania do pamieci lub pusty
    if ( fileLen > TYPES::maxFileSize ||  fileLen < 4 )
    {
        this->_pUI->showMessageBox(tr("File is larger than %1 or empty.").arg(TYPES::maxFileSize));
        file->close();
        return(-2);
    }

    QByteArray byteArray = file->readAll();
    buffer = byteArray.data();
    file->close();

    // Verify the number of curves in file
    TYPES::vectorindex_t *CurvInFile;
    CurvInFile=(TYPES::vectorindex_t*)buffer;



    QVector<QString> CurveNames(*CurvInFile);
    int nComperator=0;
    TYPES::curvefileindex_t CurvInFileLen;

    TYPES::filesize_t curveOffset=4;

    for (TYPES::vectorindex_t i=0;i<*CurvInFile;++i)
    {
        // a - start of curve
        CurvInFileLen=*(TYPES::curvefileindex_t*)(buffer+curveOffset); // length of curve with bytes representing the length

        int posInName=0;
        QByteArray ba;
        while ((char)*(buffer+curveOffset+4+posInName) != '\0') // name of curve -- +4 bytes of its size
        {
            ba.append((char)*(buffer+curveOffset+4+posInName));
            posInName++;
        }
        CurveNames[i] = QString::fromUtf8(ba);

        if( CurveNames[i].compare(CurveToAppend->CName(),Qt::CaseSensitive) == 0 ) // does curve exists in file
            nComperator = nComperator + 1;  //if does increment its name

        curveOffset += CurvInFileLen;
    }

    QString appCurveName = "";

    if( nComperator == 0 )
    {
        appCurveName = CurveToAppend->CName();
    } else {
        uint appNumber = 0;
        while(true) //change the curve name
        {
            nComperator=0;
            appCurveName = CurveToAppend->CName();

            appCurveName.append(tr("%1").arg(appNumber++));

            for ( TYPES::vectorindex_t p=0;p<*CurvInFile;p++ )
            {
                if( CurveNames[p].compare(appCurveName,Qt::CaseSensitive) == 0 )
                    nComperator = nComperator + 1;
            }

            if( nComperator==0 )
            {
                break;
            }

            if ( appNumber > 98 )
            {
                byteArray.clear();
                this->_pUI->showMessageBox(tr("Too many curves with the same name in file."));
                return(-8);
            }
        }
    }

    CurveToAppend->CName(appCurveName);
    *CurvInFile=*CurvInFile+1;	// number of curves +1 (new one) -- 4 bytes
    memcpy(buffer,CurvInFile,sizeof(TYPES::vectorindex_t));

    QString TempFileName = pFileName + ".tmp";
    QFile *outFile = new QFile(TempFileName);
    QByteArray toSave;
    if ( pFileName.right(FILES::saveCompressExt.size()).compare(FILES::saveCompressExt,Qt::CaseInsensitive) == 0 ) {
        toSave = CurveToAppend->serialize(true);
    } else {
        toSave = CurveToAppend->serialize(false);
    }
    TYPES::curvefileindex_t addLen = toSave.size() + sizeof(TYPES::curvefileindex_t);
    if ( outFile->open(QIODevice::ReadWrite) )
    {
        outFile->write(buffer,fileLen);
        outFile->write((char*)&addLen, sizeof(TYPES::curvefileindex_t));
        outFile->write(toSave);
    } else {
        this->_pUI->showMessageBox(tr("Could not open tmp file. Curve not saved."));
        return(-3);
    }
    outFile->close();

    ////////////////////////////////////////////////////////////////////////////
    //////////////////// walidacja pliku TEMP //////////////////////////////////

    TYPES::filesize_t tmpFileLen;

    if( !outFile->open(QIODevice::ReadOnly) ) {
        this->_pUI->showMessageBox(tr("Error while openning temp file. Curve not saved."));
        return(-6);
    }

    outFile->seek(0);
    tmpFileLen=outFile->size();

    if ( tmpFileLen > TYPES::maxFileSize  ||  tmpFileLen < 240 )
    {
        outFile->close();
        this->_pUI->showMessageBox(tr("Error while openning temp file, or file damaged. Curve not saved."));
        return(-6);
    }

    QByteArray qba2 = outFile->readAll();
    validTemp = qba2.data();
    outFile->close();

    TYPES::vectorindex_t CurvsInTmpFile;
    CurvsInTmpFile=*(TYPES::vectorindex_t*)validTemp;

    if ( CurvsInTmpFile<1 )
    {
        qba2.clear();
        this->_pUI->showMessageBox(tr("Error while reading number of files from tmp. Curve not saved."));
        return(-6);
    }

    TYPES::filesize_t nOffsetWholeSum = sizeof(TYPES::vectorindex_t); //First 4 bytes is number of curves in file

    for(int II=0;II<CurvsInTmpFile;II++)
    {
        TYPES::curvefileindex_t* tmpCurOffset;
        tmpCurOffset=(TYPES::curvefileindex_t*)(validTemp+nOffsetWholeSum);
        nOffsetWholeSum+= (*tmpCurOffset);
        if ( tmpFileLen < nOffsetWholeSum ) {
            qba2.clear();
            this->_pUI->showMessageBox(tr("Error while verifing size of tmp file (offset sum > filesize). Curve not saved."));
            return (-6);
        }
    }


    if( nOffsetWholeSum<=4 || (tmpFileLen!=nOffsetWholeSum) ) // verify offsets and file size
    {
        qba2.clear();
        this->_pUI->showMessageBox(tr("Error while verifing size of tmp file (offset sum != filesize). Curve not saved."));
        return(-6);
    }

    qba2.clear();

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    if ( !file->remove() ) {
        this->_pUI->showMessageBox(tr("Cannot save to file."));
        return(-7);
    }

    if ( !outFile->rename(pFileName) ) {
        this->_pUI->showMessageBox(tr("Cannot save to file."));
        return(-7);
    }

    CurveToAppend->CName(appCurveName);
    CurveToAppend->FName(pFileName);

    return 0;
}


void EAQtData::loadMesFile()
{
    int res = -1;
    try {
        res = this->_pSeriesData->LoadFromFile(this->_seriesFilePath.toStdString());
    } catch (int a) {
        this->_pUI->showMessageBox(tr("error loading file: ") + this->dispNR(a));
        return;
    }

    if ( res == -1 ) {
        QString b = this->_pSeriesData->lasterroron.c_str();
        this->_pUI->showMessageBox( tr("Syntax error on line: %1").arg(b));
        this->_isMesSeries = false;
        return;
    } else if ( res == -2 ) {
        this->_pUI->showMessageBox(tr("The file is not compatible with this version of EAQt"));
        this->_isMesSeries = false;
        return;
    } else if ( res == -3 ) {
        this->_pUI->showMessageBox(tr("Error while loading the file"));
        this->_isMesSeries = false;
        return;
    } else if ( res == 0 ) {
        this->_pUI->showMessageBox(tr("No measurements loaded, terminating.")); // Wczytano 0 krzywych, nie kontynuuję
        this->_isMesSeries = false;
        return;
    } else if ( res > 0 ) {
        this->_pUI->showMessageBox(tr("Loaded: %1 measurements. Starting the measurement series.").arg( this->dispNR(res)));
        this->_isMesSeries = true;
    } else {
        this->_pUI->showMessageBox(tr("Critical error. Terminating.")); // Błąd krytyczny, nie kontynuuję
        this->_isMesSeries = false;
        return;
    }
}

/*
 * When there is delay between measurements is MeasurementSeries, this should be done
 */
void EAQtData::seriaWait(int32_t delay_secs)
{
    QTime dieTime= QTime::currentTime().addSecs(delay_secs);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        _pUI->setLowLabelText(2,tr("Measurement series - timer: %1 s").arg(QTime::currentTime().secsTo(dieTime)));
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


int32_t EAQtData::getDropsBeforeMes()
{
    return _accessories.drops_bm;
}


void EAQtData::setDropsBeforeMes(int32_t drops)
{
    _accessories.drops_bm = drops;
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
    work = _PVParam[PARAM::stirrerSpeed];
    _TxBuf[TxN] = (unsigned char)(work & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((work >> 8) & 0x00ff); TxN++;

    // knock
    work = _PVParam[PARAM::knockPower];
    _TxBuf[TxN] = (unsigned char)(work & 0x00ff); TxN++;
    _TxBuf[TxN] = (unsigned char)((work >> 8) & 0x00ff); TxN++;

    // knock - time
    work = _PVParam[PARAM::knockTime];
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

    this->_network->sendToEA((char*)&_TxBuf[0]);

    return;
}

int32_t EAQtData::getCGMDETestNum()
{
    return _CGMDETestNr;
}

void EAQtData::sendTestCGMDEStop()
{
    _TxBuf[0] = PC2EA_RECORODS::recordCGMDEstop;
    _network->sendToEA((char*)&_TxBuf[0]);
    _measurementGo = 0;
}

bool EAQtData::sendTestCGMDE()
{
    if ( !this->_network->connectToEA() ) {
        return false;
    }

    int16_t work;

    int32_t TxN = 0;

    _TxBuf[TxN++] = PC2EA_RECORODS::recordCGMDE;

    work = ParamPV(PARAM::knockPower);
    _TxBuf[TxN++] = (unsigned char)(work & 0x00ff);
    _TxBuf[TxN++] = (unsigned char)((work >> 8) & 0x00ff);

    work = ParamPV(PARAM::knockTime);
    _TxBuf[TxN++] = (unsigned char)(work & 0x00ff);
    _TxBuf[TxN++] = (unsigned char)((work >> 8) & 0x00ff);

    //generation pulse
    _TxBuf[TxN++] = (unsigned char)(_testCGMDE.GP & 0x00ff);
    _TxBuf[TxN++] = (unsigned char)((_testCGMDE.GP >> 8) & 0x00ff);

    //break time
    _TxBuf[TxN++] = (unsigned char)(_testCGMDE.BT & 0x00ff);
    _TxBuf[TxN++] = (unsigned char)((_testCGMDE.BT >> 8) & 0x00ff);

    //potential
    _TxBuf[TxN++] = (unsigned char)(_testCGMDE.ElPot & 0x00ff);
    _TxBuf[TxN++] = (unsigned char)((_testCGMDE.ElPot >> 8) & 0x00ff);

    //potential info   0 - no potential
    _TxBuf[TxN++] = (unsigned char)(_testCGMDE.ChPot & 0x00ff);

    if ( _network->sendToEA((char*)&_TxBuf[0])>0 ) {
        _measurementGo = 1;
        return true;
    } else {
        return false;
    }
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
    if ( _xaxis_type != XAXIS::potential ) {
        _pUI->PlotSetInverted(false);
    } else {
        Curve* c;
        if ( _measurementGo ) {
            if ( getMesCurves()->get(0) != nullptr ) {
                c = getMesCurves()->get(0);
            } else {
                return;
            }
        } else {
            c = getCurves()->get(Act());
            if ( c == nullptr ) {
                this->_pUI->updateAll();
                return;
            }
        }
        if ( c->Param(PARAM::Ek) < c->Param(PARAM::Ep) ) {
            _pUI->PlotSetInverted(true);
        }
    }
    if ( _measurementGo == 0 ) {
        this->_pUI->updateAll();
    }
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
        ret = tr("%1 µA").arg(dNumber,0,'f', 5);
        break;
    case PARAM::crange_micro_500nA + PARAM::crange_macro_100nA + 1:
        ret = tr("%1 nA").arg(dNumber*1000,0,'f', 3);
        break;
    case PARAM::crange_micro_50nA + PARAM::crange_macro_100nA + 1:
        ret = tr("%1 nA").arg(dNumber*1000,0,'f', 4);
        break;
    case PARAM::crange_micro_5nA + PARAM::crange_macro_100nA + 1:
        ret = tr("%1 nA").arg(dNumber*1000,0,'f', 5);
        break;
    case PARAM::crange_macro_100mA:
        ret = tr("%1 mA").arg(dNumber/1000,0,'f', 3);
        break;
    case PARAM::crange_macro_10mA:
        ret = tr("%1 mA").arg(dNumber/1000,0,'f', 4);
        break;
    case PARAM::crange_macro_1mA:
        ret = tr("%1 mA").arg(dNumber/1000,0,'f', 5);
        break;
    case PARAM::crange_macro_100uA:
        ret = tr("%1 µA").arg(dNumber,0,'f', 3);
        break;
    case PARAM::crange_macro_10uA:
        ret = tr("%1 µA").arg(dNumber,0,'f', 4);
        break;
    case PARAM::crange_macro_1uA:
        ret = tr("%1 µA").arg(dNumber,0,'f', 5);
        break;
    case PARAM::crange_macro_100nA:
        ret = tr("%1 nA").arg(dNumber*1000,0,'f', 3);
        break;
    default:
        ret = tr("%1 µA").arg(dNumber,0,'f', 5);
        break;
    }
    return ret;
}

QString EAQtData::dispIforTXT(double dNumber) {
    static QString ret;
    switch (this->_currentRange) {
    case PARAM::crange_micro_5uA + PARAM::crange_macro_100nA + 1:
        ret = QString("%1").arg(dNumber,0,'f', 5);
        break;
    case PARAM::crange_micro_500nA + PARAM::crange_macro_100nA + 1:
        ret = QString("%1").arg(dNumber*1000,0,'f', 3);
        break;
    case PARAM::crange_micro_50nA + PARAM::crange_macro_100nA + 1:
        ret = QString("%1").arg(dNumber*1000,0,'f', 4);
        break;
    case PARAM::crange_micro_5nA + PARAM::crange_macro_100nA + 1:
        ret = QString("%1").arg(dNumber*1000,0,'f', 5);
        break;
    case PARAM::crange_macro_100mA:
        ret = QString("%1").arg(dNumber/1000,0,'f', 3);
        break;
    case PARAM::crange_macro_10mA:
        ret = QString("%1").arg(dNumber/1000,0,'f', 4);
        break;
    case PARAM::crange_macro_1mA:
        ret = QString("%1").arg(dNumber/1000,0,'f', 5);
        break;
    case PARAM::crange_macro_100uA:
        ret = QString("%1").arg(dNumber,0,'f', 3);
        break;
    case PARAM::crange_macro_10uA:
        ret = QString("%1").arg(dNumber,0,'f', 4);
        break;
    case PARAM::crange_macro_1uA:
        ret = QString("%1").arg(dNumber,0,'f', 5);
        break;
    case PARAM::crange_macro_100nA:
        ret = QString("%1").arg(dNumber*1000,0,'f', 3);
        break;
    default:
        ret = QString("%1").arg(dNumber,0,'f', 5);
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

void EAQtData::setCurrentRange(int nNewRange, int nElek) {
    int add;
    if ( nElek >= PARAM::electr_micro ) {
        add = PARAM::crange_macro_100nA + 1;
    } else {
        add = 0;
    }
    this->_currentRange = nNewRange + add;
}

int EAQtData::getCurrentRange()
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
        enabled[i] = (bool)(multi & 0x00000080);
        multi = multi << 1;
    }
    return enabled;
}

void EAQtData::setChannelName(uint channelNum, QString name)
{
    if (channelNum >= _vChannelNamesOfMultielectrode.size())
        throw 1;
    _vChannelNamesOfMultielectrode[channelNum] = name;
}

void EAQtData::setChannelEnabled(uint channelNum, bool enabled)
{
    if (channelNum >= _vChannelNamesOfMultielectrode.size())
        throw 1;
    int32_t multi = _PVParam[PARAM::multi];
    int32_t pos = 7 - channelNum;
    multi &= ~(1 << pos);
    multi |= (enabled << pos);
    _PVParam[PARAM::multi] = multi;
}

bool EAQtData::getUseMesFile()
{
   return _useSeriesFile;
}

void EAQtData::setUseMesFile(bool is)
{
    _useSeriesFile = is;
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

void EAQtData::exportToVOL(QString path)
{
    Curve* c;
    if (this->Act() >= 0) {
        c = _curves->get(_act);
        if ( c == nullptr) {
            _pUI->showMessageBox(tr("Could not find any curve to save."), tr("Error"));
            return;
        }
        if (c->Param(PARAM::ptnr) > 1000) {
            _pUI->showMessageBox(tr("Maximum number of points per curve in vol file is 1000."), tr("Error"));
            return;
        }
    } else {
        c = _curves->get(0);
        int i = 0;
        while ((c=_curves->get(i)) != nullptr) {
            ++i;
            if (c->Param(PARAM::ptnr) > 1000) {
                _pUI->showMessageBox(tr("Maximum number of points per curve in vol file is 1000."), tr("Error"));
                return;
            }
        }
        c = _curves->get(0);
    }

    QFile *ff = new QFile(path);
    if ( !ff->open(QIODevice::ReadWrite) ) {
        _pUI->showMessageBox(tr("Could not open file for writing."), tr("Error"));
        return;
    }

    int16_t curveNum = 0;
    if ( this->Act() == SELECT::all ) {
        if ( this->_curves->count() > PARAM::VOL_CMAX ) {
            _pUI->showMessageBox(tr("Vol file can only hold 50 curves."), tr("Error"));
            return;
        }
        curveNum = _curves->count();
    } else {
        curveNum = 1;
    }

    QByteArray ba;
    ba.append((char*) &curveNum, 2);

    QVector<std::string> names;
    for (int i=0; i<PARAM::VOL_CMAX; ++i) {
        std::string name;
        name.append(10, '\0');
        c = _curves->get(i);
        if ( c != nullptr ) {
            std::string str_name = c->CName().toStdString();
            for (uint ii=0; ii<str_name.length(); ++ii) {
                if (ii == 10) {
                    break;
                }
                name[ii] = str_name[ii];
            }
            bool has_name = true;
            while (has_name) {
                has_name = false;
                for (int ii=0; ii<names.length(); ++ii) {
                    if (name.compare(names[ii]) == 0) {
                        has_name = true;
                        int ni;
                        for (ni=0; ni<11; ++ni) {
                            if (ni == 10) {
                                ff->remove();
                                return;
                            }
                            if (name[ni] == '\0') {
                                name[ni] = '-';
                                break;
                            }
                        }
                    }
                }
            }
        }
        names.append(name);
    }
    int16_t offset = sizeof(int16_t) + (PARAM::VOL_CMAX*(10+sizeof(int16_t))) + ((PARAM::VOL_PMAX-2)*sizeof(int32_t));
    QVector<QByteArray> curveDatas;
    for (int16_t i=0; i<curveNum; ++i) {
        QByteArray cba;
        if (Act() > 0 ) {
            c = _curves->get(Act());
           cba = this->exportToVOLCurve(c);
        } else {
            c = _curves->get(i);
           cba = this->exportToVOLCurve(c);
        }
        offset = 2 + cba.size();
        curveDatas.append(cba);
        ba.append(names[i].c_str(), 10);
        ba.append((char*) &offset, sizeof(int16_t));
    }
    for (int i=curveNum; i<PARAM::VOL_CMAX; ++i) {
        ba.append(names[i].c_str(), 10);
        int16_t zer = 0;
        ba.append((char*)&zer, sizeof(int16_t));
    }
    int32_t param[(PARAM::VOL_PMAX-2)];
    for (int i=0; i<(PARAM::VOL_PMAX-2); ++i) {
        param[i] = c->Param(i);
    }
    param[PARAM::messc] = 0;
    ba.append((char*) param, (PARAM::VOL_PMAX-2)*sizeof(int32_t));
    for (int16_t i=0; i<curveNum; ++i) {
        ba.append((char*) &i, sizeof(int16_t));
        ba.append(curveDatas[i]);
    }
    ff->write(ba);
    ff->close();
}

QByteArray EAQtData::exportToVOLCurve(Curve *c)
{
    QByteArray ba;
    std::string name;
    name.append(10, '\0');
    if ( c != nullptr ) {
        std::string str_name = c->CName().toStdString();
        for (uint ii=0; ii<str_name.length(); ++ii) {
            if (ii == 10) {
                break;
            }
            name[ii] = str_name[ii];
        }
    }
    ba.append(name.c_str(), 10);
    std::string comment;
    comment.append(50, '\0');
    std::string str_comment = c->Comment().toStdString();
    for (uint i=0; i<str_comment.length();++i) {
        if ( i == 50 ) {
            break;
        }
        comment[i] = str_comment[i];
    }
    ba.append(comment.c_str(), 50);
    int16_t pDiff = (PARAM::VOL_PMAX-2);
    ba.append((char*) &pDiff, sizeof(int16_t));
    for (int16_t i=0; i<pDiff; ++i) {
        ba.append((char*) &i, sizeof(int16_t));
        if (i == PARAM::messc) {
            // Remove messc info from vol file
            int32_t z = 0;
            ba.append((char*) &z, sizeof(int32_t));
        } else {
            int32_t p = c->Param(i);
            ba.append((char*) &p, sizeof(int32_t));
        }
    }
    if (c->Param(PARAM::Ep) == c->getPotentialVector()->at(0)) {
        ba.append((char*) c->getCurrentVector()->data(), sizeof(double)*c->Param(PARAM::ptnr));
    } else{
        std::vector<double> v_rev = c->getCurrentVector()->toStdVector();
        std::reverse(v_rev.begin(), v_rev.end());
        ba.append((char*) v_rev.data(), sizeof(double)*c->Param(PARAM::ptnr));
    }
    return ba;
}

void EAQtData::exportToCSV(QString path)
{
    Curve *c;
    QByteArray eol = QByteArrayLiteral("\r\n");
    if (this->Act() >= 0) {
        c = _curves->get(_act);
        if ( c == nullptr) {
            _pUI->showMessageBox(tr("Could not find any curve to save."), tr("Error"));
            return;
        }
    }

    QFile *ff = new QFile(path);
    if ( !ff->open(QIODevice::ReadWrite) ) {
        _pUI->showMessageBox(tr("Could not open file for writing."), tr("Error"));
    }

    if (this->Act() == SELECT::all)  // wszystkie aktywne
    {
        int n = _curves->count();
        for ( int i = 0; i<n; ++i) {
            std::vector<std::string> csvCurve;
            csvCurve = exportToCSVCurve(_curves->get(i));
            ff->write(csvCurve[0].data(), csvCurve[0].size());
            ff->write(eol);
            ff->write(csvCurve[1].data(),csvCurve[1].size());
            if ( i+1 < n ) {
                ff->write(eol);
            }
        }
    } else { // jedna aktywna
            std::vector<std::string> csvCurve;
            csvCurve = exportToCSVCurve(c);
            ff->write(csvCurve[0].data(), csvCurve[0].size());
            ff->write(eol);
            ff->write(csvCurve[1].data(),csvCurve[1].size());
    }
    ff->close();
    delete ff;
}

std::vector<std::string> EAQtData::exportToCSVCurve(Curve *c)
{
    QVector<double> yvec = c->getYVector();
    QVector<double> xvec = c->getXVector();
    int n = yvec.size();
    std::string strY;
    std::string strX;
    std::vector<std::string> ret(2);
    strY.reserve(n*6);
    strX.reserve(n*4);
    for ( int i =0; i<n-1; ++i ) {
        strY.append(dispIforTXT(yvec[i]).toStdString() + ",");
        strX.append(dispEforTXT(xvec[i]).toStdString() + ",");
    }
    strY.append(dispIforTXT(yvec[n-1]).toStdString());
    strX.append(dispEforTXT(xvec[n-1]).toStdString());
    ret[0] = strX;
    ret[1] = strY;
    return ret;
}

void EAQtData::exportToTXT(QString path)
{
    Curve *c;
    int sizeData;
    QByteArray eol = QByteArrayLiteral("\r\n");
    if (this->Act() >= 0) {
        c = _curves->get(_act);
        if ( c == nullptr) {
            _pUI->showMessageBox(tr("Could not find any curve to save."), tr("Error"));
            return;
        }
        sizeData = c->getYVector().size();
    } else if ( Act() == SELECT::all ) {
        if ( (c=_curves->get(0)) == nullptr ) {
            _pUI->showMessageBox(tr("Could not find any curve to save."), tr("Error"));
            return;
        }
        int n = _curves->count();
        sizeData = _curves->get(0)->getYVector().size();
        for ( int i =0; i<n; ++i ) {
            if ( _curves->get(i)->getYVector().size() != sizeData ) {
                _pUI->showMessageBox(tr("Cannot export to TXT curves with different number of points."), tr("Error"));
                return;
            }
        }
    }

    QFile *ff = new QFile(path);
    if ( !ff->open(QIODevice::ReadWrite) ) {
        _pUI->showMessageBox(tr("Could not open file for writing."), tr("Error"));
    }

    std::vector<std::string> lines(sizeData);
    QVector<double> xvals = c->getXVector();
    for ( int i =0; i<sizeData;++i) {
        lines[i] = dispEforTXT(xvals[i]).toStdString();
    }

    if (this->Act() == SELECT::all)  // wszystkie aktywne
    {
        int n = _curves->count();
        for ( int i = 0; i<n; ++i) {
            std::vector<std::string> cLines = exportToTXTCurve(_curves->get(i));
            for ( int i = 0; i<sizeData; ++i) {
                lines[i].append("\t" + cLines[i]);
            }
        }
    } else { // jedna aktywna
        std::vector<std::string> cLines = exportToTXTCurve(c);
        for ( int i = 0; i<sizeData; ++i) {
            lines[i].append("\t" + cLines[i]);
        }
    }
    for ( int i =0; i<sizeData-1; ++i) {
        ff->write(lines[i].data(), lines[i].size());
        ff->write(eol);
    }
    ff->write(lines[sizeData-1].data(), lines[sizeData-1].size());
    ff->close();
    delete ff;
}

std::vector<std::string> EAQtData::exportToTXTCurve(Curve *c)
{
    QVector<double> vals = c->getYVector();
    int n = vals.size();
    std::vector<std::string> ret(n);
    for ( int i =0; i<n;++i) {
        ret[i] = dispIforTXT(vals[i]).toStdString();
    }
    return ret;
}

bool EAQtData::getWasLSV()
{
    return _wasLSVMeasurement;
}

const QVector<int16_t>& EAQtData::getPotentialProgram()
{
    return _PVParam_PotentialProgram;
}

void EAQtData::setPotentialProgram(QVector<int16_t> pp)
{
    _PVParam_PotentialProgram = pp;
}

void EAQtData::undoPrepare()
{
    if ( undoStruct._curves != NULL) {
        delete undoStruct._curves;
    }
    undoStruct._curves = new CurveCollection(_curves);
    undoStruct._act = _act;
    memcpy(undoStruct._LSVParam,_LSVParam,PARAM::PARAMNUM*sizeof(int32_t));
    memcpy(undoStruct._PVParam,_PVParam,PARAM::PARAMNUM*sizeof(int32_t));
    undoStruct.undoReady = true;
    emit(undoPrepared(true));
}

void EAQtData::undoExecute()
{
    if ( undoStruct.undoReady == true
     && _measurementGo == 0 ) {
        _curves->clear();
        delete _curves;
        _curves = undoStruct._curves;
        undoStruct._curves = NULL;
        for ( int i=0; i<_curves->count();++i) {
            _curves->get(i)->setPlot(_pUI->PlotAddQCPCurve());
        }
        _act=undoStruct._act;
        memcpy(_LSVParam,undoStruct._LSVParam,PARAM::PARAMNUM*sizeof(int32_t));
        memcpy(_PVParam,undoStruct._PVParam,PARAM::PARAMNUM*sizeof(int32_t));
        undoStruct.undoReady = false;
        emit(undoExecuted(true));
        _pUI->updateAll(true);
    }
}

QVector<double> EAQtData::generateTimeVector(int32_t* param, TYPES::vectorindex_t curve_number)
{
    int32_t tsize = param[PARAM::ptnr];
    QVector<double> vtime(param[PARAM::ptnr]);
    int32_t tp = param[PARAM::tp];
    int32_t tw = param[PARAM::tw];
    int32_t td_or_tk = param[PARAM::td];
    bool is_polar = (param[PARAM::mespv] == PARAM::mespv_polarography);
    bool is_special_cgmde = (
                (param[PARAM::electr] == PARAM::electr_cgmde)
                || (param[PARAM::electr] == PARAM::electr_microCgmde)
            ) && (param[PARAM::cgmdeMode] > PARAM::cgmdeMode_dropBeforeMes);

    int32_t tstep = 0;
    int32_t time = 0;
    if (!is_polar && !is_special_cgmde) {
        if (param[PARAM::method] != PARAM::method_lsv) {
            tstep = 2*(tp + tw);
            time = tstep;
        } else {
            tstep = MEASUREMENT::LSVtime[param[PARAM::dEdt]];
            time = tstep;
        }
    } else if (is_polar) {
        tstep = td_or_tk;
        time = tstep;
    } else if (is_special_cgmde) {
        if (param[PARAM::cgmdeMode] == PARAM::cgmdeMode_pulseBeforeEachPoint) {
            tstep = 2*(tp + tw) + param[PARAM::valveTime];
            time = tstep;
        } else if (param[PARAM::cgmdeMode] == PARAM::cgmdeMode_dropBeforeEachPoint) {
            tstep = 2*(tp + tw) + param[PARAM::knockTime] + param[PARAM::valveCntr] * (param[PARAM::valveTime] + param[PARAM::valveDelay]);
            time = tstep;
        } else {
            tstep = 2*(tp + tw);
            time = tstep;
        }
    }

    for (int i=0; i<tsize; ++i) {
        vtime[i] = time + (tsize * tstep * curve_number);
        time += tstep;
    }
    return vtime;
}
