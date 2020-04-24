//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: rdivide_helper.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//

// Include Files
#include "rt_nonfinite.h"
#include "arPLS2Ver2.h"
#include "rdivide_helper.h"
#include "arPLS2Ver2_emxutil.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *y
//                emxArray_real_T *z
// Return Type  : void
//
void rdivide_helper(const emxArray_real_T *y, emxArray_real_T *z)
{
  int i2;
  int loop_ub;
  i2 = z->size[0];
  z->size[0] = y->size[0];
  emxEnsureCapacity_real_T(z, i2);
  loop_ub = y->size[0];
  for (i2 = 0; i2 < loop_ub; i2++) {
    z->data[i2] = 1.0 / y->data[i2];
  }
}

//
// File trailer for rdivide_helper.cpp
//
// [EOF]
//
