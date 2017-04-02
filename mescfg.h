
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
#ifndef MESCFG_H
#define MESCFG_H

#define MAXMESCFG 55

#include "singlemescfg.h"

class MesCFG
{
public:
    MesCFG();
    ~MesCFG();
    int LoadFromFile(string);
    int SaveToFile(string);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
    int SaveToFile(wstring);
#endif
    int AddMes(SingleMesCFG);
    int AddMes();
    int GetNumberOfMes();
    bool RemoveMes(int);
    bool RemoveMes();
    bool SetActiveMes(int);
    void setParam(int,int);
    int getParam(int);
    void setNrBreaks(int);
    int getNrBreaks();
    void setMesRepetitions(int);
    int getMesRepetitions();
    void setDelay(int);
    int getDelay();
    void setFirstDelay(int);
    int getFirstDelay();
    void setWaitForInput(bool);
    bool getWaitForInput();
    void setFileName(string);
    string getFileName();
    void setCurveName(string);
    string getCurveName();
    void setCurveComment(string);
    string getCurveComment();
    void setMesName(string);
    string getMesName();
    bool Mes_Start();
    bool Mes_Next();
    int Mes_Param(int);
    std::vector<int16_t> Mes_PotentialProgram();
	int Mes_PotentialProgramLength();
    bool Mes_WaitForUser();
    int Mes_Delay();
    string Mes_FileName();
    string Mes_CName();
    string Mes_CComment();
    void Mes_Stop();
    SingleMesCFG** pMes;
    string lasterroron;

private:
    int nActiveMes;
    int nNumberOfMes;
    int nMesActive_CurrentCount;
    int nMesActive_GlobalCount;
    int nMesActive_MesIndex;

};

#endif // MESCFG_H
