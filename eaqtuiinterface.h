
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
#ifndef GRAPHINTERFACE_H
#define GRAPHINTERFACE_H

#include "./Qcustomplot/qcustomplot.h"
#include "eaqtsavefiledialog.h"
#include "eaqtplotcursor.h"

class EAQtUIInterface
{
public:
    virtual void updateAll(bool rescale = true) = 0;
    virtual QCPGraph* PlotAddGraph() = 0;
    virtual void PlotRescaleAxes() = 0;
    virtual bool PlotRemoveGraph(QCPGraph* graph) = 0;
    virtual EAQtPlotCursor *PlotAddCursor() = 0;
    virtual double PlotGetXMiddle() = 0;
    virtual void PlotReplot() = 0;
    virtual void PlotSetInteraction(QCP::Interactions) = 0;
    virtual QCPItemStraightLine* PlotAddLine() = 0;
    struct PlotLayer {
        QCPLayer* Active;
        QCPLayer* NonActive;
        QCPLayer* Measurement;
        QCPLayer* Markers;
    };
    virtual PlotLayer* PlotGetLayers() = 0;
    virtual void PlotDisconnectMouse() = 0;
    virtual void PlotConnectMouse() = 0;
	virtual void PlotSetInverted(bool) = 0;

    virtual void MeasurementSetup() = 0;
    virtual void MeasurementUpdate(uint32_t curveNr, uint32_t pointNr) = 0;
    virtual void MeasurementAfter() = 0;
    virtual void enableAll() = 0;
    virtual void disableButtonsOnly() = 0;
    virtual void disableButtonsAndTable() = 0;
    virtual void showMessageBox(QString text, QString title = "") = 0;
    virtual bool showQuestionBox(QString text, QString title = "") = 0;
    virtual EAQtSaveFiledialog::SaveDetails DialogSaveInFile() = 0;
    virtual void changeStartButtonText(QString) = 0;
    virtual void setLowLabelText(int n, QString text) = 0;
    virtual void setStatusText(QString str) = 0;
};

#endif // GRAPHINTERFACE_H
