
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
//#include "stdafx.h"

#include "mescfg.h"
#include <errno.h>
#include <locale>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

MesCFG::MesCFG()
{
    this->pMes = new SingleMesCFG*[MAXMESCFG];
    this->pMes[0] = new SingleMesCFG();
    this->pMes[1] = NULL;
    this->nNumberOfMes = 1;
    this->nActiveMes = 1;
    this->nMesActive_CurrentCount = 0;
    this->nMesActive_GlobalCount = 0;
    this->nMesActive_MesIndex = 0;
    this->lasterroron = "";

}

MesCFG::~MesCFG() {
    for(int i=0; i<this->nNumberOfMes; i++)
        delete this->pMes[i];
    delete []this->pMes;
}

int MesCFG::GetNumberOfMes()
{
    return this->nNumberOfMes;
}

bool MesCFG::SetActiveMes(int nActivate)
{
    if ( nActivate >= 0
         && nActivate < this->nNumberOfMes
         && this->pMes[nActivate] != NULL ) {
        this->nActiveMes = nActivate;
        return true;
    } else {
        return false;
    }
}

void MesCFG::setParam(int nParam,int nValue)
{
    this->pMes[this->nActiveMes]->setParam(nParam,nValue);
}

int MesCFG::getParam(int nParam)
{
	// niezbędne zmiany, parametry, ktorych nie powinno sie zmieniac //
	this->pMes[this->nActiveMes]->setParam(3,1);
	this->pMes[this->nActiveMes]->setParam(6,1);
	this->pMes[this->nActiveMes]->setParam(8,1);
	// end niezbedne zmiany //
    return this->pMes[this->nActiveMes]->getParam(nParam);
}

void MesCFG::setNrBreaks(int nBreaks)
{
     this->pMes[this->nActiveMes]->setNrBreaks(nBreaks);
}

int MesCFG::getNrBreaks()
{
    return this->pMes[this->nActiveMes]->getNrBreaks();
}

void MesCFG::setMesRepetitions(int nRep)
{
    this->pMes[this->nActiveMes]->setMesRepetitions(nRep);
}

int MesCFG::getMesRepetitions()
{
    return this->pMes[this->nActiveMes]->getMesRepetitions();
}
void MesCFG::setDelay(int nDelay)
{
    this->pMes[this->nActiveMes]->setDelay(nDelay);
}

int MesCFG::getDelay()
{
    return this->pMes[this->nActiveMes]->getDelay();
}

void MesCFG::setFirstDelay(int nFD)
{
    this->pMes[this->nActiveMes]->setFirstDelay(nFD);
}

int MesCFG::getFirstDelay() {
    return this->pMes[this->nActiveMes]->getFirstDelay();
}

void MesCFG::setWaitForInput(bool nWFI)
{
    this->pMes[this->nActiveMes]->setWaitForInput(nWFI);
}

bool MesCFG::getWaitForInput()
{
    return this->pMes[this->nActiveMes]->getWaitForInput();
}

void MesCFG::setFileName(string sFN)
{
    this->pMes[this->nActiveMes]->setFileName(sFN);
}

string MesCFG::getFileName()
{
    return this->pMes[this->nActiveMes]->getFileName();
}

void MesCFG::setCurveName(string sCN)
{
    this->pMes[this->nActiveMes]->setCurveName(sCN);
}

string MesCFG::getCurveName()
{
    return this->pMes[this->nActiveMes]->getCurveName();
}

void MesCFG::setCurveComment(string sCC)
{
    this->pMes[this->nActiveMes]->setCurveComment(sCC);
}

string MesCFG::getCurveComment()
{
    return this->pMes[this->nActiveMes]->getCurveComment();
}

void MesCFG::setMesName(string sMN)
{
    this->pMes[this->nActiveMes]->setMesName(sMN);
}

string MesCFG::getMesName()
{
    return this->pMes[this->nActiveMes]->getMesName();
}

int MesCFG::LoadFromFile(string sFileName)
{
    //zwraca ilosc wczytanych serii pomiarowych
    // -3 błąd w czasie wczytywania
    // -2 plik nie kompatybilny
    // -1 nie udalo się otworzyć pliku
    // 0 nie zawiera żadnej konfiguracji
    // int -- ilość wczytanych serii konfiguracji
    for(int i=1; i<this->nNumberOfMes; i++)
        delete this->pMes[i];
    this->pMes[1] = NULL;
    this->nNumberOfMes = 1;

    int nLineNumber = 0;
	string line;
	this->lasterroron += sFileName.c_str();
	this->lasterroron += "; ";
	ifstream eacfgfile (sFileName.c_str());
    int nAddingMes = 0;
    if ( eacfgfile.is_open() ) {
        while ( getline (eacfgfile,line) )  {
            if ( line[line.size()-1] == '\r' ) {
                line.erase(line.size()-1,1);
            }
            nLineNumber++;
            if ( nLineNumber == 1) {
                if ( !SingleMesCFG::isHeaderOK(line) ) {
                    return -2;
                } else {
                    continue;
                }
            }
            if ( line.length() == 0 )
                break;
            if ( nAddingMes == 0 ) {
                // nie można pozwolić, żeby się nie wczytała krzywa 0, więc testujemy jej wczytanie.
                this->pMes[1] = new SingleMesCFG();
                if ( this->pMes[1]->unserialize(line) == -1 ) {
                    delete this->pMes[1];
                    this->pMes[1] = NULL;
                    this->lasterroron = line;
                    return -3;
                } else {
                    delete this->pMes[1];
                    this->pMes[1] = NULL;
                }
            }

            this->pMes[nAddingMes] = new SingleMesCFG();
            if ( this->pMes[nAddingMes]->unserialize(line) == -1 ) {
                delete this->pMes[nAddingMes];
                this->pMes[nAddingMes] = NULL;
                this->lasterroron = line;
                return -3;
            }
            if (nAddingMes == 0)
                this->pMes[nAddingMes]->setMesName("");
            this->pMes[nAddingMes+1] = NULL;
            nAddingMes++;
        }
        eacfgfile.close();
        this->nNumberOfMes = nAddingMes;
        return nAddingMes;
    } else {
		this->lasterroron += strerror(errno);
		return -1;
    }
}

