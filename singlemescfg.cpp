
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

#include "singlemescfg.h"

SingleMesCFG::SingleMesCFG()
{
    for (int i=0; i<62; i++) {
        this->setParam(i,0);
    }

	// niezbedne zmiany, parametry, ktorych nie powinno sie zmieniac //
	this->setParam(3,1);
	this->setParam(6,1);
	this->setParam(8,1);
	// end niezbedne zmiany //

    this->nMesRepetitions = 1;
    this->nDelay = 0;
    this->nFirstDelay = 0;
    this->sCurveComment = "";
    this->sCurveName = "USER";
    this->sFileName = "./seria_pomiarow.vol";
    this->sMesName = "";
    this->bWaitForInput = false;

    this->pointsOfPotentialPogram=0;
    this->potentialProgram.clear();

    this->initialized_check1=1;
    this->initialized_check2=2;
    this->initialized_check3=3;
    this->initialized_check4=4;
}

SingleMesCFG::SingleMesCFG(SingleMesCFG *mesOrigin)
{
    for (int i=0; i<62; i++) {
        this->setParam(i,mesOrigin->getParam(i));
    }
    this->setMesRepetitions(mesOrigin->getMesRepetitions());
    try {
        this->setMesName(mesOrigin->getMesName());
    } catch (int a) {
        this->setMesName("");
    }
    this->setDelay(mesOrigin->getDelay());
    this->setFirstDelay(mesOrigin->getFirstDelay());
    this->setCurveComment(mesOrigin->getCurveComment());
    this->setCurveName(mesOrigin->getCurveName());
    this->setFileName(mesOrigin->getFileName());
    this->setWaitForInput(mesOrigin->getWaitForInput());
    this->setPotentialProgram(mesOrigin->getPotentialProgram());

    initialized_check1=1;
    initialized_check2=2;
    initialized_check3=3;
    initialized_check4=4;
}

string SingleMesCFG::to_string(int value)
{
  //create an output string stream
  std::ostringstream os ;
  //throw the value into the string stream
  os << value ;
  //convert the string stream into a string and return
  return os.str() ;
}

void SingleMesCFG::setParam(int nParamNum,int nParamVal)
{
	if ( nParamNum >= 0 && nParamNum < PARAMNUMBER ) {
		this->nParams[nParamNum] = nParamVal;
	} else {
        throw 1;
	}
    return;
}

int SingleMesCFG::getParam(int nParamNum)
{
    if ( nParamNum < PARAMNUMBER && nParamNum >= 0 )
        return this->nParams[nParamNum];
	else if (nParamNum >= PARAMNUMBER )
		return 0;
    else
        throw 1;
}

void SingleMesCFG::setNrBreaks(int nNrBreaks)
{
    if ( nNrBreaks <= MAXBREAKS && nNrBreaks >= 0 )
        this->setParam(19,nNrBreaks);
    else
        throw 333;
}

int SingleMesCFG::getNrBreaks()
{
    return this->getParam(19);
}

void SingleMesCFG::setMesRepetitions(int nRepetitions)
{
    if ( nRepetitions >= 0 && nRepetitions <= MAXREPETITIONS )
        this->nMesRepetitions = nRepetitions;
    else
        throw 1;
    return;
}

int SingleMesCFG::getMesRepetitions()
{
    return this->nMesRepetitions;
}

void SingleMesCFG::setDelay(int nDelay)
{
    if ( nDelay >= 0 && nDelay <= MAXDELAY )
        this->nDelay = nDelay;
    else
        throw 1;
    return;
}

int SingleMesCFG::getDelay()
{
    return this->nDelay;
}

void SingleMesCFG::setFirstDelay(int nDelay)
{
    if ( nDelay >= 0 && nDelay <= MAXDELAY )
        this->nFirstDelay = nDelay;
    else
        throw 1;
    return;
}

int SingleMesCFG::getFirstDelay()
{
    return this->nFirstDelay;
}

void SingleMesCFG::setWaitForInput(bool bWait)
{
    this->bWaitForInput = bWait;
    return;
}

