
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
#include "curvecollection.h"

CurveCollection::CurveCollection(EAQtUIInterface *ui)
{
    _pUI = ui;
    _vCurves.clear();
}

CurveCollection::~CurveCollection()
{
    for ( uint32_t i=0; i<_vCurves.size(); ++i) {
        if ( _vCurves[i] == NULL ) {
            continue;
        }
        delete this->_vCurves[i];
    }
    this->_vCurves.clear();
}

uint32_t CurveCollection::append(Curve *curve)
{
    uint32_t index = _vCurves.size();
    _vCurves.push_back(curve);
    return index;
}

uint32_t CurveCollection::addNew(uint32_t nrOfDataPoints)
{
    uint32_t index = _vCurves.size();
    _vCurves.push_back(new Curve(nrOfDataPoints));
    _vCurves[index]->setPlot(_pUI->PlotAddGraph());
    return index;
}

Curve* CurveCollection::get(uint32_t index)
{
    if ( index < _vCurves.size() ) {
        return _vCurves[index];
    } else {
        return NULL;
    }
}

void CurveCollection::remove(uint32_t index)
{
    /*
    * Remove from Vector AND call destructor
    */
    if ( this->_vCurves.size() > index ) {
        this->_pUI->PlotRemoveGraph(this->_vCurves[index]->getPlot());
        delete _vCurves[index];
        this->_vCurves.erase(this->_vCurves.begin()+index);
    } else {
        throw std::invalid_argument( "Curve index out of range" );
    }
}

void CurveCollection::remove(Curve* curve)
{
    /*
    * Remove from Vector AND call destructor
    */
    for (uint32_t index=0; index<_vCurves.size(); ++index ) {
        if ( _vCurves[index] == curve ) {
            this->remove(index);
            return;
        }
    }
    throw std::invalid_argument( "Could not find curve" );
}

void CurveCollection::unset(uint32_t index)
{
    /*
    * Remove from Vector WITHOUT calling destructor
    */
    if ( this->_vCurves.size() > index ) {
        this->_vCurves[index] = NULL;
        this->_vCurves.erase(this->_vCurves.begin()+index);
    } else {
        throw std::invalid_argument( "Curve index out of range" );
    }
}

void CurveCollection::unset(Curve* curve)
{
    /*
    * Remove from Vector WITHOUT calling destructor
    */
    for (uint32_t index=0; index<_vCurves.size(); ++index ) {
        if ( _vCurves[index] == curve ) {
            this->unset(index);
            return;
        }
    }
    throw std::invalid_argument( "Could not find curve" );
}

void CurveCollection::clear()
{
    for ( uint32_t i=0; i<_vCurves.size(); ++i) {
        if ( _vCurves[i] == NULL ) {
            continue;
        }
        this->_pUI->PlotRemoveGraph(this->_vCurves[i]->getPlot());
        delete this->_vCurves[i];
    }
    this->_vCurves.clear();
}

uint32_t CurveCollection::count()
{
    return _vCurves.size();
}
