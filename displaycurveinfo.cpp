
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
#include "displaycurveinfo.h"

DisplayCurveInfo::DisplayCurveInfo(Curve* c) : QObject()
{
    _curve = c;
}

QString DisplayCurveInfo::getHTMLInfo()
{
    //TODO for total revamp ... no Estart support
    QString pte;
    int width = 240;

    bool isLSV = (_curve->Param(PARAM::method) == PARAM::method_lsv);
    QString sweepRate = "";
    if ( isLSV ) {
        double speed = MEASUREMENT::LSVstepE[_curve->Param(PARAM::dEdt)]/MEASUREMENT::LSVtime[_curve->Param(PARAM::dEdt)];
        if ( speed < 0.025 )
            sweepRate = tr("%1 mV/s").arg(speed*1000,0,'f',1);
        else if ( speed < 1 )
            sweepRate = tr("%1 mV/s").arg(speed*1000,0,'f',0);
        else
            sweepRate = tr("%1 V/s").arg(speed,0,'f',0);
    }

    QString breaks = "";
    if ( _curve->Param(PARAM::breaknr) > 0 ) {
        breaks=tr("<table><tr><th>E&nbsp;[mV]</th><th>t&nbsp;[min:sec]</th></tr>");
        for ( int i = 0; i<_curve->Param(PARAM::breaknr) ; ++i ) {
            breaks.append(tr("<tr><td>%1</td><td>%2:%3</td></tr>").arg(_curve->Param(PARAM::breakE+i))
                                                                   .arg(_curve->Param(PARAM::breakmin+i), 2, 10, QChar('0'))
                                                                   .arg(_curve->Param(PARAM::breaksec+i), 2, 10, QChar('0'))
                       );
        }
    }

    pte = tr("<b>General</b><hr>")
          + tr("<table width=%1 style='border: 1px gray solid'>").arg(width)
          + tr("<tr><th width=45 align=left>Date: </th><td>%1-%2-%3&nbsp;%4:%5:%6</td></tr>")
                                                .arg(_curve->Param(PARAM::date_year))
                                                .arg(_curve->Param(PARAM::date_month), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_day), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_hour), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_minutes), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_seconds), 2, 10, QChar('0'))
          + tr("<tr><th width=45 align=left>Name: </th><td>%1</td></tr>").arg(_curve->CName())
          + tr("<tr><th colspan=2>Comment:</th></tr>")
          + tr("<tr><td colspan=2><div style='height:70px'>%1</div></td></tr>").arg(_curve->Comment())
          + tr("<tr><th colspan=2>File:</th></tr>")
          + tr("<tr><td colspan=2><div style='height:70px'>%1</div></td></tr>").arg(_curve->FName())
          + tr("</table>")
          + tr("<hr><b>Measurement</b><hr>")
          + tr("<table>")
          + tr("<tr><td colspan=2><b>Type:</b></td></tr>")
          + tr("<tr><td colspan=2><div style='height:30px'>%1</div></td></tr>").arg( getMespv() +","+getMethod() + (!isLSV?","+getSampl():"") )
          + tr("<tr><td colspan=2><b>Current range:</b> %1</td></tr>").arg(this->getCrangeInfo())
          + tr("<tr><td width=%1><b>Ep:</b>&nbsp;%2&nbsp;mV</td><td width=%3><b>Ek:</b>&nbsp;%4&nbsp;mV</td></tr>")
                                .arg(width/2)
                                .arg(_curve->Param(PARAM::Ep))
                                .arg(width/2)
                                .arg(_curve->Param(PARAM::Ek))
          + (isLSV?
              tr("<tr><td colspan=2><b>Sweep rate:</b>&nbsp;%1&nbsp;mV/s</td></tr>").arg(sweepRate)
            : tr("<tr><td><b>Estep:</b>&nbsp;%1&nbsp;mV</td>").arg(_curve->Param(PARAM::Estep))
                   + ((_curve->Param(PARAM::method)==PARAM::method_dpv)||(_curve->Param(PARAM::method)==PARAM::method_sqw_osteryoung)?
                          tr("<td><b>dE:</b>&nbsp;%1&nbsp;mV</td></tr>").arg(_curve->Param(PARAM::dE))
                        : ((_curve->Param(PARAM::method)==PARAM::method_npv)?
                              tr("<td><b>E0:</b>&nbsp;%1&nbsp;mV<td></tr>")
                            : tr("<td></td></tr>")
                           )
                      )
                  + tr("<tr><td><b>tp:</b>&nbsp;%1&nbsp;ms</td><td><b>tw:</b>&nbsp;%2&nbsp;ms</td></tr>").arg(_curve->Param(PARAM::tp)).arg(_curve->Param(PARAM::tw))
             )
          + tr("<tr><td><b>td:</b>&nbsp;%1&nbsp;ms</td><td></td></tr>").arg(_curve->Param(PARAM::td))
          + tr("<tr><td colspan=2><b>Data points:</b>&nbsp;%1</td></tr>").arg(_curve->Param(PARAM::ptnr))
          + tr("<tr><td colspan=2><b>Multielectrode setup:</b>&nbsp;%1</td></tr>").arg(_curve->Param(PARAM::multi),8,2)
          + (_curve->Param(PARAM::breaknr)>0?
                 tr("<tr><td colspan=2><b>Breaks:</b></td></tr>")
               + tr("<tr><td colspan=2>%1</td></tr>").arg(breaks)
             : tr("")
            );

    return pte;


    pte.append( "<h4>" + QApplication::translate("EAQtMainWindow","General") + "</h4>"
                + "<tr><th>" + QApplication::translate("EAQtMainWindow","name: ") + "</th></tr><tr><td>" + _curve->CName() + "</td></tr>"
                + "<tr><th>" + QApplication::translate("EAQtMainWindow","comment: ") + "</th></tr><tr><td>" + _curve->Comment() + "</td></tr>"
                + "<tr><th>" + QApplication::translate("EAQtMainWindow","file: ") + "</th></tr><tr><td>" + _curve->FName() + "</td></tr>"
                + "<tr><th>" + QApplication::translate("EAQtMainWindow","date: ") + "</th></tr><tr><td>" + QString("%1-%2-%3 %4:%5:%6").arg(_curve->Param(PARAM::date_year))
                                                .arg(_curve->Param(PARAM::date_month), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_day), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_hour), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_minutes), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_seconds), 2, 10, QChar('0'))
                                                    + "</td></tr></table>"
                + "<h4>" + QApplication::translate("EAQtMainWindow","Measurement") + "</h4>"
                + "<table><tr><th colspan=2>" + QApplication::translate("EAQtMainWindow","setup: ") + "</th></tr><tr><td colspan=2>" + getEl32() + "</td></tr>"
                + "<tr><th colspan=2>" + QApplication::translate("EAQtMainWindow","type: ") + "</th></tr><tr><td colspan=2>" + getMespv() + ", "
                                                                     + getMethod()
                                                                     + (!isLSV?","+getSampl():"")
                                                                     + "</td></tr>"
                + "<tr><th>" + QApplication::translate("EAQtMainWindow","electrode: ") +"</th><td>" + getElectr() + "</td></tr>"
                + "<tr><th>" + QApplication::translate("EAQtMainWindow","cyclic: ") + "</th><td>" + getMessc() + "</td></tr>"
                + "<tr><td>" + QApplication::translate("EAQtMainWindow","Ep: %1 mV</td><td>Ek: %2 mV").arg(_curve->Param(PARAM::Ep)).arg(_curve->Param(PARAM::Ek)) + "</td></tr>" );
    if ( _curve->Param(PARAM::method) == PARAM::method_dpv || _curve->Param(PARAM::method) == PARAM::method_sqw_osteryoung ) {
        pte.append(QApplication::translate("EAQtMainWindow","<tr><td colspan=2>dE: %1 mV</td></tr>").arg(_curve->Param(PARAM::dE)));
    } else if (_curve->Param(PARAM::method) == PARAM::method_npv) {
        pte.append(QApplication::translate("EAQtMainWindow","<tr><td colspan=2>E0: %1 mV</td></tr>").arg(_curve->Param(PARAM::E0)));
    }
    if ( isLSV ) {
        pte.append("<tr><td colspan=2>dE/dt: ");
        double speed = MEASUREMENT::LSVstepE[_curve->Param(PARAM::dEdt)]/MEASUREMENT::LSVtime[_curve->Param(PARAM::dEdt)];
        if ( speed < 0.025 )
            pte.append(tr("%1 mV/s").arg(speed*1000,0,'f',1));
        else if ( speed < 1 )
            pte.append(tr("%1 mV/s").arg(speed*1000,0,'f',0));
        else
            pte.append(tr("%1 V/s").arg(speed,0,'f',0));
        pte.append("</td></tr>");
    } else {
        pte.append(QApplication::translate("EAQtMainWindow", "<tr><td colspan=2>Estep: %1 mV</td></tr>").arg(_curve->Param(PARAM::Estep)));
        pte.append(QApplication::translate("EAQtMainWindow", "<tr><td>ts: %1 ms</td><td>tw: %2 ms</td></tr>").arg(_curve->Param(PARAM::tp)).arg(_curve->Param(PARAM::tw)));
    }
    pte.append(QApplication::translate("EAQtMainWindow","<tr><td colspan=2>td: %1 ms</td></tr>").arg(_curve->Param(PARAM::td)));
    pte.append(QApplication::translate("EAQtMainWindow","<tr><td colspan=2>current range: %1</td></tr>").arg(this->getCranage()));

    if ( _curve->Param(PARAM::breaknr) > 0 ) {
        pte.append(QApplication::translate("EAQtMainWindow","<tr><th colspan=2>Breaks:</td></tr>"));
        pte.append(QApplication::translate("EAQtMainWindow","<tr><th>E [mV]</th><th>t [min:sec]</th></tr>"));
        for ( int i=0; i<_curve->Param(PARAM::breaknr); ++i ) {
            pte.append(QApplication::translate("EAQtMainWindow", "<tr><td>%1</td><td>%2:%3</td></tr>").arg(_curve->Param(PARAM::breakE+i))
                                                                    .arg(_curve->Param(PARAM::breakmin+i), 2, 10, QChar('0'))
                                                                    .arg(_curve->Param(PARAM::breaksec+i), 2, 10, QChar('0'))
                       );
        }
    }
    pte.append("</table>");
    return pte;
}

