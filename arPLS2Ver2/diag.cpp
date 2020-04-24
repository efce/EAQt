//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: diag.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//

// Include Files
#include "rt_nonfinite.h"
#include "arPLS2Ver2.h"
#include "diag.h"
#include "arPLS2Ver2_emxutil.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *v
//                emxArray_real_T *d
// Return Type  : void
//
void diag(const emxArray_real_T *v, emxArray_real_T *d)
{
  int nv;
  int unnamed_idx_0;
  int unnamed_idx_1;
  int i1;
  nv = v->size[0];
  unnamed_idx_0 = v->size[0];
  unnamed_idx_1 = v->size[0];
  i1 = d->size[0] * d->size[1];
  d->size[0] = unnamed_idx_0;
  d->size[1] = unnamed_idx_1;
  emxEnsureCapacity_real_T(d, i1);
  unnamed_idx_0 *= unnamed_idx_1;
  for (i1 = 0; i1 < unnamed_idx_0; i1++) {
    d->data[i1] = 0.0;
  }

  for (unnamed_idx_0 = 0; unnamed_idx_0 < nv; unnamed_idx_0++) {
    d->data[unnamed_idx_0 + d->size[0] * unnamed_idx_0] = v->data[unnamed_idx_0];
  }
}

//
// File trailer for diag.cpp
//
// [EOF]
//
