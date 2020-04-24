
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

CurveCollection::CurveCollection(CurveCollection* toCopy)
{
   _vCurves.clear();
   _vCurves.resize(toCopy->count());
   _pUI = toCopy->_pUI;
   for (int i =0; i<toCopy->count();++i) {
       Curve* copyc = new Curve(toCopy->get(i));
       _vCurves[i] = copyc;
   }
}

CurveCollection::~CurveCollection()
{
    for ( auto i=0; i<_vCurves.size(); ++i) {
        if ( _vCurves[i] == nullptr ) {
            continue;
        }
        delete this->_vCurves[i];
    }
    this->_vCurves.clear();
}

int32_t CurveCollection::append(Curve *curve)
{
    int32_t index = _vCurves.size();
    if ( index >= TYPES::maxVectorIndex
    || index < 0 ) {
        throw 1;
    }
    _vCurves.push_back(curve);
    return index;
}

TYPES::vectorindex_t CurveCollection::addNew(TYPES::vectorindex_t nrOfDataPoints)
{
    TYPES::vectorindex_t index = _vCurves.size();
    if ( index >= TYPES::maxVectorIndex
    || index < 0 ) {
        throw 1;
    }
    _vCurves.push_back(new Curve(nrOfDataPoints));
    _vCurves[index]->setPlot(_pUI->PlotAddQCPCurve());
    //_vCurves[index]->getPlot()->setAdaptiveSampling(false);
    return index;
}

Curve* CurveCollection::get(TYPES::vectorindex_t index)
{
    if ( index < _vCurves.size() && index >= 0 ) {
        return _vCurves[index];
    } else {
        return nullptr;
    }
}

void CurveCollection::remove(TYPES::vectorindex_t index)
{
    /*
    * Remove from Vector AND call destructor
    */
    if ( this->_vCurves.size() > index
    && index >= 0 ) {
        this->_pUI->PlotRemoveQCPCurve(this->_vCurves[index]->getPlot());
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
    for (TYPES::vectorindex_t index=0; index<_vCurves.size(); ++index ) {
        if ( _vCurves[index] == curve ) {
            this->remove(index);
            return;
        }
    }
    throw std::invalid_argument( "Could not find curve" );
}

void CurveCollection::unset(TYPES::vectorindex_t index)
{
    /*
    * Remove from Vector WITHOUT calling destructor
    */
    if ( this->_vCurves.size() > index
    && index >= 0 ) {
        this->_vCurves[index] = nullptr;
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
    for (TYPES::vectorindex_t index=0; index<_vCurves.size(); ++index ) {
        if ( _vCurves[index] == curve ) {
            this->unset(index);
            return;
        }
    }
    throw std::invalid_argument( "Could not find curve" );
}

void CurveCollection::clear()
{
    for ( TYPES::vectorindex_t i=0; i<_vCurves.size(); ++i) {
        if (_vCurves[i] == nullptr) {
            continue;
        }
        this->_pUI->PlotRemoveQCPCurve(this->_vCurves[i]->getPlot());
        delete this->_vCurves[i];
    }
    this->_vCurves.clear();
}

int32_t CurveCollection::count()
{
    return _vCurves.size();
}
