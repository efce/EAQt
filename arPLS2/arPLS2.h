//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: arPLS2.h
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 16-Apr-2020 13:35:30
//
#ifndef ARPLS2_H
#define ARPLS2_H

// Include Files
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "arPLS2_types.h"

// Function Declarations
extern void arPLS2(const emxArray_real_T *y, double lambda, double ratio, int
                   maxIter, int includeEndsNb, double threshold, emxArray_real_T
                   *bkg, emxArray_real_T *weights);

#endif

//
// File trailer for arPLS2.h
//
// [EOF]
//
