//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: xscal.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//

// Include Files
#include "rt_nonfinite.h"
#include "arPLS2Ver2.h"
#include "xscal.h"

// Function Definitions

//
// Arguments    : int n
//                double a
//                emxArray_real_T *x
//                int ix0
// Return Type  : void
//
void xscal(int n, double a, emxArray_real_T *x, int ix0)
{
  int i5;
  int k;
  i5 = (ix0 + n) - 1;
  for (k = ix0; k <= i5; k++) {
    x->data[k - 1] *= a;
  }
}

//
// File trailer for xscal.cpp
//
// [EOF]
//
