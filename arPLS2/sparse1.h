//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: sparse1.h
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 16-Apr-2020 13:35:30
//
#ifndef SPARSE1_H
#define SPARSE1_H

// Include Files
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "arPLS2_types.h"

// Function Declarations
extern void permuteVector(const emxArray_int32_T *idx, emxArray_int32_T *y);
extern void sparse_plus(const emxArray_real_T *a_d, const emxArray_int32_T
  *a_colidx, const emxArray_int32_T *a_rowidx, const emxArray_real_T *b,
  emxArray_real_T *s);

#endif

//
// File trailer for sparse1.h
//
// [EOF]
//
