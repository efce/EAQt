
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
#include "eaqtmousehandler.h"
#include "eaqtsignalprocessing.h"


EAQtMouseHandler::EAQtMouseHandler(EAQtDataInterface* pd, EAQtUIInterface* pui) : QObject()
{
    this->_pData = pd;
    this->_pUI = pui;
    /// Add Cursors to Graph ///
    _vCursors.resize(cl_LAST);
    for (int i=0; i<cl_LAST; ++i) {
        _vCursors[i] = pui->PlotAddCursor();
        _vCursors[i]->setVisible(false);
    }
    /// Setup Curors ///
    _structRPL.baseLine = NULL;
    this->setDefaults();
}

EAQtMouseHandler::~EAQtMouseHandler(void)
{
    if ( _vCursors.size() > 0 ) {
        for ( int i = 0; i< _vCursors.size(); ++i ) {
            delete _vCursors[i];
        }
    }
}

void EAQtMouseHandler::onSelectionChanged()
{
    switch (this->_activeMode) {
    case mm_dataCursor:
        if ( this->_pData->Act() >= 0 ) {
            this->_vCursors[cl_movingCursor]->setSnapTo(this->_pData->getCurves()->get(this->_pData->Act())->getPlot());
            this->_vCursors[cl_movingCursor]->move(this->_vCursors[cl_movingCursor]->getX());
            this->updateCursorLabel();
        } else {
            this->ChangeMouseMode(mm_normal,uf_none);
        }
        break;
    case mm_nearestYonPlot:
        if ( this->_pData->Act() >= 0 ) {
            this->_vCursors[cl_snapXonPlot]->setSnapTo(this->_pData->getCurves()->get(this->_pData->Act())->getPlot());
        } else {
            this->ChangeMouseMode(mm_normal,uf_none);
        }
        break;
    default:
        //this->ChangeMouseMode(mm_normal,uf_none);
        break;
    }
}

void EAQtMouseHandler::setDefaults()
{
    this->_pUI->enableAll();
    this->_pUI->clearPeaks();
    this->_pUI->PlotSetInteraction(_plotInteractions.plotDefaultInteractions);
    _pUI->PlotConnectMouse();
    this->_timesPressed = 0;
    this->_activeMode = this->mm_normal;
    this->_userFunction = this->uf_none;
    for (int i=0;i<cl_LAST;i++) {
        _vCursors[i]->setVisible(false);
    }
    this->_vCursors[cl_movingCursor]->setMarker(QCPScatterStyle::ssCircle);
    this->_vCursors[cl_relativeParameterCursor]->setMarker(QCPScatterStyle::ssCross);
    this->_vCursors[cl_relativeParameterCursor]->setColor(QColor(0,255,0));
    for (int i=cl_multipleSelect1;i<=cl_multipleSelect4;i++) {
        this->_vCursors[i]->setMarker(QCPScatterStyle::ssStar);
    }
    for ( int i=0; i<5; ++i) {
        this->_pUI->setLowLabelText(i,"");
    }
    this->_pUI->updateAll(false);
}

bool EAQtMouseHandler::userCanUseCurveList()
{
    switch(_activeMode) {
        case mm_normal:
        case mm_nearestYonPlot:
            return true;
        default:
            return false;
    }
}

int32_t EAQtMouseHandler::GetCursorPointIndex(cursorsList cursNum)
{
    return this->_vCursors[static_cast<int>(cursNum)]->getIndex();
}

double EAQtMouseHandler::GetCursorX(cursorsList cursNum)
{
    return this->_vCursors[static_cast<int>(cursNum)]->getX();
}

double EAQtMouseHandler::GetCursorY(cursorsList cursNum)
{
    return this->_vCursors[static_cast<int>(cursNum)]->getY();
}

bool EAQtMouseHandler::GetCursorVisible(cursorsList cursNum)
{
    return this->_vCursors[static_cast<int>(cursNum)]->getVisible();
}

