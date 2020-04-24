//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: exp.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//

// Include Files
#include <cmath>
#include "rt_nonfinite.h"
#include "arPLS2Ver2.h"
#include "exp.h"

// Function Definitions

//
// Arguments    : emxArray_real_T *x
// Return Type  : void
//
void b_exp(emxArray_real_T *x)
{
  int nx;
  int k;
  nx = x->size[0];
  for (k = 0; k < nx; k++) {
    x->data[k] = std::exp(x->data[k]);
  }
}

//
// File trailer for exp.cpp
//
// [EOF]
//
