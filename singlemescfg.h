
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
#ifndef SINGLEMESCFG_H
#define SINGLEMESCFG_H

#define MAXDELAY 9999
#define PARAMNUMBER 62
#define MAXREPETITIONS 50
#define MAXFILENAME 256
#define MAXCURVENAME 20
#define MAXCURVECOMMENT 200
#define MAXMESNAME 64
#define MAXBREAKS 7
#define FILETYPE 1
#define VERSION 1

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <sstream>
#include <vector>
#include "./pstdin/pstdin.h" //Added for Windows compatibility //
using namespace std;


class SingleMesCFG
{
public:
    SingleMesCFG();
    SingleMesCFG(SingleMesCFG*);
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
    string serialize();
    int unserialize(string);
    int getFileType();
    int getVersion();
    static char* generateHeader();
    static bool isHeaderOK(string);
    static string to_string(int);
    void addToPotentialProgram(int);
    int getPotentialProgramLength();
    std::vector<int16_t> getPotentialProgram();
    void setPotentialProgram(std::vector<int16_t>);

private:
    int nID;
    int nParams[PARAMNUMBER];
    int nMesRepetitions;
    int nDelay;
    int nFirstDelay;
    bool bWaitForInput;
    string sFileName;
    string sCurveName;
    string sCurveComment;
    string sMesComment;
    string sMesName;
    double initialized_check1;
    double initialized_check2;
    double initialized_check3;
    double initialized_check4;
    string escape(string);
    string unescape(string);
    std::vector<int16_t> potentialProgram;

    int pointsOfPotentialPogram;
};

#endif // SINGLEMESCFG_H
