
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
#ifndef EAQTMOUSEHANDLER_H
#define EAQTMOUSEHANDLER_H

#include "eaqtdatainterface.h"
#include "eaqtuiinterface.h"
#include "eaqtplotcursor.h"

class EAQtMouseHandler : public QObject
{
    Q_OBJECT

public:
    EAQtMouseHandler(EAQtDataInterface*, EAQtUIInterface*);
    ~EAQtMouseHandler(void);

    const struct PlotInteractions {
        QCP::Interactions plotDefaultInteractions = QCP::iRangeZoom | QCP::iRangeDrag | QCP::iSelectPlottables;
        QCP::Interactions plotCursorInteractions = QCP::iRangeZoom | QCP::iRangeDrag;
        QCP::Interactions plotNoInteractions = NULL;
    } _plotInteractions;

    enum mouseMode {
        mm_normal,
        mm_dataCursor,
        mm_relativeParamters,
        mm_nearestYonPlot,
        mm_place2markers,
        mm_place4markers,
        mm_measurement,
        mm_LAST
    };

    enum cursorsList {
        cl_relativeParameterCursor,
        cl_multipleSelect1,
        cl_multipleSelect2,
        cl_multipleSelect3,
        cl_multipleSelect4,
        cl_snapXonPlot,
        cl_movingCursor,
        cl_LAST
    };

    enum userFunctions {
        uf_none,
        uf_background_correction ,
        uf_calibration_data,
        uf_statistic_data,
        uf_reg_data,
        uf_move_curve,
        uf_relative_height
    };

    void ChangeMouseMode(mouseMode, userFunctions);
    void ForwardClick(double MX, double MY);
    void BackClick();
    void UpdateLocation(double *MX, double *MY);
    bool wantsClicks();
    QVector<EAQtPlotCursor*>* getCursors();
    int32_t GetCursorPointIndex(cursorsList);

private:
    void setDefaults();
    double GetCursorX(cursorsList);
    double GetCursorY(cursorsList);
    bool GetCursorVisible(cursorsList);
    void SetCursorsOnActPlot();
    void callUserFunction();
    void toggleMovingCursor(QCPCurve* gr);
    bool userCanUseCurveList();
    void updateCursorLabel();
    double dClickCoordinates[2];
    struct RelativeParametersLine {
        double pointX;
        double pointY;
        double baseLineA;
        double baseLineB;
        bool followCursor;
        bool staticLine;
        QCPItemStraightLine* baseLine;
    };
    RelativeParametersLine _structRPL;
    short _timesPressed;
    mouseMode _activeMode;
    userFunctions _userFunction;
    bool _isFunctionSetupNeeded;
    QVector<EAQtPlotCursor*> _vCursors;
    EAQtDataInterface* _pData;
    EAQtUIInterface* _pUI;

public slots:
    void onSelectionChanged();

};

#endif // EAQTMOUSEHANDLER_H