int MesCFG::SaveToFile(string sFileName)
{
    ofstream eacfgfile (sFileName.c_str());
    if (eacfgfile.is_open()) {
        eacfgfile << SingleMesCFG::generateHeader();
        int nSavingMes = 0;
        while ( this->pMes[nSavingMes] != NULL ) {
            eacfgfile << this->pMes[nSavingMes]->serialize().data();
            nSavingMes++;
        }
        eacfgfile.close();
    } else {
        return 0;
    }
    return 1;
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
int MesCFG::SaveToFile(wstring sFileName)
{
    FILE* fp;
    //_wfopen_s(&fp, sFileName.c_str(),);
    fp = _wfopen(sFileName.c_str(), L"w+");
    if ( fp != NULL ) {
        fwrite(SingleMesCFG::generateHeader(), sizeof(char), 4, fp);
        int nSavingMes = 0;
        while ( this->pMes[nSavingMes] != NULL ) {
            string s = this->pMes[nSavingMes]->serialize();
            fwrite(s.data(), sizeof(char), s.size(), fp);
            nSavingMes++;
        }
        fclose(fp);
    } else {
        return 0;
    }
    return 1;
}
#endif

int MesCFG::AddMes(SingleMesCFG newMes)
{
    // zwraca pozycje na której dodano obiekt;
    this->pMes[this->nNumberOfMes] = new SingleMesCFG(newMes);
    this->pMes[this->nNumberOfMes+1] = NULL;
    nNumberOfMes++;
    return nNumberOfMes-1;
}

int MesCFG::AddMes()
{
    // zwraca pozycje na której dodano obiekt;
    this->pMes[this->nNumberOfMes] = new SingleMesCFG;
    this->pMes[this->nNumberOfMes+1] = NULL;
    nNumberOfMes++;
    return nNumberOfMes-1;
}

bool MesCFG::RemoveMes(int toDel)
{
    this->nNumberOfMes--;
    int i;
    for ( i = toDel; i<=this->nNumberOfMes;i++ ) {
        this->pMes[i] = this->pMes[i+1];
    }
    this->pMes[i+1] = NULL;
    return true;
}

bool MesCFG::RemoveMes()
{
    return true;
}

bool MesCFG::Mes_Start()
{
    this->nMesActive_CurrentCount = 0;
    this->nMesActive_GlobalCount = 0;
    this->nMesActive_MesIndex = 1; // 1 indexed, 0 to sa ustawienia domyslne!!!
    return this->SetActiveMes(1);
}

bool MesCFG::Mes_Next()
{
    this->nMesActive_GlobalCount++;
    this->nMesActive_CurrentCount++;
    if ( this->getMesRepetitions() <= this->nMesActive_CurrentCount ) {
        this->nMesActive_CurrentCount = 0;
        this->nMesActive_MesIndex++;
        if ( this->SetActiveMes( this->nMesActive_MesIndex) ) {
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool MesCFG::Mes_WaitForUser()
{
    if ( this->nMesActive_CurrentCount == 0 && this->getWaitForInput() )
        return true;
    else
        return false;
}

int MesCFG::Mes_Delay()
{
    if ( this->nMesActive_CurrentCount == 0 )
        return this->getFirstDelay();
    else
        return this->getDelay();
}

int MesCFG::Mes_Param(int nParamNum)
{
	return this->getParam(nParamNum);
}

string MesCFG::Mes_FileName()
{
    return this->getFileName();
}

string MesCFG::Mes_CName()
{
    return this->getCurveName();
}

string MesCFG::Mes_CComment()
{
    return this->getCurveComment();
}

void MesCFG::Mes_Stop()
{
    this->~MesCFG();
}

std::vector<int16_t> MesCFG::Mes_PotentialProgram()
{
	return this->pMes[this->nActiveMes]->getPotentialProgram();
}

int MesCFG::Mes_PotentialProgramLength()
{
	return this->pMes[this->nActiveMes]->getPotentialProgramLength();
}