QString DisplayCurveInfo::getGeneralInfo()
{
    QString info;
    info =    getMespv() + ", "
            + getMethod() + ", "
            + (_curve->Param(PARAM::method) != PARAM::method_lsv? getSampl() + ", " : "")
            + getMessc();
    return info;
}

QString DisplayCurveInfo::getCrangeInfo()
{
    return getCranage();
}

QString DisplayCurveInfo::getEl23Info()
{
    return getEl32();
}

QString DisplayCurveInfo::getElectrInfo()
{
    return getElectr();
}

inline QString DisplayCurveInfo::getMethod()
{
    QString method;
    switch ( _curve->Param(PARAM::method) ) {
    default:
    case PARAM::method_scv:
        method = QApplication::translate("EAQtMainWindow","SCV");
        break;
    case PARAM::method_npv:
        method = QApplication::translate("EAQtMainWindow","NPV");
        break;
    case PARAM::method_dpv:
        method = QApplication::translate("EAQtMainWindow","DPV");
        break;
    case PARAM::method_sqw_osteryoung:
        method = QApplication::translate("EAQtMainWindow","O.SQW");
        break;
    case PARAM::method_sqw_classic:
        method = QApplication::translate("EAQtMainWindow","[NEW] SQW");
        break;
    case PARAM::method_lsv:
        method = QApplication::translate("EAQtMainWindow","LSV");
        break;
    }
    return method;
}

