
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
#include "eaqtparampotentialprogram.h"
#include "eaqtdata.h"

EAQtParamPotentialProgram::EAQtParamPotentialProgram(QWidget *parent) : QDialog(parent)
{
    this->setModal(true);
    this->setWindowTitle(tr("Potential program"));

    this->separatorList.push_back(',');
    this->separatorList.push_back(';');
    this->separatorList.push_back('\t');
    this->separatorList.push_back(' ');
    this->separatorNum = 0;

    potentialProgramTextArea = new QPlainTextEdit();
    potentialProgramTextArea->setFixedHeight(250);
    potentialProgramTextArea->setFixedWidth(400);
    potentialProgramTextArea->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    this->oldPotentialList = EAQtData::getInstance().getPotentialProgram();

    if ( this->oldPotentialList.size() > 2 ) {
        QString program;
        program.append(tr("%1").arg(this->oldPotentialList.at(0)));
        for ( int i =1; i<this->oldPotentialList.size(); ++i ) {
            program.append(",");
            program.append(tr("%1").arg(this->oldPotentialList.at(i)));
        }
        potentialProgramTextArea->setPlainText(program);
    }
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(potentialProgramTextArea,0,0,2,1);
    summary = new QLabel();
    summary->setText(tr("> Summary <"));
    grid->addWidget(summary,2,0);

    connect(potentialProgramTextArea,SIGNAL(textChanged()),this,SLOT(updateSummary()));

    QPushButton *butOK = new QPushButton(tr("OK"));
    QPushButton *butCancel = new QPushButton(tr("Cancel"));
    grid->addWidget(butOK,3,0);
    grid->addWidget(butCancel,3,1);
    this->setLayout(grid);

    connect(butOK,SIGNAL(clicked(bool)),this,SLOT(clickedOK()));
    connect(butCancel,SIGNAL(clicked(bool)),this,SLOT(clickedCancel()));
    this->checkPotentialProgramArea();
}

void EAQtParamPotentialProgram::updateSummary()
{
   checkPotentialProgramArea();
}

bool EAQtParamPotentialProgram::checkPotentialProgramArea()
{
    newPotentialList.clear();
    std::string checktext = potentialProgramTextArea->document()->toPlainText().toStdString();
    int posp = -1;
    std::string tmp;
    int16_t data;

    bool foundSeparator = false;
    for (int i=0; i<this->separatorList.size();++i) {
        if ( checktext.find(this->separatorList[i]) != std::string::npos ) {
            this->separatorNum = i;
            foundSeparator = true;
            break;
        }
    }
    if ( !foundSeparator ) {
        summary->setText(tr("Could not detect data separator."));
        return false;
    }
    while ( (posp=checktext.find(separatorList[separatorNum])) != -1 ) {
        tmp = checktext.substr(0,posp);
        if ( isTextInteger( tmp ) ) {
             data = atoi(tmp.c_str());
        } else {
            summary->setText(tr("Only integer values are allowed."));
            return false;
        }
        checktext = checktext.substr(posp+1);
        newPotentialList.push_back(data);
    }

    tmp = checktext; //ostatni punkt
    if ( isTextInteger( tmp ) ) {
         data = atoi(tmp.c_str());
    } else {
        summary->setText(tr("Only integer values are allowed."));
        return false;
    }
    newPotentialList.push_back(data);
    summary->setText(tr("Decoded %1 values.").arg(newPotentialList.size()));
    if ( newPotentialList.size() > 2 ) {
        return true;
    } else {
        summary->setText(tr("Minimum three pontential points are required."));
        return false;
    }
}

bool EAQtParamPotentialProgram::isTextInteger(const std::string& s)
{
    return( strspn( s.c_str(), "-0123456789" ) == s.size() && s.c_str()[0] != '\0' );
}

void EAQtParamPotentialProgram::clickedOK()
{
    if ( this->checkPotentialProgramArea() ) {
        EAQtData::getInstance().setPotentialProgram(this->newPotentialList);
        this->destroy();
    } else {
       // do nothing ...
    }
}

void EAQtParamPotentialProgram::clickedCancel()
{
    this->destroy();
}

