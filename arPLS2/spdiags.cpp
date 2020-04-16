//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: spdiags.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 16-Apr-2020 13:35:30
//

// Include Files
#include <cmath>
#include "rt_nonfinite.h"
#include "arPLS2.h"
#include "spdiags.h"
#include "arPLS2_emxutil.h"
#include "sparse.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *arg1
//                double arg3
//                double arg4
//                emxArray_real_T *res1_d
//                emxArray_int32_T *res1_colidx
//                emxArray_int32_T *res1_rowidx
// Return Type  : void
//
void spdiags(const emxArray_real_T *arg1, double arg3, double arg4,
             emxArray_real_T *res1_d, emxArray_int32_T *res1_colidx,
             emxArray_int32_T *res1_rowidx)
{
  int trueCount;
  double len[2];
  emxArray_int32_T *aRows;
  double minAdjustedDim_data_idx_0;
  int expl_temp;
  int loop_ub;
  emxArray_int32_T *aCols;
  emxArray_real_T *aDat;
  emxArray_real_T *idx;
  emxArray_real_T *i;
  emxArray_int32_T *r2;
  int k;
  trueCount = 0;
  if ((0.0 >= -arg3 + 1.0) && (0.0 <= arg4 - 1.0)) {
    trueCount = 1;
  }

  len[0] = 0.0;
  if (0 <= trueCount - 1) {
    if ((arg3 < arg4) || rtIsNaN(arg4)) {
      minAdjustedDim_data_idx_0 = arg3;
    } else {
      minAdjustedDim_data_idx_0 = arg4;
    }

    len[1] = (minAdjustedDim_data_idx_0 - 1.0) + 1.0;
  }

  emxInit_int32_T(&aRows, 1);
  expl_temp = aRows->size[0];
  loop_ub = (int)len[trueCount];
  aRows->size[0] = loop_ub;
  emxEnsureCapacity_int32_T(aRows, expl_temp);
  for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
    aRows->data[expl_temp] = 0;
  }

  emxInit_int32_T(&aCols, 1);
  expl_temp = aCols->size[0];
  aCols->size[0] = loop_ub;
  emxEnsureCapacity_int32_T(aCols, expl_temp);
  for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
    aCols->data[expl_temp] = 0;
  }

  emxInit_real_T(&aDat, 1);
  expl_temp = aDat->size[0];
  aDat->size[0] = loop_ub;
  emxEnsureCapacity_real_T(aDat, expl_temp);
  for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
    aDat->data[expl_temp] = 0.0;
  }

  emxInit_real_T(&idx, 2);
  emxInit_real_T(&i, 1);
  emxInit_int32_T(&r2, 2);
  for (k = 0; k < trueCount; k++) {
    if (minAdjustedDim_data_idx_0 < 1.0) {
      idx->size[0] = 1;
      idx->size[1] = 0;
    } else {
      expl_temp = idx->size[0] * idx->size[1];
      idx->size[0] = 1;
      loop_ub = (int)std::floor(minAdjustedDim_data_idx_0 - 1.0);
      idx->size[1] = loop_ub + 1;
      emxEnsureCapacity_real_T(idx, expl_temp);
      for (expl_temp = 0; expl_temp <= loop_ub; expl_temp++) {
        idx->data[expl_temp] = 1.0 + (double)expl_temp;
      }
    }

    expl_temp = i->size[0];
    i->size[0] = idx->size[1];
    emxEnsureCapacity_real_T(i, expl_temp);
    loop_ub = idx->size[1];
    for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
      i->data[expl_temp] = idx->data[expl_temp];
    }

    if (len[1] < 1.0) {
      idx->size[0] = 1;
      idx->size[1] = 0;
    } else {
      expl_temp = idx->size[0] * idx->size[1];
      idx->size[0] = 1;
      loop_ub = (int)std::floor(len[1] - 1.0);
      idx->size[1] = loop_ub + 1;
      emxEnsureCapacity_real_T(idx, expl_temp);
      for (expl_temp = 0; expl_temp <= loop_ub; expl_temp++) {
        idx->data[expl_temp] = 1.0 + (double)expl_temp;
      }
    }

    expl_temp = r2->size[0] * r2->size[1];
    r2->size[0] = 1;
    r2->size[1] = idx->size[1];
    emxEnsureCapacity_int32_T(r2, expl_temp);
    loop_ub = idx->size[0] * idx->size[1];
    for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
      r2->data[expl_temp] = (int)idx->data[expl_temp];
    }

    loop_ub = r2->size[0] * r2->size[1];
    for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
      aRows->data[r2->data[expl_temp] - 1] = (int)i->data[expl_temp];
    }

    expl_temp = r2->size[0] * r2->size[1];
    r2->size[0] = 1;
    r2->size[1] = idx->size[1];
    emxEnsureCapacity_int32_T(r2, expl_temp);
    loop_ub = idx->size[0] * idx->size[1];
    for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
      r2->data[expl_temp] = (int)idx->data[expl_temp];
    }

    loop_ub = r2->size[0] * r2->size[1];
    for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
      aCols->data[r2->data[expl_temp] - 1] = (int)i->data[expl_temp];
    }

    expl_temp = r2->size[0] * r2->size[1];
    r2->size[0] = 1;
    r2->size[1] = idx->size[1];
    emxEnsureCapacity_int32_T(r2, expl_temp);
    loop_ub = idx->size[0] * idx->size[1];
    for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
      r2->data[expl_temp] = (int)idx->data[expl_temp];
    }

    loop_ub = r2->size[0] * r2->size[1];
    for (expl_temp = 0; expl_temp < loop_ub; expl_temp++) {
      aDat->data[r2->data[expl_temp] - 1] = arg1->data[(int)i->data[expl_temp] -
        1];
    }
  }

  emxFree_int32_T(&r2);
  emxFree_real_T(&i);
  emxFree_real_T(&idx);
  sparse(aRows, aCols, aDat, arg3, arg4, res1_d, res1_colidx, res1_rowidx,
         &trueCount, &expl_temp);
  emxFree_real_T(&aDat);
  emxFree_int32_T(&aCols);
  emxFree_int32_T(&aRows);
}

//
// File trailer for spdiags.cpp
//
// [EOF]
//