bool SingleMesCFG::getWaitForInput()
{
    return this->bWaitForInput;
}

void SingleMesCFG::setFileName(string sFileName)
{
    this->sFileName=sFileName;
    return;
    string notallowed  ("\\/:*?\"<>|");
    if ( sFileName.length() <= MAXFILENAME )
        if ( sFileName.find_first_of( notallowed ) == false )
            this->sFileName=sFileName;
        else
            throw 1;
    else
        throw 1;
    return;
}

string SingleMesCFG::getFileName()
{
    return this->sFileName;
}

void SingleMesCFG::setCurveName(string sCurveName)
{
    if ( sCurveName.length() <= MAXCURVENAME && sCurveName.length()> 0 )
        this->sCurveName = sCurveName;
    else
        throw 1;
    return;
}

string SingleMesCFG::getCurveName()
{
    return this->sCurveName;
}

void SingleMesCFG::setCurveComment(string sCurveComment)
{
    if ( sCurveComment.length() <= MAXCURVECOMMENT )
        this->sCurveComment = sCurveComment;
    else
        throw 1;
    return;
}

string SingleMesCFG::getCurveComment()
{
    return this->sCurveComment;
}

void SingleMesCFG::setMesName(string sMesName)
{
    if ( sMesName.length() <= MAXMESNAME )
        this->sMesName = sMesName;
    else
        throw 1;
    return;
}

string SingleMesCFG::getMesName()
{
    return this->sMesName;
}

string SingleMesCFG::serialize()
{
    if ( initialized_check1!=1
        && initialized_check2!=2
        && initialized_check3!=3
        && initialized_check4!=4 )
        return "";

    using namespace std;
    string serialized;
    serialized = "{";
    serialized = serialized + "*:\""+this->to_string(nMesRepetitions)+"\";";
    serialized = serialized + "mesname:\"" + sMesName + "\";";
    serialized = serialized + "fname:\""+sFileName+"\";";
    serialized = serialized + "cname:\""+sCurveName+"\";";
    serialized = serialized + "ccomment:\""+sCurveComment+"\";";
    serialized = serialized + "cdelay:\""+this->to_string(nDelay)+"\";";
    serialized = serialized + "cfirstdelay:\""+to_string(nFirstDelay)+"\";";
    serialized = serialized + "bwaitinput:\""+to_string(bWaitForInput)+"\";";
    if ( this->getPotentialProgramLength() > 2 ) {
        std::string tmp;
        tmp.append(to_string(this->getPotentialProgram()[0]));
        for ( int i =1; i<this->getPotentialProgramLength(); ++i ) {
            tmp.append(",");
            tmp.append(to_string(this->getPotentialProgram()[i]));
        }
        serialized = serialized + "potentialProgram:\"" + tmp + "\";";
    }
    for (int i=0;i<62;i++) {
        serialized = serialized + "nr"+this->to_string(i)+":\""+this->to_string(this->getParam(i))+"\";";
    }
    serialized = serialized +  "}\n";
    return serialized;
}

