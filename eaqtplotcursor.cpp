
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
#include "eaqtplotcursor.h"

EAQtPlotCursor::EAQtPlotCursor(QCPItemStraightLine* sl, QCPCurve* gr)
{
    this->_line = sl;
    sl->setSelectable(false);
    sl->setVisible(false);
    this->_point = gr;
    gr->setSelectable(QCP::stNone);
    gr->setVisible(false);
    //gr->setLineStyle(QCPGraph::lsNone);
    gr->setScatterStyle(QCPScatterStyle::ssStar);
    QColor col = QColor(0,0,0);
    QPen qp = this->_line->pen();
    qp.setColor(col);
    this->_line->setPen(qp);
    QPen qpp = this->_point->pen();
    qpp.setColor(col);
    this->_point->setPen(qpp);
    this->_snappedTo = NULL;
    this->_xpos = 0;
    this->_ypos = 0;
}

void EAQtPlotCursor::setVisible(bool b)
{
    this->_line->setVisible(b);
    this->_point->setVisible(b);
}

bool EAQtPlotCursor::getVisible()
{
    return this->_line->visible();

}

void EAQtPlotCursor::setColor(QColor col)
{
    QPen qp = this->_line->pen();
    qp.setColor(col);
    this->_line->setPen(qp);
    QPen qpp = this->_point->pen();
    qpp.setColor(col);
    this->_point->setPen(qpp);

}

QColor EAQtPlotCursor::getColor()
{
    QPen qp = this->_line->pen();
    return qp.color();
}

void EAQtPlotCursor::setMarker(QCPScatterStyle ss)
{
    this->_point->setScatterStyle(ss);
}

void EAQtPlotCursor::move(double x, double y)
{
    if ( this->_snappedTo == NULL ) {
        this->_line->point1->setCoords(x,y);
        this->_line->point2->setCoords(x,y+1);
        this->_point->data()->clear();
        this->_point->addData(x,y);
    } else {
        int32_t index = 0;
        double diff = fabs(x - this->_snappedTo->data()->at(index)->key);
        double diffnew;
        for ( int i = 0; i<this->_snappedTo->data()->size(); ++i ) {
            diffnew = fabs(x - this->_snappedTo->data()->at(i)->key);
            if ( diff > diffnew ) {
                index = i;
                diff = diffnew;
            }
        }
        x = this->_snappedTo->data()->at(index)->key;
        y = this->_snappedTo->data()->at(index)->value;
        this->_line->point1->setCoords(x,y);
        this->_line->point2->setCoords(x,y+1);
        this->_point->data()->clear();
        this->_point->addData(x,y);
        this->_posIndex = index;
    }
    this->_xpos = x;
    this->_ypos = y;
}

double EAQtPlotCursor::getX()
{
    return this->_xpos;
}

double EAQtPlotCursor::getY()
{
    return this->_ypos;
}

int32_t EAQtPlotCursor::getIndex()
{
    return this->_posIndex;
}

void EAQtPlotCursor::setSnapTo(QCPCurve *g)
{
    this->_snappedTo = g;
}
