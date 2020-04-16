//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: sparse1.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 16-Apr-2020 13:35:30
//

// Include Files
#include "rt_nonfinite.h"
#include "arPLS2.h"
#include "sparse1.h"
#include "arPLS2_emxutil.h"

// Function Definitions

//
// Arguments    : const emxArray_int32_T *idx
//                emxArray_int32_T *y
// Return Type  : void
//
void permuteVector(const emxArray_int32_T *idx, emxArray_int32_T *y)
{
  emxArray_int32_T *t;
  int ny;
  int k;
  int loop_ub;
  emxInit_int32_T(&t, 1);
  ny = y->size[0];
  k = t->size[0];
  t->size[0] = y->size[0];
  emxEnsureCapacity_int32_T(t, k);
  loop_ub = y->size[0];
  for (k = 0; k < loop_ub; k++) {
    t->data[k] = y->data[k];
  }

  for (k = 0; k < ny; k++) {
    y->data[k] = t->data[idx->data[k] - 1];
  }

  emxFree_int32_T(&t);
}

//
// Arguments    : const emxArray_real_T *a_d
//                const emxArray_int32_T *a_colidx
//                const emxArray_int32_T *a_rowidx
//                const emxArray_real_T *b
//                emxArray_real_T *s
// Return Type  : void
//
void sparse_plus(const emxArray_real_T *a_d, const emxArray_int32_T *a_colidx,
                 const emxArray_int32_T *a_rowidx, const emxArray_real_T *b,
                 emxArray_real_T *s)
{
  int sm;
  int sn;
  int col;
  int idx;
  int row;
  double val;
  sm = s->size[0] * s->size[1];
  s->size[0] = b->size[0];
  s->size[1] = b->size[1];
  emxEnsureCapacity_real_T(s, sm);
  sn = b->size[0] * b->size[1];
  for (sm = 0; sm < sn; sm++) {
    s->data[sm] = 0.0;
  }

  sm = b->size[0];
  sn = b->size[1];
  for (col = 0; col < sn; col++) {
    idx = a_colidx->data[col];
    for (row = 0; row < sm; row++) {
      if ((idx < a_colidx->data[col + 1]) && (row + 1 == a_rowidx->data[idx - 1]))
      {
        val = a_d->data[idx - 1] + b->data[row + b->size[0] * col];
        idx++;
      } else {
        val = b->data[row + b->size[0] * col];
      }

      s->data[row + s->size[0] * col] = val;
    }
  }
}

//
// File trailer for sparse1.cpp
//
// [EOF]
//
