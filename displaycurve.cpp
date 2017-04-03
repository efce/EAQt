#include "displaycurve.h"

DisplayCurve::DisplayCurve(Curve* c)
{
    _curve = c;
}

QString DisplayCurve::getHTMLInfo()
{
    QString pte;
    pte.append( "<h3>" + QApplication::translate("EAQtMainWindow","General") + "</h3>"
                + "<p><strong>" + QApplication::translate("EAQtMainWindow","name: ") + "</strong>" + _curve->CName() + "</p>"
                + "<p><strong>" + QApplication::translate("EAQtMainWindow","file: ") + "</strong>" + _curve->FName() + "</p>"
                + "<p><strong>" + QApplication::translate("EAQtMainWindow","date: ") + "</strong>" + QString("%1-%2-%3 %4:%5:%6").arg(_curve->Param(PARAM::date_year))
                                                .arg(_curve->Param(PARAM::date_month), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_day), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_hour), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_minutes), 2, 10, QChar('0'))
                                                .arg(_curve->Param(PARAM::date_seconds), 2, 10, QChar('0'))
                                                    + "</p>"
                + "<h3>" + QApplication::translate("EAQtMainWindow","Measurement") + "</h3>"
                + "<p>" + QApplication::translate("EAQtMainWindow","setup: ") + getEl32() + "</p>"
                + "<p>" + QApplication::translate("EAQtMainWindow","type: ") + getMespv() + ", "
                                                                     + getMethod() + ", "
                                                                     + (_curve->Param(PARAM::method)!=PARAM::method_lsv?","+getSampl():"")
                                                                     + "</p>"
                + "<p>" + QApplication::translate("EAQtMainWindow","electrode: ") + getElectr() + "</p>"
                + "<p>" + QApplication::translate("EAQtMainWindow","cyclic: ") + getMessc() + "</p>"
                + "<p>" + QApplication::translate("EAQtMainWindow","current range: ") + getCranage() + "</p>" );
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
