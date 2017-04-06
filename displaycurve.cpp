#include "displaycurve.h"

DisplayCurve::DisplayCurve(Curve* c)
{
    _curve = c;
}

QString DisplayCurve::getHTMLInfo()
{
    //TODO for total revamp ...
    QString pte;
    bool isLSV = (_curve->Param(PARAM::method) == PARAM::method_lsv);
    pte.append( "<h4>" + QApplication::translate("EAQtMainWindow","General") + "</h4>"
                + "<tr><td style='width: 120px'></td><td style='width:120px'></td></tr>"
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
                                                                     + getMethod() + ", "
                                                                     + (isLSV?","+getSampl():"")
                                                                     + "</td></tr>"
                + "<tr><th>" + QApplication::translate("EAQtMainWindow","electrode: ") +"</th><td>" + getElectr() + "</td></tr>"
                + "<tr><th>" + QApplication::translate("EAQtMainWindow","cyclic: ") + "</th><td>" + getMessc() + "</td></tr>"
                + "<tr><td>" + QApplication::translate("EAQtMainWindow","Ep: %1 mV</td><td>Ek: %2 mV").arg(_curve->Param(PARAM::Ep)).arg(_curve->Param(PARAM::Ek)) + "</td></tr>" );
    if ( isLSV ) {
        pte.append("<tr><td colspan=2>dE/dt: ");
        double speed = MEASUREMENT::LSVstepE[_curve->Param(PARAM::dEdt)]/MEASUREMENT::LSVtime[_curve->Param(PARAM::dEdt)];
        if ( speed < 0.025 )
            pte.append("%1 mV/s").arg(speed*1000,0,'f',1);
        else if ( speed < 1 )
            pte.append("%1 mV/s").arg(speed*1000,0,'f',0);
        else
            pte.append("%1 V/s").arg(speed,0,'f',0);
        pte.append("</td></tr>");
    } else {
        pte.append(QString("<tr><td colspan=2>Estep: %1 mV</td></tr>").arg(_curve->Param(PARAM::Estep)));
        pte.append(QString("<tr><td>ts: %1 ms</td><td>tw: %2 ms</td></tr>").arg(_curve->Param(PARAM::tp)).arg(_curve->Param(PARAM::tw)));
    }
    pte.append(QString("<tr><td colspan=2>td: %1 ms</td></tr>").arg(_curve->Param(PARAM::td)));
    pte.append(QString("<tr><td colspan=2>current range: %1</td></tr>").arg(this->getCranage()));

    if ( _curve->Param(PARAM::breaknr) > 0 ) {
        pte.append(QApplication::translate("EAQtMainWindow","<tr><th colspan=2>Breaks:</td></tr>"));
        pte.append(QApplication::translate("EAQtMainWindow","<tr><th>E [mV]</th><th>t [min:sec]</th></tr>"));
        for ( int i=0; i<_curve->Param(PARAM::breaknr); ++i ) {
            pte.append(QString("<tr><td>%1</td><td>%2:%3</td></tr>").arg(_curve->Param(PARAM::breakE+i))
                                                                    .arg(_curve->Param(PARAM::breakmin+i), 2, 10, QChar('0'))
                                                                    .arg(_curve->Param(PARAM::breaksec+i), 2, 10, QChar('0'))
                       );
        }
    }
    pte.append("</table>");
    return pte;
}

QString DisplayCurve::getGeneralInfo()
{
    QString info;
    info =    getMespv() + ", "
            + getMethod() + ", "
            + (_curve->Param(PARAM::method) != PARAM::method_lsv? getSampl() + ", " : "")
            + getMessc();
    return info;
}

QString DisplayCurve::getCrangeInfo()
{
    return getCranage();
}

QString DisplayCurve::getEl23Info()
{
    return getEl32();
}

QString DisplayCurve::getElectrInfo()
{
    return getElectr();
}

inline QString DisplayCurve::getMethod()
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
    case PARAM::method_sqw:
        method = QApplication::translate("EAQtMainWindow","SQW");
        break;
    case PARAM::method_lsv:
        method = QApplication::translate("EAQtMainWindow","LSV");
        break;
    }
    return method;
}

inline QString DisplayCurve::getSampl()
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

inline QString DisplayCurve::getEl32()
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

inline QString DisplayCurve::getMessc()
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

inline QString DisplayCurve::getCranage()
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

inline QString DisplayCurve::getMespv()
{
    QString mespv;
    switch (_curve->Param(PARAM::mespv) ) {
    default:
    case PARAM::mespv_polarography:
        mespv = QApplication::translate("EAQtMainWindow","polarography");
        break;
    case PARAM::mespv_voltamperometry:
        mespv= QApplication::translate("EAQtMainWindow","voltammetry");
        break;
    }
    return mespv;
}

inline QString DisplayCurve::getElectr()
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