void EAQtMouseHandler::SetCursorsOnActPlot()
{
    if ( this->_pData->Act() != SELECT::none ) {
        int act;
        if ( this->_pData->Act() == SELECT::all ) {
            act = 0;
        } else {
            act = this->_pData->Act();
        }
        QCPCurve* gr = this->_pData->getCurves()->get(act)->getPlot();
        if ( gr->visible() ) {
            for (int i=0;i<cl_LAST; ++i) {
                this->_vCursors[i]->setSnapTo(gr);
            }
        }
    } else {
        return;
    }
}

void EAQtMouseHandler::toggleMovingCursor(QCPCurve* gr) {
    if ( this->_vCursors[cl_movingCursor]->getVisible() ) {
        this->_vCursors[cl_movingCursor]->setVisible(false);
    } else {
        this->_vCursors[cl_movingCursor]->setSnapTo(gr);
        this->_vCursors[cl_movingCursor]->move(this->_pUI->PlotGetXMiddle());
        this->_vCursors[cl_movingCursor]->setVisible(true);
    }
}

void EAQtMouseHandler::UpdateLocation(double* MX, double* MY)
{
    if ( this->_activeMode == mm_normal ) {
        if (this->_pData->getXAxis() == XAXIS::potential ) {
            this->_pUI->setLowLabelText(0,tr("Mouse cursor:") +  " E=" + this->_pData->dispE(*MX) + " I=" + this->_pData->dispI(*MY));
        } else if (this->_pData->getXAxis() == XAXIS::time ) {
            this->_pUI->setLowLabelText(0,tr("Mouse cursor:") + " t=" + this->_pData->dispTIME1(*MX) + " I=" + this->_pData->dispI(*MY));
        } else {
             this->_pUI->setLowLabelText(0,tr("Mouse cursor:") + " sample#=" + this->_pData->dispNR(*MX) + " I=" + this->_pData->dispI(*MY));
        }
    } else if ( this->_activeMode == mm_relativeParamters ) {
        if ( this->_structRPL.followCursor ) {
            // obliczenie prostej y=ax+b;
            // this->structRPL.pointY=this->structRPL.pointX*a+b;
            // MY = MX*a+b;
            this->_structRPL.baseLineA = (this->_structRPL.pointY-*MY) / (this->_structRPL.pointX-*MX);
            this->_structRPL.baseLineB = this->_structRPL.pointY - (this->_structRPL.baseLineA*this->_structRPL.pointX);
            double x1 = 1;
            double x2 = 2;
            double y1 = this->_structRPL.baseLineA * x1 + this->_structRPL.baseLineB;
            double y2 = this->_structRPL.baseLineA * x2 + this->_structRPL.baseLineB;
            this->_structRPL.baseLine->point1->setCoords(x1,y1);
            this->_structRPL.baseLine->point2->setCoords(x2,y2);
            this->_structRPL.baseLine->setVisible(true);
            this->_pUI->PlotGetLayers()->Markers->replot();
        } else if ( this->_structRPL.staticLine == true ) {
            this->_vCursors[cl_relativeParameterCursor]->move(*MX);
            double dCurrentRelativeValue = this->_vCursors[cl_relativeParameterCursor]->getY() - (this->_structRPL.baseLineA * *MX + this->_structRPL.baseLineB);
            if ( this->_pData->getXAxis() == XAXIS::potential ) {
                this->_pUI->setLowLabelText(0, tr("Relative position: ") + _pData->dispE(*MX) + "  " + _pData->dispI(dCurrentRelativeValue) + " ");
            } else if ( this->_pData->getXAxis() == XAXIS::time ) {
                this->_pUI->setLowLabelText(0, tr("Relative position: ") + _pData->dispTIME1(*MX) + "  " + _pData->dispI(dCurrentRelativeValue) + " ");
            }
            this->_pUI->PlotGetLayers()->Markers->replot();
        }
    }
}

