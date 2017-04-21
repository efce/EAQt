#ifndef DISPLAYCURVE_H
#define DISPLAYCURVE_H

#include "curve.h"

class DisplayCurveInfo
{
public:
    DisplayCurveInfo(Curve* c);
    QString getGeneralInfo();
    QString getCrangeInfo();
    QString getEl23Info();
    QString getElectrInfo();
    QString getHTMLInfo();
private:
    Curve* _curve;
    QString getMethod();
    QString getSampl();
    QString getEl32();
    QString getMessc();
    QString getCranage();
    QString getMespv();
    QString getElectr();

};

#endif // DISPLAYCURVE_H