inline QString DisplayCurveInfo::getSampl()
{
    QString sampl;
    switch( _curve->Param(PARAM::sampl) ) {
    default:
    case PARAM::sampl_single:
        sampl = QApplication::translate("EAQtMainWindow","single sampling");
        break;
    case PARAM::sampl_double:
        sampl= QApplication::translate("EAQtMainWindow","double sampling");
        break;
    }
    return sampl;
}

inline QString DisplayCurveInfo::getEl32()
{
    QString el23;
    switch ( _curve->Param(PARAM::el23) ) {
    default:
    case PARAM::el23_two:
        el23 = QApplication::translate("EAQtMainWindow","two electrodes");
        break;
    case PARAM::el23_three:
        el23 = QApplication::translate("EAQtMainWindow","three electrodes");
        break;
    case PARAM::el23_dummy:
        el23 = QApplication::translate("EAQtMainWindow","internal");
        break;
    }
    return el23;
}

inline QString DisplayCurveInfo::getMessc()
{
    QString messc;
    switch ( _curve->Param(PARAM::messc) ) {
    default:
    case PARAM::messc_single:
        messc = QApplication::translate("EAQtMainWindow","no");
        break;
    case PARAM::messc_cyclic:
        messc = QApplication::translate("EAQtMainWindow","yes");
        break;
    case PARAM::messc_multicyclic:
        messc = QApplication::translate("EAQtMainWindow","multicyclic");
        break;
    }
    return messc;
}