int SingleMesCFG::unserialize(string sSM)
{
    using namespace std;
    if ( sSM.empty() || sSM.substr(0,1) != "{" ) {
        //this = NULL;
        return -1;
    }
    sSM=sSM.substr(1);
    string sParam;
    string sValue;
    int iParamsSet = 0;
    int pos;
    while ( sSM.length() > 0 ) {
        if ( sSM.length() == 1 && sSM == "}" )
            return 0;
        pos = sSM.find(":\"");
        sParam = sSM.substr(0,pos);
        sSM=sSM.substr(pos+2);
        pos=sSM.find("\";");
        sValue = sSM.substr(0,pos);
        sSM=sSM.substr(pos+2);
        if ( sParam == "*" ) {
            this->setMesRepetitions(atoi(sValue.c_str()));
        } else if ( sParam == "cdelay" ) {
            this->setDelay(atoi(sValue.c_str()));
        } else if ( sParam == "cfirstdelay" ) {
            this->setFirstDelay(atoi(sValue.c_str()));
        } else if ( sParam == "mesname" ) {
            this->setMesName(sValue);
        } else if ( sParam == "fname" ) {
            this->setFileName(sValue);
        } else if ( sParam == "cname" ) {
            this->setCurveName(sValue);
        } else if ( sParam == "ccomment" ) {
            this->setCurveComment(sValue);
        } else if ( sParam == "bwaitinput" ) {
            this->setWaitForInput(atoi(sValue.c_str()));
        } else if ( sParam == "potentialProgram" ) {
            int posp = -1;
            while ( (posp=sValue.find(',')) != -1 ) {
                int data = atoi(sValue.substr(0,posp).c_str());
                sValue = sValue.substr(posp+1);
                this->addToPotentialProgram(data);
            }
            int data = atoi(sValue.c_str()); //ostatni punkt
            this->addToPotentialProgram(data);
        } else if ( sParam.substr(0,2) == "nr" && sParam.length() > 2 ) {
            sParam = sParam.substr(2);
            this->setParam(atoi(sParam.c_str()), atoi(sValue.c_str()) );
            iParamsSet++;
        } else {
            //this = NULL;
            return -1;
        }
    }
    if ( iParamsSet != PARAMNUMBER ) {
        return -1;
    }
    return 0;
}

string SingleMesCFG::escape(string toEsc)
{
    int pos;
    pos=0;
    while ( (pos=toEsc.find(":",pos) < string::npos) ) {
        toEsc.replace(pos,1,"\\:");
        pos+=3;
    }
    pos=0;
    while ( (pos=toEsc.find(";",pos) < string::npos) ) {
        toEsc.replace(pos,1,"\\;");
        pos+=3;
    }
    pos=0;
    while ( (pos=toEsc.find("}",pos) < string::npos) ) {
        toEsc.replace(pos,1,"\\}");
        pos+=3;
    }
    pos=0;
    while ( (pos=toEsc.find("{",pos) < string::npos) ) {
        toEsc.replace(pos,1,"\\{");
        pos+=3;
    }
    return toEsc;
}

string SingleMesCFG::unescape(string toUnesc)
{
    int pos;
    pos=0;
    while ( (pos=toUnesc.find("\\:",pos) < string::npos) )
        toUnesc.replace(pos,3,":");
    pos=0;
    while ( (pos=toUnesc.find(";",pos) < string::npos) )
        toUnesc.replace(pos,3,"\\;");
    pos=0;
    while ( (pos=toUnesc.find("\\}",pos) < string::npos) )
        toUnesc.replace(pos,3,"}");
    pos=0;
    while ( (pos=toUnesc.find("\\{",pos) < string::npos) )
        toUnesc.replace(pos,3,"{");
    return toUnesc;
}

int SingleMesCFG::getFileType()
{
    return FILETYPE;
}

int SingleMesCFG::getVersion()
{
    return VERSION;
}

char* SingleMesCFG::generateHeader()
{
    char* header = new char[5];
    header[0] = *SingleMesCFG::to_string(FILETYPE).data();
    header[1] = ':';
    header[2] = *SingleMesCFG::to_string(VERSION).data();
    header[3] = '\n';
    header[4] = '\0';
    return header;
}

bool SingleMesCFG::isHeaderOK(string sHeader)
{
    if ( atoi( &sHeader.substr(0,1)[0] ) != FILETYPE )
        return false;
    if (sHeader.substr(1,1).data()[0] != ':' )
        return false;
    if ( atoi(&sHeader.substr(2,1)[0]) > VERSION )
        return false;
    if ( sHeader.length() != 3 )
        return false;

    return true;
}

void SingleMesCFG::addToPotentialProgram(int data)
{
    this->potentialProgram.push_back(data);
    this->pointsOfPotentialPogram++;
}

int SingleMesCFG::getPotentialProgramLength()
{
    return this->potentialProgram.size();
    //return this->pointsOfPotentialPogram;
}

std::vector<int16_t> SingleMesCFG::getPotentialProgram()
{
    return this->potentialProgram;
}

void SingleMesCFG::setPotentialProgram(std::vector<int16_t> program)
{
    this->potentialProgram = program;
    this->pointsOfPotentialPogram = program.size();
}
