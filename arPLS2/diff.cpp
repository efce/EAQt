//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: diff.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 16-Apr-2020 13:35:30
//

// Include Files
#include "rt_nonfinite.h"
#include "arPLS2.h"
#include "diff.h"
#include "arPLS2_emxutil.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *x
//                emxArray_real_T *y
// Return Type  : void
//
void diff(const emxArray_real_T *x, emxArray_real_T *y)
{
  int dimSize;
  int orderForDim;
  int ySize_idx_1;
  int newDimSize;
  int i2;
  int ixStart;
  int iyStart;
  int r;
  int ixLead;
  int iyLead;
  double work_data[2];
  double tmp1;
  double tmp2;
  int m;
  int k;
  dimSize = x->size[0];
  if (x->size[0] == 0) {
    ySize_idx_1 = x->size[1];
    y->size[0] = 0;
    y->size[1] = ySize_idx_1;
  } else {
    orderForDim = x->size[0] - 1;
    if (orderForDim >= 2) {
      orderForDim = 2;
    }

    if (orderForDim < 1) {
      ySize_idx_1 = x->size[1];
      y->size[0] = 0;
      y->size[1] = ySize_idx_1;
    } else {
      newDimSize = x->size[0] - orderForDim;
      ySize_idx_1 = x->size[1];
      i2 = y->size[0] * y->size[1];
      y->size[0] = newDimSize;
      y->size[1] = ySize_idx_1;
      emxEnsureCapacity_real_T(y, i2);
      if ((y->size[0] != 0) && (y->size[1] != 0)) {
        ySize_idx_1 = x->size[1];
        ixStart = 1;
        iyStart = 0;
        for (r = 0; r < ySize_idx_1; r++) {
          ixLead = ixStart;
          iyLead = iyStart;
          work_data[0] = x->data[ixStart - 1];
          if (orderForDim >= 2) {
            tmp1 = x->data[ixStart];
            tmp2 = tmp1;
            tmp1 -= work_data[0];
            work_data[0] = tmp2;
            work_data[1] = tmp1;
            ixLead = ixStart + 1;
          }

          i2 = orderForDim + 1;
          for (m = i2; m <= dimSize; m++) {
            tmp1 = x->data[ixLead];
            for (k = 0; k < orderForDim; k++) {
              tmp2 = work_data[k];
              work_data[k] = tmp1;
              tmp1 -= tmp2;
            }

            ixLead++;
            y->data[iyLead] = tmp1;
            iyLead++;
          }

          ixStart += dimSize;
          iyStart += newDimSize;
        }
      }
    }
  }
}

//
// File trailer for diff.cpp
//
// [EOF]
//