inline QString DisplayCurveInfo::getCranage()
{
    QString crange;
    if ( _curve->Param(PARAM::electr) < PARAM::electr_micro ) { // macroelectrode
        switch ( _curve->Param(PARAM::crange) ) {
        default:
        case PARAM::crange_macro_100mA:
            crange = "100 mA";
            break;
        case PARAM::crange_macro_10mA:
            crange = "10 mA";
            break;
        case PARAM::crange_macro_1mA:
            crange = "1 mA";
            break;
        case PARAM::crange_macro_100uA:
            crange = "100 µA";
            break;
        case PARAM::crange_macro_10uA:
            crange = "10 µA";
            break;
        case PARAM::crange_macro_1uA:
            crange = "1 µA";
            break;
        case PARAM::crange_macro_100nA:
            crange = "100 nA";
            break;
        }
    } else {
        switch ( _curve->Param(PARAM::crange) ) {
        default:
        case PARAM::crange_micro_5uA:
            crange = "5 µA";
            break;
        case PARAM::crange_micro_500nA:
            crange = "500 nA";
            break;
        case PARAM::crange_micro_50nA:
            crange = "50 nA";
            break;
        case PARAM::crange_micro_5nA:
            crange = "5 nA";
            break;
        }
    }
    return crange;
}

inline QString DisplayCurveInfo::getMespv()
{
    QString mespv;
    switch (_curve->Param(PARAM::mespv) ) {
    default:
    case PARAM::mespv_polarography:
        mespv = QApplication::translate("EAQtMainWindow","polarography");
        break;
    case PARAM::mespv_voltammetry:
        mespv= QApplication::translate("EAQtMainWindow","voltammetry");
        break;
    }
    return mespv;
}

inline QString DisplayCurveInfo::getElectr()
{
    QString electr;
    switch ( _curve->Param(PARAM::electr) ) {
    default:
    case PARAM::electr_dme:
        electr = "DME";
        break;
    case PARAM::electr_solid:
        electr = QApplication::translate("EAQtMainWindow","solid");
        break;
    case PARAM::electr_cgmde:
        electr = "CGMDE";
        break;
    case PARAM::electr_microDme:
        electr = "µDME";
        break;
    case PARAM::electr_microSolid:
        electr = QApplication::translate("EAQtMainWindow","µsolid");
        break;
    case PARAM::electr_microCgmde:
        electr = "µCGMDE";
        break;
    case PARAM::electr_multiSolid:
        electr = QApplication::translate("EAQtMainWindow","multielectrode");
    }
    return electr;
}