void EAQtMouseHandler::ChangeMouseMode(mouseMode mm, userFunctions uf)
{
    QPen qp;
    this->setDefaults();
    this->_activeMode = mm;
    this->_userFunction = uf;
    this->_isFunctionSetupNeeded = true;
    this->callUserFunction();

    switch ( mm ) {
    default:
    case mm_normal:
        this->_pUI->enableAll();
        this->_pUI->PlotSetInteraction(this->_plotInteractions.plotDefaultInteractions);
        break;

    case mm_place2markers:
        this->_pUI->disableButtonsAndTable();
        this->SetCursorsOnActPlot();
        this->_pUI->PlotSetInteraction(this->_plotInteractions.plotCursorInteractions);
        break;


    case mm_place4markers:
        this->_pUI->disableButtonsAndTable();
        this->SetCursorsOnActPlot();
        this->_pUI->PlotSetInteraction(this->_plotInteractions.plotCursorInteractions);
        break;

    case mm_dataCursor:
        this->_pUI->disableButtonsOnly();
        this->_pUI->PlotSetInteraction(this->_plotInteractions.plotCursorInteractions);
        this->toggleMovingCursor(this->_pData->getCurves()->get(this->_pData->Act())->getPlot());
        break;

    case mm_relativeParamters:
        this->_pUI->disableButtonsAndTable();
        this->_structRPL.followCursor = false;
        this->_structRPL.staticLine = false;
        this->SetCursorsOnActPlot();
        this->_pUI->PlotSetInteraction(this->_plotInteractions.plotCursorInteractions);
        if ( this->_structRPL.baseLine == NULL ) {
            this->_structRPL.baseLine = this->_pUI->PlotAddLine();
        }
        qp = this->_structRPL.baseLine->pen();
        qp.setColor(QColor(0, 200, 50));
        this->_structRPL.baseLine->setPen(qp);
        this->_pUI->PlotGetLayers()->Markers->replot();
        break;

    case mm_measurement:
        this->_pUI->disableButtonsAndTable();
        this->_pUI->PlotSetInteraction(this->_plotInteractions.plotNoInteractions);
        _pUI->PlotDisconnectMouse();
        break;
    }
}

