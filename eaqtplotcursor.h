
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
#ifndef EAQTPLOTCURSOR_H
#define EAQTPLOTCURSOR_H

#include "./Qcustomplot/qcustomplot.h"

class EAQtPlotCursor
{
public:
    EAQtPlotCursor(QCPItemStraightLine*, QCPCurve*);
    ~EAQtPlotCursor();
    void setVisible(bool);
    bool getVisible();
    void setColor(QColor);
    QColor getColor();
    void setMarker(QCPScatterStyle);
    void move(double x, double y = 0);
    void setSnapTo(QCPCurve*);
    QCPCurve* getSnappedTo();
    double getX();
    double getY();
    int32_t getIndex();
private:
    QCPItemStraightLine* _line;
    QCPCurve* _point;
    QCPCurve* _snappedTo;
    double _xpos;
    double _ypos;
    int32_t _posIndex;
};

#endif // EAQTPLOTCURSOR_H
