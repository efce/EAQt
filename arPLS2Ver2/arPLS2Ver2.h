//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: arPLS2Ver2.h
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//
#ifndef ARPLS2VER2_H
#define ARPLS2VER2_H

// Include Files
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "arPLS2Ver2_types.h"

// Function Declarations
extern void arPLS2Ver2(const emxArray_real_T *y, double lambda, double ratio,
  double maxIter, double includeEndsNb, double threshold, emxArray_real_T *bkg,
  emxArray_real_T *weights, double *iter);

#endif

//
// File trailer for arPLS2Ver2.h
//
// [EOF]
//