void EAQtMouseHandler::ForwardClick(double MouseCursorX,double MouseCursorY)
{
    if ( this->_activeMode == mm_measurement ) {
        return;
    }

    QString m_String;
    this->dClickCoordinates[0] = MouseCursorX;
    this->dClickCoordinates[1] = MouseCursorY;
    //int nOldTimesPressed = this->nTimesPressed;
    this->_timesPressed++;
    //this->pDoc->view->MessageBox(this->pDoc->_NR(this->nTimesPressed));

    switch ( _activeMode ) {

    case mm_measurement:
    //case mm_unset:
        this->_timesPressed = 0;
        return;

    default:
    case mm_normal:
        return;
        break;

    case mm_dataCursor:
        this->_timesPressed = 0;
        this->_vCursors[cl_movingCursor]->move(MouseCursorX);
        this->updateCursorLabel();
        this->_pUI->updateAll(false);
        break;

    case mm_place2markers:
        if ( this->_timesPressed > 3 ) {
            this->callUserFunction();
            this->setDefaults();
            this->_pUI->updateAll(false);
        } else if( this->_timesPressed == 3 ) { // wyswietl operacje
            this->callUserFunction();
            this->_pUI->updateAll(false);
        } else if ( this->_timesPressed < 3 ) {
            int nCurNum = cl_multipleSelect1-1+this->_timesPressed;
            this->_vCursors[nCurNum]->move(MouseCursorX);
            this->_vCursors[nCurNum]->setVisible(true);
            QString strDisp;
            for(int ii=cl_multipleSelect1; ii<=nCurNum ; ii++) {
                double nPos = this->GetCursorX(static_cast<cursorsList>(ii));

                //m_String.LoadString(IDS_kursor); // Kursor
                if ( _pData->getXAxis() == XAXIS::potential ) {
                    strDisp += m_String + _pData->dispNR(ii-cl_multipleSelect1+1) + ": " + this->_pData->dispE(nPos) + "     ";
                } else if ( _pData->getXAxis() == XAXIS::time ) {
                    strDisp += m_String + _pData->dispNR(ii-cl_multipleSelect1+1) + ": " + this->_pData->dispTIME1(nPos) + "     ";
                } else if ( _pData->getXAxis() == XAXIS::nonaveraged ) {
                    //TODO
                }
            }
            this->_pUI->setLowLabelText(0,strDisp);
            this->_pUI->updateAll(false);
        }
        break;

    case mm_place4markers:
        if ( this->_timesPressed > 5 ) {
            this->callUserFunction();
            this->setDefaults();
            this->_pUI->updateAll(false);
        } else if( this->_timesPressed == 5 ) { // wyswietl operacje
            this->callUserFunction();
            this->_pUI->updateAll(false);
        } else if ( this->_timesPressed < 5 ) {
            this->callUserFunction();
            int nCurNum = cl_multipleSelect1-1+this->_timesPressed;
            this->_vCursors[nCurNum]->move(MouseCursorX);
            this->_vCursors[nCurNum]->setVisible(true);
            QString strDisp;
            for(int ii=cl_multipleSelect1; ii<=nCurNum ; ii++) {
                double nPos = this->GetCursorX(static_cast<cursorsList>(ii));
                //m_String.LoadString(IDS_kursor); // Kursor
                if ( _pData->getXAxis() == XAXIS::potential )
                    strDisp += m_String + _pData->dispNR(ii-cl_multipleSelect1+1) + ": " + this->_pData->dispE(nPos) + "     ";
                else if ( _pData->getXAxis() == XAXIS::time )
                    strDisp += m_String + _pData->dispNR(ii-cl_multipleSelect1+1) + ": " + this->_pData->dispTIME1(nPos) + "     ";
                else {
                    //TODO
                }
            }
            this->_pUI->setLowLabelText(0,strDisp);
            this->_pUI->updateAll(false);
        }
        break;

    case mm_nearestYonPlot:
        this->_vCursors[cl_snapXonPlot]->setSnapTo(_pData->getCurves()->get(_pData->Act())->getPlot());
        this->_vCursors[cl_snapXonPlot]->move(MouseCursorX);
        this->_vCursors[cl_snapXonPlot]->setVisible(false);
        this->_timesPressed = 0;
        this->callUserFunction();
        break;

    case mm_relativeParamters:
        if ( this->_timesPressed == 1 ) {
            this->_structRPL.baseLine->setVisible(false);
            this->_structRPL.pointX = this->dClickCoordinates[0];
            this->_structRPL.pointY = this->dClickCoordinates[1];
            this->_structRPL.followCursor = true;
            this->_structRPL.staticLine = false;
        } else if ( this->_timesPressed == 2 ) {
            this->_structRPL.followCursor = false;
            this->_structRPL.staticLine = true;
            this->_vCursors[cl_relativeParameterCursor]->move(this->dClickCoordinates[0]);
            this->_vCursors[cl_relativeParameterCursor]->setVisible(true);
        } else if ( this->_timesPressed > 2 ) {
            this->_structRPL.followCursor = false;
            this->_structRPL.staticLine = false;
            this->_structRPL.baseLine->setVisible(false);
            this->setDefaults();
        }
        this->_pUI->PlotGetLayers()->Markers->replot();
        break;
    }
}

