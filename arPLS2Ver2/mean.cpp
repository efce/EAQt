//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: mean.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//

// Include Files
#include "rt_nonfinite.h"
#include "arPLS2Ver2.h"
#include "mean.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *x
// Return Type  : double
//
double mean(const emxArray_real_T *x)
{
  double y;
  int vlen;
  int k;
  vlen = x->size[0];
  y = x->data[0];
  for (k = 2; k <= vlen; k++) {
    y += x->data[k - 1];
  }

  y /= (double)x->size[0];
  return y;
}

//
// File trailer for mean.cpp
//
// [EOF]
//
