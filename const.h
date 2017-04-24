
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
#ifndef _CONST_H_
#define _CONST_H_ 1

#include <QColor>
#include <vector>

namespace COLOR {
    const QColor regular = QColor(20,20,240);
    const QColor measurement = QColor(0,0,0);
    const QColor active = QColor(240,20,20);
}

namespace MEASUREMENT {
    const int LSVnrOfDEdt = 14;
    const double LSVstepE[14] = { // potential step [mV]
            0.125, 0.25, 0.25, 0.125, 0.25, 0.5, 0.25, 0.5,1.0, 0.25, 0.5, 1.0, 2.0, 5.0 };
    const double LSVtime[14] = { // time [ms]
            120, 120, 60, 20, 20, 20, 5, 5, 5, 1, 1, 1, 1, 1 };
    int const LSVstartData = 8;
    int const PVstartData = 8;
    const double scale_macro[7] = {650.0*1000.0, 80.0*1000.0, 10.0*1000.0,
                                             1250.0, 156.25, 19531.25/1000.0, 2441.4/1000.0};
    // mA, mA, mA, µA , µA, nA, nA
    static const double scale_micro[7]
            = {200.0*25.12, 20.0*25.12, 2.0*25.12, 0.2*25.12, 0, 0, 0};
    // mA, mA, mA, µA , µA, nA, nA

    const double multiply = (1.0 / 60.0 * 5000.0 / 32768.0 * 2.5 * -1.0 * 8.0 / 1000000.0 * 2.0);

    const int displayDelay = 10; //Delay display to update every 60 ms (in EAQtData::MesUpdate)
}

namespace NETWORK {
    enum {
        TxBufLength = 320,
        RxBufLength = 60
    };
}

//EA2PC:
namespace EA2PC_RECORDS  {
    enum : uint8_t {
        calibPV = 3,
        calibLSV = 5,
        recordPV = 1,
        recordLSV = 4,
        recordPause = 2,
        recordTestCGMDE = 6,
        startELSV = 7,
        startPVData = 8,
        startLSVData = 8
    };
}

// PC2EA:
namespace PC2EA_RECORODS {
    enum : uint8_t {
        recordPV = 10,
        recordLSV = 11,
        recordAcces = 12,
        recordStop = 13,
        recordCGMDE = 14,
        recordCGMDEstop = 15,
        recordPVE = 17
    };
}


namespace XAXIS{
    enum: int {
        potential = 0,
        time = 1,
        nonaveraged = 2
    };
}

namespace SELECT {
    enum: int {
        none = -1,
        all = -2,
    };
}

namespace PARAM {
    enum : int32_t {
        PARAMNUM = 64,
        VOL_CMAX = 50, // maximum number of curves in ".vol" file (not in .volt)
        VOL_PMAX = 62, // number of parameters of each curve in ".vol" file

        method = 0, //measurment method
        method_scv = 0,
        method_npv = 1,
        method_dpv = 2,
        method_sqw = 3,
        method_lsv = 4,

        sampl = 1, //type of sampling (usually single sampling for SCV/LSV and double sampling for NPV/DPV/SQW)
        sampl_single = 0,
        sampl_double = 1,

        el23 = 2, //cell setup dummy = internal
        el23_dummy = 2,
        el23_two = 0,
        el23_three = 1,

        aver = 3, //average the curve for # of measuremnts

        messc = 4, //cyclicity of measurement
        messc_single = 0,
        messc_cyclic = 1,
        messc_multicyclic = 2,

        crange = 5, //current range (other for micro and other for marco)
        crange_macro_100nA = 6,
        crange_macro_1uA = 5,
        crange_macro_10uA = 4,
        crange_macro_100uA = 3,
        crange_macro_1mA = 2,
        crange_macro_10mA = 1,
        crange_macro_100mA = 0,
        crange_micro_5nA = 3,
        crange_micro_50nA = 2,
        crange_micro_500nA = 1,
        crange_micro_5uA = 0,

        mespv = 6, //polarographic (DME) or voltamperometric (other)
        mespv_polarography = 0,
        mespv_voltamperometry = 1,

        electr = 7, //type of electrode used
        electr_macro = 0,
        electr_dme = 0,
        electr_solid = 1,
        electr_cgmde = 2,
        electr_micro = 3,
        electr_microDme = 3, //does not exists IRL
        electr_microSolid = 4,
        electr_microCgmde = 5,
        electr_multi = 6,
        electr_multiSolid = 6,

        multi = 8, //multielectrode measurement (with m164 electrode stand) -- bitewise description of aquisition channels
        Ep = 9,    //start potential [mV]
        Ek = 10,   //end potential [mV]
        Estep = 11, //potential step [mV]
        dEdt = 11,  //lsv potential change rate (according to lsv_stepE and lsv_time)
        E0 = 12,    //NPV base potential [mV]
        dE = 12,    //DPV/SQW impulse potential [mV]
        EstartLSV = 12,    //LSV multicyclic starting potential [mV]
        tp = 13,    //probing time [ms]
        tw = 14,    //waiting time [ms]
        tk = 15,    //unknown [ms]
        td = 15,    //before first potential step apply potential [ms]
        ts = 15,    //LSV multicyclic time of starting potential
        ptnr = 16,  //number of points of the curve
        kn = 17,    //hammer (knock power?)
        mix = 18,   //mixer speed

        breaknr = 19, //number of interruput (eg. preconcentration) 0-7
        breakmin = 20, //time in [min] of each interrupt (from 20 to 26)
        breaksec = 27, //time in [sec] of each interrupt (from 27 to 34)
        breakE = 34,   //potential in [sec] of each interrupt (from 34 to 40)

        impnr = 41, /* offset of nr of imp. - KER-KW  */
        imptime = 42, /* offset of impulse time         */
        inttime = 43, /* offset of interrupt time       */
        gtype = 44, /* offset of type of generation   */
        maxse = 45, /* nr of impulse in max. drop */

        param46 = 46, // not in use

        inf_smooth = 47, //was curve smoothed
        inf_smooth_no = 0,
        inf_smooth_yes = 1,

        inf_bkgs = 48, //was background subtracted
        inf_bkgs_no = 0,
        inf_bkgs_yes = 1,

        inf_move = 49, //was the baseline moved
        inf_move_no = 0,
        inf_move_yes = 1,

        sti = 50, //stirrer speed
        kp = 51,  //knock power
        kpt = 52, //knock pulse time

        Escheck = 53, //use Es potential for LSV measurement
        Escheck_no = 0,
        Escheck_yes = 1,

        date_day = 54,
        date_month = 55,
        date_year = 56,
        date_hour = 57,
        date_minutes = 58,
        date_seconds = 59,

        nonaveragedsampling = 60, // (old ms1) 0=regular sampling, value = sampling frequency in kHz

        pro = 61, // potential program in external file
        pro_no = 0,
        pro_yes = 1,

        xxbeg = 62,
        xxend = 63,
    };
}

#endif