void EAQtMouseHandler::BackClick()
{
    if ( this->_activeMode == mm_measurement ) {
        return;
    }
    QString m_String;
    //int nOldTimesPressed = this->nTimesPressed;
    this->_timesPressed--;
    if ( this->_timesPressed <= -1 ) {
        this->setDefaults();
        return;
    }

    switch ( this->_activeMode ) {

    case mm_place2markers:
        if( this->_timesPressed == 2 ) {
            this->callUserFunction();
            this->_pUI->updateAll(false);
        } else if ( this->_timesPressed < 2 ) {
            int nCurNum = cl_multipleSelect1+this->_timesPressed;
            this->_vCursors[nCurNum]->setVisible(false);
            QString strDisp;
            for(int ii=cl_multipleSelect1; ii<=nCurNum-1 ; ii++) {
                double nPos = this->GetCursorX(static_cast<cursorsList>(ii));
                //m_String.LoadString(IDS_kursor); // Kursor
                if ( _pData->getXAxis() == XAXIS::potential ) {
                    strDisp += m_String + _pData->dispNR(ii-cl_multipleSelect1+1) + ": " + this->_pData->dispE(nPos) + "     ";
                } else if ( _pData->getXAxis() == XAXIS::time ) {
                    strDisp += m_String + _pData->dispNR(ii-cl_multipleSelect1+1) + ": " + this->_pData->dispTIME1(nPos) + "     ";
                } else {
                    //TODO:
                }
            }
            this->_pUI->setLowLabelText(0,strDisp);
            this->_pUI->updateAll(false);
        }
        break;

    case mm_place4markers:
        if( this->_timesPressed == 4 ) {
            this->callUserFunction();
            this->_pUI->updateAll(false);
        } else if ( this->_timesPressed < 4 ) {
            int nCurNum = cl_multipleSelect1+this->_timesPressed;
            this->_vCursors[nCurNum]->setVisible(false);
            QString strDisp;
            for(int ii=cl_multipleSelect1; ii<=nCurNum-1 ; ii++) {
                double nPos = this->GetCursorX(static_cast<cursorsList>(ii));
                //m_String.LoadString(IDS_kursor); // Kursor
                if ( _pData->getXAxis() == XAXIS::potential) {
                    strDisp += m_String + _pData->dispNR(ii-cl_multipleSelect1+1) + ": " + this->_pData->dispE(nPos) + "     ";
                } else if ( _pData->getXAxis() == XAXIS::time) {
                    strDisp += m_String + _pData->dispNR(ii-cl_multipleSelect1+1) + ": " + this->_pData->dispTIME1(nPos) + "     ";
                } else {
                    //TODO:
                }
            }
            this->_pUI->setLowLabelText(0,strDisp);
            this->_pUI->updateAll(false);
        }
        break;

    case mm_relativeParamters:
        if ( this->_timesPressed == 0 ) {
            this->_structRPL.followCursor = false;
            this->_structRPL.staticLine = false;
            this->_structRPL.baseLine->setVisible(false);
        } else if ( this->_timesPressed == 1 ) {
            this->_structRPL.followCursor = true;
            this->_structRPL.staticLine = false;
            this->_structRPL.baseLine->setVisible(true);
            this->_vCursors[cl_relativeParameterCursor]->setVisible(false);
        }
        this->_pUI->PlotGetLayers()->Markers->replot();
        break;

    default:
        return;
    }
}

