
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
}

void EAQtMouseHandler::onSelectionChanged()
{
    switch (this->_activeMode) {
    case mm_dataCursor:
        if ( this->_pData->Act() >= 0 ) {
            this->_vCursors[cl_movingCursor]->setSnapTo(this->_pData->getCurves()->get(this->_pData->Act())->getPlot());
            this->_vCursors[cl_movingCursor]->move(this->_vCursors[cl_movingCursor]->getX());
            //TODO: update text on lowlabel
        }
        break;
    default:
        break;
    }
}

void EAQtMouseHandler::setDefaults()
{
    this->_pUI->enableAll();
    this->_pUI->PlotSetInteraction(_plotInteractions.plotDefaultInteractions);
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
    this->_pUI->updateAll();
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

uint EAQtMouseHandler::GetCursorPointIndex(cursorsList cursNum)
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
    if ( this->_pData->Act() != SELECT::all
    && this->_pData->Act() != SELECT::none ) {
        QCPGraph* gr = this->_pData->getCurves()->get(this->_pData->Act())->getPlot();
        if ( gr->visible() ) {
            for (int i=0;i<cl_LAST; ++i) {
                this->_vCursors[i]->setSnapTo(gr);
            }
        }
    } else {
        return;
    }
}

void EAQtMouseHandler::toggleMovingCursor(QCPGraph* gr) {
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
            this->_pUI->setLowLabelText(0,tr("Mouse cursor:") +  " E=" + this->_pData->_E(*MX) + " I=" + this->_pData->_I(*MY));
        } else if (this->_pData->getXAxis() == XAXIS::time ) {
            this->_pUI->setLowLabelText(0,tr("Mouse cursor:") + " t=" + this->_pData->_TIME1(*MX) + " I=" + this->_pData->_I(*MY));
        } else {
             this->_pUI->setLowLabelText(0,tr("Mouse cursor:") + " sample#=" + this->_pData->_NR(*MX) + " I=" + this->_pData->_I(*MY));
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
            this->_pUI->PlotReplot();
        } else if ( this->_structRPL.staticLine == true ) {
            this->_vCursors[cl_relativeParameterCursor]->move(*MX);
            double dCurrentRelativeValue = this->_vCursors[cl_relativeParameterCursor]->getY() - (this->_structRPL.baseLineA * *MX + this->_structRPL.baseLineB);
            if ( this->_pData->getXAxis() == XAXIS::potential ) {
                this->_pUI->setLowLabelText(0, tr("Relative position: ") + _pData->_E(*MX) + "  " + _pData->_I(dCurrentRelativeValue) + " ");
            } else if ( this->_pData->getXAxis() == XAXIS::time ) {
                this->_pUI->setLowLabelText(0, tr("Relative position: ") + _pData->_TIME1(*MX) + "  " + _pData->_I(dCurrentRelativeValue) + " ");
            }
            this->_pUI->PlotReplot();
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
        break;

    case mm_measurement:
        this->_pUI->disableButtonsAndTable();
        this->_pUI->PlotSetInteraction(this->_plotInteractions.plotNoInteractions);
        //this->pUI->MeasurementSetup();
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
        this->_pUI->setLowLabelText(0,tr("Data cursor:")
                                   + " nr=" + _pData->_NR(_vCursors[cl_movingCursor]->getIndex())
                                   + " E=" + _pData->_E(this->_vCursors[cl_movingCursor]->getX())
                                   + " I=" + _pData->_I(this->_vCursors[cl_movingCursor]->getY()));
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
                    strDisp += m_String + _pData->_NR(ii-cl_multipleSelect1+1) + ": " + this->_pData->_E(nPos) + "     ";
                } else if ( _pData->getXAxis() == XAXIS::time ) {
                    strDisp += m_String + _pData->_NR(ii-cl_multipleSelect1+1) + ": " + this->_pData->_TIME1(nPos) + "     ";
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
                    strDisp += m_String + _pData->_NR(ii-cl_multipleSelect1+1) + ": " + this->_pData->_E(nPos) + "     ";
                else if ( _pData->getXAxis() == XAXIS::time )
                    strDisp += m_String + _pData->_NR(ii-cl_multipleSelect1+1) + ": " + this->_pData->_TIME1(nPos) + "     ";
                else {
                    //TODO
                }
            }
            this->_pUI->setLowLabelText(0,strDisp);
            this->_pUI->updateAll(false);
        }
        break;

    case mm_nearestYonPlot:
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
                    strDisp += m_String + _pData->_NR(ii-cl_multipleSelect1+1) + ": " + this->_pData->_E(nPos) + "     ";
                } else if ( _pData->getXAxis() == XAXIS::time ) {
                    strDisp += m_String + _pData->_NR(ii-cl_multipleSelect1+1) + ": " + this->_pData->_TIME1(nPos) + "     ";
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
                    strDisp += m_String + _pData->_NR(ii-cl_multipleSelect1+1) + ": " + this->_pData->_E(nPos) + "     ";
                } else if ( _pData->getXAxis() == XAXIS::time) {
                    strDisp += m_String + _pData->_NR(ii-cl_multipleSelect1+1) + ": " + this->_pData->_TIME1(nPos) + "     ";
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
            this->_pUI->setStatusText("IDS_mysz1");
            this->_pUI->updateAll(false);
            return;
        }
        if ( this->_timesPressed < 5 ) {
            this->_pUI->updateAll(false);
            this->_pData->getProcessing()->hideBackground();
        } else if ( this->_timesPressed == 5 ) {
            this->_pData->getProcessing()->generateBackground(
                    this->GetCursorPointIndex(cl_multipleSelect1)
                    ,this->GetCursorPointIndex(cl_multipleSelect2)
                    ,this->GetCursorPointIndex(cl_multipleSelect3)
                    ,this->GetCursorPointIndex(cl_multipleSelect4)
                );
            this->_pUI->updateAll(false);
        } else if ( this->_timesPressed > 5 ) {
            //this->pData->PrepareUndoCurve();
            this->_pData->getProcessing()->subtractBackground();
            this->_pData->getProcessing()->hideBackground();
            this->_pUI->updateAll(false);
        }
        break;

    case uf_calibration_data:
        if ( this->_isFunctionSetupNeeded ) { // tylko na poczatku
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText("IDS_mysz2"); // DANE DO KALIBRACJI: Ustaw dwa wskaŸniki. U¿yj lewego przycisku myszy, aby dodaæ wskaŸnik. U¿yj prawego, aby cofn¹æ operacjê.
            this->_pUI->updateAll(false);
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
            this->_pData->getProcessing()->calibrationData(cl1, cl2);
            this->setDefaults();
            this->_pUI->updateAll(false);
        }
        break;

    case uf_statistic_data:
        if ( this->_isFunctionSetupNeeded ) { // tylko na poczatku
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText("IDS_mysz3"); // ANALIZA PIKU/FALI: Ustaw dwa wskaŸniki. U¿yj lewego przycisku myszy dodaæ wskaŸnik. U¿yj prawego, aby cofn¹æ operacjê.
            this->_pUI->updateAll(false);
            return;
        }
        if ( this->_timesPressed > 2 ) {
            int cl1, cl2;
            if ( this->GetCursorX(cl_multipleSelect1) < this->GetCursorX(cl_multipleSelect2) ) {
                cl1 = this->GetCursorPointIndex(cl_multipleSelect1);
                cl2  = this->GetCursorPointIndex(cl_multipleSelect2);
            } else {
                cl2 = this->GetCursorPointIndex(cl_multipleSelect1);
                cl1 = this->GetCursorPointIndex(cl_multipleSelect2);
            }
            //this->pData->OnInterAnStatMA(cl1, cl2);
            this->setDefaults();
            this->_pUI->updateAll(false);
        }
        break;

    case uf_reg_data:
        if ( this->_isFunctionSetupNeeded ) { // tylko na poczatku
            this->_isFunctionSetupNeeded = false;
            this->_pUI->setStatusText("IDS_mysz4"); // ANALIZA REGRESJI: Ustaw dwa wskaŸniki. U¿yj lewego przycisku myszy, aby dodaæ wskaŸnik. U¿yj prawego, aby cofn¹æ operacjê.
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
            this->_pUI->setStatusText("IDS_mysz5"); // PRZESUWANIE KRZYWEJ: U¿yj lewego przycisku myszy aby przesun¹æ aktywn¹ krzyw¹. U¿yj prawego aby zakoñczyæ operacjê.
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
            this->_pUI->setStatusText("IDS_mysz6"); // WYSOKOŒÆ WZGLÊDNA: U¿yj lewego przycisku myszy aby ustawiæ pierwszy punkt krzywej bazowej. U¿yj prawego, aby cofn¹æ operacjê.
            this->_pUI->PlotReplot();
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
