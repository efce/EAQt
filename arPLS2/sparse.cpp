//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: sparse.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 16-Apr-2020 13:35:30
//

// Include Files
#include "rt_nonfinite.h"
#include "arPLS2.h"
#include "sparse.h"
#include "arPLS2_emxutil.h"
#include "sparse1.h"
#include "introsort.h"

// Function Definitions

//
// Arguments    : const emxArray_int32_T *varargin_1
//                const emxArray_int32_T *varargin_2
//                const emxArray_real_T *varargin_3
//                double varargin_4
//                double varargin_5
//                emxArray_real_T *y_d
//                emxArray_int32_T *y_colidx
//                emxArray_int32_T *y_rowidx
//                int *y_m
//                int *y_n
// Return Type  : void
//
void sparse(const emxArray_int32_T *varargin_1, const emxArray_int32_T
            *varargin_2, const emxArray_real_T *varargin_3, double varargin_4,
            double varargin_5, emxArray_real_T *y_d, emxArray_int32_T *y_colidx,
            emxArray_int32_T *y_rowidx, int *y_m, int *y_n)
{
  emxArray_int32_T *ridxInt;
  int nc;
  int ns;
  int i3;
  int ridx;
  emxArray_int32_T *cidxInt;
  emxArray_int32_T *sortedIndices;
  cell_wrap_2 this_tunableEnvironment[2];
  int i4;
  int c;
  double val;
  emxInit_int32_T(&ridxInt, 1);
  nc = varargin_2->size[0];
  ns = varargin_1->size[0];
  i3 = ridxInt->size[0];
  ridxInt->size[0] = varargin_1->size[0];
  emxEnsureCapacity_int32_T(ridxInt, i3);
  for (ridx = 0; ridx < ns; ridx++) {
    ridxInt->data[ridx] = varargin_1->data[ridx];
  }

  emxInit_int32_T(&cidxInt, 1);
  ns = varargin_2->size[0];
  i3 = cidxInt->size[0];
  cidxInt->size[0] = varargin_2->size[0];
  emxEnsureCapacity_int32_T(cidxInt, i3);
  for (ridx = 0; ridx < ns; ridx++) {
    cidxInt->data[ridx] = varargin_2->data[ridx];
  }

  emxInit_int32_T(&sortedIndices, 1);
  i3 = sortedIndices->size[0];
  sortedIndices->size[0] = varargin_2->size[0];
  emxEnsureCapacity_int32_T(sortedIndices, i3);
  for (ridx = 0; ridx < nc; ridx++) {
    sortedIndices->data[ridx] = ridx + 1;
  }

  emxInitMatrix_cell_wrap_2(this_tunableEnvironment);
  i3 = this_tunableEnvironment[0].f1->size[0];
  this_tunableEnvironment[0].f1->size[0] = cidxInt->size[0];
  emxEnsureCapacity_int32_T(this_tunableEnvironment[0].f1, i3);
  ns = cidxInt->size[0];
  for (i3 = 0; i3 < ns; i3++) {
    this_tunableEnvironment[0].f1->data[i3] = cidxInt->data[i3];
  }

  i3 = this_tunableEnvironment[1].f1->size[0];
  this_tunableEnvironment[1].f1->size[0] = ridxInt->size[0];
  emxEnsureCapacity_int32_T(this_tunableEnvironment[1].f1, i3);
  ns = ridxInt->size[0];
  for (i3 = 0; i3 < ns; i3++) {
    this_tunableEnvironment[1].f1->data[i3] = ridxInt->data[i3];
  }

  introsort(sortedIndices, cidxInt->size[0], this_tunableEnvironment);
  permuteVector(sortedIndices, cidxInt);
  permuteVector(sortedIndices, ridxInt);
  emxFreeMatrix_cell_wrap_2(this_tunableEnvironment);
  if (varargin_2->size[0] >= 1) {
    ns = varargin_2->size[0];
  } else {
    ns = 1;
  }

  i3 = y_d->size[0];
  y_d->size[0] = ns;
  emxEnsureCapacity_real_T(y_d, i3);
  for (i3 = 0; i3 < ns; i3++) {
    y_d->data[i3] = 0.0;
  }

  i3 = y_colidx->size[0];
  i4 = (int)varargin_5;
  y_colidx->size[0] = i4 + 1;
  emxEnsureCapacity_int32_T(y_colidx, i3);
  y_colidx->data[0] = 1;
  i3 = y_rowidx->size[0];
  y_rowidx->size[0] = ns;
  emxEnsureCapacity_int32_T(y_rowidx, i3);
  for (i3 = 0; i3 < ns; i3++) {
    y_rowidx->data[i3] = 0;
  }

  ns = 0;
  for (c = 0; c < i4; c++) {
    ridx = 1 + c;
    while ((ns + 1 <= nc) && (cidxInt->data[ns] == ridx)) {
      y_rowidx->data[ns] = ridxInt->data[ns];
      ns++;
    }

    y_colidx->data[ridx] = ns + 1;
  }

  emxFree_int32_T(&cidxInt);
  emxFree_int32_T(&ridxInt);
  for (ridx = 0; ridx < nc; ridx++) {
    y_d->data[ridx] = varargin_3->data[sortedIndices->data[ridx] - 1];
  }

  emxFree_int32_T(&sortedIndices);
  ns = 1;
  i3 = y_colidx->size[0];
  for (c = 0; c <= i3 - 2; c++) {
    ridx = y_colidx->data[c];
    y_colidx->data[c] = ns;
    while (ridx < y_colidx->data[c + 1]) {
      val = 0.0;
      nc = y_rowidx->data[ridx - 1];
      while ((ridx < y_colidx->data[c + 1]) && (y_rowidx->data[ridx - 1] == nc))
      {
        val += y_d->data[ridx - 1];
        ridx++;
      }

      if (val != 0.0) {
        y_d->data[ns - 1] = val;
        y_rowidx->data[ns - 1] = nc;
        ns++;
      }
    }
  }

  y_colidx->data[y_colidx->size[0] - 1] = ns;
  *y_m = (int)varargin_4;
  *y_n = i4;
}

//
// File trailer for sparse.cpp
//
// [EOF]
//