void EAQtMouseHandler::callUserFunction()
{
    /*********************************************************************************
    * Tworzenie w³asnych operacji mysz¹:
    * Krok 1: Dodac do zmiennej enum userFunctions nowa pozycje z nazwa nowej funkcji
    * Krok 2: Wybrac odpowiedni tryb myszy z enum mouseMode
    *** np.: place4markers - funkcja potrzebuje 4 wspolrzednych z kursorow
    ***      place2markers - funkcja potrzebuje 2 wspolrzednych z kursorow
    ***	     Funkcja jest wywolywana 3 razy ! przed polozeniem jakiegokolwiek kursora
    ***		 po po³o¿eniu ostatniego i po kolejnym kliknieciu !
    * Krok 3: Stworzenie odpowiedniego case w switchu
    * Krok 4: utworzenie struktury w casie
    * if ( this->bFunctionSetupNeeded ) {
    *	this->bFunctionSetupNeeded = false;
    **	tutaj maja byc operacje wykonywane przed procedura np.:
    **  this->pDoc->view->disableAllButtons(); ** wylaczenie klawiszy w oknie
    **  return; ** !!!
    * }
    * if (  this->nTimesPressed < [ilosc kursorow+1] ) {
    ***	   UWAGA: Jezeli podczas [ilosc kursorow+1] klikniecia sa robione
    ***	   jakies zmiany na ekrania tutaj musz¹ byæ cofniête!
    * } else if ( this->nTimesPressed == [ilosc kursorow+1] ) {
    ***	   Operacje ktore maja byc wykonane po polozeniu ostatniego kursora
    * } else if ( this->nTimesPressed > [ilosc kursorow+1] ) {
    ***	   Operacje ktore maja byc wykonane po kliknieciu kolejnym
    * }
    *
    * W kazdym momencie umieszczenie this->setDefaults() anuluje ukladanie
    * kursorow i powrot do normalnego trybu ekranu. (np. jezeli nie potrzebna
    * jest obsluga:
    * if ( this->nTimesPressed > [ilosc kursorow+1] )
    * to w bloku:
    * if ( this->nTimesPressed == [ilosc kursorow+1] )
    * nalezy umiescic this->setDefaults()
    * Dostêp do CEAProDoc: this->pDoc
    * Dostêp do MesForm: this->pDoc->view
    *********************************************************************************/

    switch ( this->_userFunction ) {

    case uf_background_correction:
        if ( this->_isFunctionSetupNeeded ) {
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText(tr("Background correction: set four markers to generate fit"));
            this->_pUI->updateAll(false);
            return;
        }
        if ( this->_timesPressed < 5 ) {
            this->_pUI->updateAll(false);
            this->_pData->getProcessing()->hideBackground();
        } else if ( this->_timesPressed == 5 ) {
            int act = 0;
            if ( _pData->Act() != SELECT::all ) {
                act = _pData->Act();
            }
            try {
                this->_pData->getProcessing()->generateBackground(
                        _pData->getCurves()->get(act)
                        ,this->GetCursorPointIndex(cl_multipleSelect1)
                        ,this->GetCursorPointIndex(cl_multipleSelect2)
                        ,this->GetCursorPointIndex(cl_multipleSelect3)
                        ,this->GetCursorPointIndex(cl_multipleSelect4)
                    );
            } catch (int e) {
                _pUI->showMessageBox(tr("Data must have at least %1 points").arg(e), tr("Error"));
                _timesPressed = 4;
                return;
            }
            this->_pData->getProcessing()->showBackground();
            this->_pUI->PlotGetLayers()->Markers->replot();
        } else if ( this->_timesPressed > 5 ) {
            _pData->undoPrepare();
            CurveCollection* _curves = _pData->getCurves();
            if ( _pData->Act() == SELECT::all ) {
                for ( int i=0; i<_curves->count(); ++i ) {
                    Curve* c = _curves->get(i);
                    QVector<double> bkg_y = _pData->getProcessing()->generateBackground(
                                c
                                ,this->GetCursorPointIndex(cl_multipleSelect1)
                                ,this->GetCursorPointIndex(cl_multipleSelect2)
                                ,this->GetCursorPointIndex(cl_multipleSelect3)
                                ,this->GetCursorPointIndex(cl_multipleSelect4)
                            );
                    _pData->getProcessing()->subtractBackground(c, bkg_y);

                }
            } else {
                Curve* c = _curves->get(this->_pData->Act());
                QVector<double> bkg_y = _pData->getProcessing()->generateBackground(
                            c
                            ,this->GetCursorPointIndex(cl_multipleSelect1)
                            ,this->GetCursorPointIndex(cl_multipleSelect2)
                            ,this->GetCursorPointIndex(cl_multipleSelect3)
                            ,this->GetCursorPointIndex(cl_multipleSelect4)
                        );
                _pData->getProcessing()->subtractBackground(c, bkg_y);

            }
            this->_pData->getProcessing()->hideBackground();
            this->_pUI->updateAll(false);
        }
        break;

    case uf_calibration_data:
        if ( this->_isFunctionSetupNeeded ) { // tylko na poczatku
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText(tr("Calibration: set two markers on the plot")); // DANE DO KALIBRACJI: Ustaw dwa wskaŸniki. U¿yj lewego przycisku myszy, aby dodaæ wskaŸnik. U¿yj prawego, aby cofn¹æ operacjê.
            this->_pUI->updateAll(false);
            return;
        }
        if ( this->_timesPressed > 2 ) {
            QVector<std::array<int,2>> coords;
            coords.resize(_pData->getCurves()->count());
            for ( int i =0; i< coords.size(); ++i) {
                _vCursors[cl_multipleSelect1]->setSnapTo(_pData->getCurves()->get(i)->getPlot());
                _vCursors[cl_multipleSelect2]->setSnapTo(_pData->getCurves()->get(i)->getPlot());
                _vCursors[cl_multipleSelect1]->move(_vCursors[cl_multipleSelect1]->getX());
                _vCursors[cl_multipleSelect2]->move(_vCursors[cl_multipleSelect2]->getX());
                if ( this->GetCursorX(cl_multipleSelect1) < this->GetCursorX(cl_multipleSelect2) ) {
                    coords[i][0] = this->GetCursorPointIndex(cl_multipleSelect1);
                    coords[i][1] = this->GetCursorPointIndex(cl_multipleSelect2);
                } else {
                    coords[i][1] = this->GetCursorPointIndex(cl_multipleSelect1);
                    coords[i][0] = this->GetCursorPointIndex(cl_multipleSelect2);
                }
            }
            int act = _pData->Act();
            if ( act < 0 ) {
                act = 0;
            }
            _vCursors[cl_multipleSelect1]->setSnapTo(_pData->getCurves()->get(act)->getPlot());
            _vCursors[cl_multipleSelect2]->setSnapTo(_pData->getCurves()->get(act)->getPlot());
            this->_pData->getProcessing()->calibrationData(coords);
            this->setDefaults();
            this->_pUI->PlotGetLayers()->Markers->replot();
        }
        break;

    case uf_statistic_data:
        if ( this->_isFunctionSetupNeeded ) { // tylko na poczatku
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText(tr("Analysis: set two markers on the plot")); // ANALIZA PIKU/FALI: Ustaw dwa wskaŸniki. U¿yj lewego przycisku myszy dodaæ wskaŸnik. U¿yj prawego, aby cofn¹æ operacjê.
            this->_pUI->updateAll(false);
            return;
        }
        if ( this->_timesPressed > 2 ) {
            QVector<std::array<int,2>> coords;
            coords.resize(_pData->getCurves()->count());
            for ( int i =0; i< coords.size(); ++i) {
                _vCursors[cl_multipleSelect1]->setSnapTo(_pData->getCurves()->get(i)->getPlot());
                _vCursors[cl_multipleSelect2]->setSnapTo(_pData->getCurves()->get(i)->getPlot());
                _vCursors[cl_multipleSelect1]->move(_vCursors[cl_multipleSelect1]->getX());
                _vCursors[cl_multipleSelect2]->move(_vCursors[cl_multipleSelect2]->getX());
                if ( this->GetCursorX(cl_multipleSelect1) < this->GetCursorX(cl_multipleSelect2) ) {
                    coords[i][0] = this->GetCursorPointIndex(cl_multipleSelect1);
                    coords[i][1] = this->GetCursorPointIndex(cl_multipleSelect2);
                } else {
                    coords[i][1] = this->GetCursorPointIndex(cl_multipleSelect1);
                    coords[i][0] = this->GetCursorPointIndex(cl_multipleSelect2);
                }
            }
            _vCursors[cl_multipleSelect1]->setSnapTo(_pData->getCurves()->get(_pData->Act())->getPlot());
            _vCursors[cl_multipleSelect2]->setSnapTo(_pData->getCurves()->get(_pData->Act())->getPlot());
            this->_pData->getProcessing()->curvesStats(coords);
            this->setDefaults();
            this->_pUI->updateAll(false);
        }
        break;

    case uf_reg_data:
        if ( this->_isFunctionSetupNeeded ) { // tylko na poczatku
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText(tr("Regression: set two markers on the plot")); // ANALIZA REGRESJI: Ustaw dwa wskaŸniki. U¿yj lewego przycisku myszy, aby dodaæ wskaŸnik. U¿yj prawego, aby cofn¹æ operacjê.
            this->_pUI->updateAll();
            return;
        }
        if ( this->_timesPressed > 2 ) {
            int cl1, cl2;
            if ( this->GetCursorX(cl_multipleSelect1) < this->GetCursorX(cl_multipleSelect2) ) {
                cl1 = this->GetCursorPointIndex(cl_multipleSelect1);
                cl2 = this->GetCursorPointIndex(cl_multipleSelect2);
            } else {
                cl2 = this->GetCursorPointIndex(cl_multipleSelect1);
                cl1 = this->GetCursorPointIndex(cl_multipleSelect2);
            }
            //this->pData->OnInterAnRegMA(cl1, cl2);
            this->setDefaults();
            this->_pUI->updateAll();
        }
        break;

    case uf_move_curve:
        if ( this->_isFunctionSetupNeeded ) { // tylko na poczatku
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText(tr("Move: use left mouse button to move the curve up or down")); // PRZESUWANIE KRZYWEJ: U¿yj lewego przycisku myszy aby przesun¹æ aktywn¹ krzyw¹. U¿yj prawego aby zakoñczyæ operacjê.
            this->_pUI->updateAll();
            return;
        }
        //this->pData->PrepareUndoCurve();
        double dMV;
        this->SetCursorsOnActPlot();
        _vCursors[cl_snapXonPlot]->move(dClickCoordinates[0]);
        dMV = this->dClickCoordinates[1] - this->GetCursorY(cl_snapXonPlot);
        this->_pData->getProcessing()->shiftCurve(dMV);
        this->_pUI->updateAll();
        break;

    case uf_relative_height:
        if ( this->_isFunctionSetupNeeded ) { // tylko na poczatku
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText(tr("Relative height: use left mouse button to set first and second point for baseline")); // WYSOKOŒÆ WZGLÊDNA: U¿yj lewego przycisku myszy aby ustawiæ pierwszy punkt krzywej bazowej. U¿yj prawego, aby cofn¹æ operacjê.
            this->_pUI->PlotGetLayers()->Markers->replot();
            return;
        }
        break;

    default:
        return;
    }
}

bool EAQtMouseHandler::wantsClicks()
{
    if ( this->_activeMode == mm_normal ) {
        return false;
    } else {
        return true;
    }
}

QVector<EAQtPlotCursor*>* EAQtMouseHandler::getCursors()
{
    return &_vCursors;
}

void EAQtMouseHandler::updateCursorLabel()
{
    this->_pUI->setLowLabelText(0,tr("Data cursor:")
                               + " nr=" + _pData->dispNR(_vCursors[cl_movingCursor]->getIndex())
                               + " E=" + _pData->dispE(this->_vCursors[cl_movingCursor]->getX())
                               + " i=" + _pData->dispI(this->_vCursors[cl_movingCursor]->getY()));
}
