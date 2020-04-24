//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: arPLS2Ver2.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//

// Include Files
#include <cmath>
#include "rt_nonfinite.h"
#include "arPLS2Ver2.h"
#include "arPLS2Ver2_emxutil.h"
#include "mldivide.h"
#include "diag.h"
#include "norm.h"
#include "rdivide_helper.h"
#include "exp.h"
#include "std.h"
#include "mean.h"

// Function Definitions

//
// Estimate basline with arPLS
// Arguments    : const emxArray_real_T *y
//                double lambda
//                double ratio
//                double maxIter
//                double includeEndsNb
//                double threshold
//                emxArray_real_T *bkg
//                emxArray_real_T *weights
//                double *iter
// Return Type  : void
//
void arPLS2Ver2(const emxArray_real_T *y, double lambda, double ratio, double
                maxIter, double includeEndsNb, double threshold, emxArray_real_T
                *bkg, emxArray_real_T *weights, double *iter)
{
  emxArray_real_T *H;
  int varargin_1;
  int nv;
  int ySize_idx_0;
  int ySize_idx_1;
  int i0;
  int aoffset;
  int j;
  int dimSize;
  emxArray_real_T *b_y1;
  int orderForDim;
  int newDimSize;
  emxArray_real_T *a;
  int iy;
  int k;
  int ixLead;
  int n;
  int iyLead;
  double work_data[2];
  double tmp1;
  double tmp2;
  emxArray_real_T *W;
  emxArray_real_T *d;
  emxArray_real_T *dn;
  emxArray_real_T *b_dn;
  emxArray_real_T *b_W;
  int exitg1;
  boolean_T exitg2;
  unsigned int ind1;
  emxArray_int32_T *r0;
  emxInit_real_T(&H, 2);

  //  Sung-June Baek, Aaron Park, Young-Jin Ahna and Jaebum Choo, Analyst, 2015, 140, 250 
  varargin_1 = y->size[0] - 1;
  nv = y->size[0];
  ySize_idx_0 = y->size[0];
  ySize_idx_1 = y->size[0];
  i0 = H->size[0] * H->size[1];
  H->size[0] = ySize_idx_0;
  H->size[1] = ySize_idx_1;
  emxEnsureCapacity_real_T(H, i0);
  aoffset = ySize_idx_0 * ySize_idx_1;
  for (i0 = 0; i0 < aoffset; i0++) {
    H->data[i0] = 0.0;
  }

  for (j = 0; j < nv; j++) {
    H->data[j + H->size[0] * j] = 1.0;
  }

  dimSize = H->size[0];
  emxInit_real_T(&b_y1, 2);
  if (H->size[0] == 0) {
    ySize_idx_1 = H->size[1];
    b_y1->size[0] = 0;
    b_y1->size[1] = ySize_idx_1;
  } else {
    orderForDim = H->size[0] - 1;
    if (orderForDim >= 2) {
      orderForDim = 2;
    }

    if (orderForDim < 1) {
      ySize_idx_1 = H->size[1];
      b_y1->size[0] = 0;
      b_y1->size[1] = ySize_idx_1;
    } else {
      newDimSize = H->size[0] - orderForDim;
      ySize_idx_1 = H->size[1];
      i0 = b_y1->size[0] * b_y1->size[1];
      b_y1->size[0] = newDimSize;
      b_y1->size[1] = ySize_idx_1;
      emxEnsureCapacity_real_T(b_y1, i0);
      if ((b_y1->size[0] != 0) && (b_y1->size[1] != 0)) {
        ySize_idx_0 = H->size[1];
        nv = 1;
        ySize_idx_1 = 0;
        for (aoffset = 0; aoffset < ySize_idx_0; aoffset++) {
          ixLead = nv;
          iyLead = ySize_idx_1;
          work_data[0] = (signed char)H->data[nv - 1];
          if (orderForDim >= 2) {
            tmp1 = (signed char)H->data[nv];
            i0 = (int)tmp1;
            tmp1 -= (double)(int)work_data[0];
            work_data[0] = i0;
            work_data[1] = tmp1;
            ixLead = nv + 1;
          }

          i0 = orderForDim + 1;
          for (iy = i0; iy <= dimSize; iy++) {
            tmp1 = (signed char)H->data[ixLead];
            for (k = 0; k < orderForDim; k++) {
              tmp2 = work_data[k];
              work_data[k] = tmp1;
              tmp1 -= tmp2;
            }

            ixLead++;
            b_y1->data[iyLead] = tmp1;
            iyLead++;
          }

          nv += dimSize;
          ySize_idx_1 += newDimSize;
        }
      }
    }
  }

  emxInit_real_T(&a, 2);

  //  my change
  i0 = a->size[0] * a->size[1];
  a->size[0] = b_y1->size[1];
  a->size[1] = b_y1->size[0];
  emxEnsureCapacity_real_T(a, i0);
  aoffset = b_y1->size[0];
  for (i0 = 0; i0 < aoffset; i0++) {
    ySize_idx_0 = b_y1->size[1];
    for (k = 0; k < ySize_idx_0; k++) {
      a->data[k + a->size[0] * i0] = b_y1->data[i0 + b_y1->size[0] * k];
    }
  }

  if ((a->size[1] == 1) || (b_y1->size[0] == 1)) {
    i0 = H->size[0] * H->size[1];
    H->size[0] = a->size[0];
    H->size[1] = b_y1->size[1];
    emxEnsureCapacity_real_T(H, i0);
    aoffset = a->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      ySize_idx_0 = b_y1->size[1];
      for (k = 0; k < ySize_idx_0; k++) {
        H->data[i0 + H->size[0] * k] = 0.0;
        nv = a->size[1];
        for (ySize_idx_1 = 0; ySize_idx_1 < nv; ySize_idx_1++) {
          H->data[i0 + H->size[0] * k] += a->data[i0 + a->size[0] * ySize_idx_1]
            * b_y1->data[ySize_idx_1 + b_y1->size[0] * k];
        }
      }
    }
  } else {
    iy = a->size[0];
    ySize_idx_0 = a->size[1];
    n = b_y1->size[1];
    i0 = H->size[0] * H->size[1];
    H->size[0] = a->size[0];
    H->size[1] = b_y1->size[1];
    emxEnsureCapacity_real_T(H, i0);
    for (j = 0; j < n; j++) {
      nv = j * iy;
      ySize_idx_1 = j * ySize_idx_0;
      for (ixLead = 0; ixLead < iy; ixLead++) {
        H->data[nv + ixLead] = 0.0;
      }

      for (k = 0; k < ySize_idx_0; k++) {
        aoffset = k * iy;
        tmp1 = b_y1->data[ySize_idx_1 + k];
        for (ixLead = 0; ixLead < iy; ixLead++) {
          i0 = nv + ixLead;
          H->data[i0] += tmp1 * a->data[aoffset + ixLead];
        }
      }
    }
  }

  emxFree_real_T(&a);
  emxFree_real_T(&b_y1);
  i0 = H->size[0] * H->size[1];
  k = H->size[0] * H->size[1];
  emxEnsureCapacity_real_T(H, k);
  aoffset = i0 - 1;
  for (i0 = 0; i0 <= aoffset; i0++) {
    H->data[i0] *= lambda;
  }

  i0 = weights->size[0];
  weights->size[0] = y->size[0];
  emxEnsureCapacity_real_T(weights, i0);
  aoffset = y->size[0];
  for (i0 = 0; i0 < aoffset; i0++) {
    weights->data[i0] = 1.0;
  }

  *iter = 0.0;
  emxInit_real_T(&W, 2);
  emxInit_real_T(&d, 1);
  emxInit_real_T(&dn, 1);
  emxInit_real_T(&b_dn, 1);
  emxInit_real_T(&b_W, 2);
  do {
    exitg1 = 0;
    nv = weights->size[0];
    ySize_idx_0 = weights->size[0];
    ySize_idx_1 = weights->size[0];
    i0 = W->size[0] * W->size[1];
    W->size[0] = ySize_idx_0;
    W->size[1] = ySize_idx_1;
    emxEnsureCapacity_real_T(W, i0);
    aoffset = ySize_idx_0 * ySize_idx_1;
    for (i0 = 0; i0 < aoffset; i0++) {
      W->data[i0] = 0.0;
    }

    for (j = 0; j < nv; j++) {
      W->data[j + W->size[0] * j] = weights->data[j];
    }

    //  my change
    // Cholesky decomposition
    i0 = W->size[0] * W->size[1];
    k = W->size[0] * W->size[1];
    emxEnsureCapacity_real_T(W, k);
    aoffset = i0 - 1;
    for (i0 = 0; i0 <= aoffset; i0++) {
      W->data[i0] += H->data[i0];
    }

    n = W->size[1];
    if (W->size[1] != 0) {
      iyLead = W->size[0];
      dimSize = -1;
      if (W->size[0] != 0) {
        orderForDim = 0;
        j = 0;
        exitg2 = false;
        while ((!exitg2) && (j <= iyLead - 1)) {
          ySize_idx_0 = orderForDim + j;
          tmp1 = 0.0;
          if (j >= 1) {
            newDimSize = orderForDim;
            iy = orderForDim;
            for (k = 0; k < j; k++) {
              tmp1 += W->data[newDimSize] * W->data[iy];
              newDimSize++;
              iy++;
            }
          }

          tmp1 = W->data[ySize_idx_0] - tmp1;
          if (tmp1 > 0.0) {
            tmp1 = std::sqrt(tmp1);
            W->data[ySize_idx_0] = tmp1;
            if (j + 1 < iyLead) {
              ySize_idx_1 = (iyLead - j) - 2;
              aoffset = ySize_idx_0 + iyLead;
              ixLead = (orderForDim + iyLead) + 1;
              if ((j == 0) || (ySize_idx_1 + 1 == 0)) {
              } else {
                iy = aoffset;
                i0 = ixLead + n * ySize_idx_1;
                for (ySize_idx_0 = ixLead; n < 0 ? ySize_idx_0 >= i0 :
                     ySize_idx_0 <= i0; ySize_idx_0 += n) {
                  newDimSize = orderForDim;
                  tmp2 = 0.0;
                  k = (ySize_idx_0 + j) - 1;
                  for (nv = ySize_idx_0; nv <= k; nv++) {
                    tmp2 += W->data[nv - 1] * W->data[newDimSize];
                    newDimSize++;
                  }

                  W->data[iy] += -tmp2;
                  iy += n;
                }
              }

              tmp1 = 1.0 / tmp1;
              i0 = (aoffset + iyLead * ySize_idx_1) + 1;
              for (k = aoffset + 1; iyLead < 0 ? k >= i0 : k <= i0; k += iyLead)
              {
                W->data[k - 1] *= tmp1;
              }

              orderForDim = ixLead - 1;
            }

            j++;
          } else {
            W->data[ySize_idx_0] = tmp1;
            dimSize = j;
            exitg2 = true;
          }
        }
      }

      if (dimSize + 1 == 0) {
        ySize_idx_0 = n;
      } else {
        ySize_idx_0 = dimSize;
      }

      for (j = 0; j < ySize_idx_0; j++) {
        i0 = j + 2;
        for (ixLead = i0; ixLead <= ySize_idx_0; ixLead++) {
          W->data[(ixLead + W->size[0] * j) - 1] = 0.0;
        }
      }
    }

    i0 = bkg->size[0];
    bkg->size[0] = weights->size[0];
    emxEnsureCapacity_real_T(bkg, i0);
    aoffset = weights->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      bkg->data[i0] = weights->data[i0] * y->data[i0];
    }

    i0 = b_W->size[0] * b_W->size[1];
    b_W->size[0] = W->size[1];
    b_W->size[1] = W->size[0];
    emxEnsureCapacity_real_T(b_W, i0);
    aoffset = W->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      ySize_idx_0 = W->size[1];
      for (k = 0; k < ySize_idx_0; k++) {
        b_W->data[k + b_W->size[0] * i0] = W->data[i0 + W->size[0] * k];
      }
    }

    mldivide(b_W, bkg);
    mldivide(W, bkg);
    i0 = d->size[0];
    d->size[0] = y->size[0];
    emxEnsureCapacity_real_T(d, i0);
    aoffset = y->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      d->data[i0] = y->data[i0] - bkg->data[i0];
    }

    // make d-, and get w^t with m and s
    //     dn = d(d<0);
    //     m = mean(dn);
    //     s = std(dn);
    ind1 = 1U;
    i0 = dn->size[0];
    dn->size[0] = varargin_1 + 1;
    emxEnsureCapacity_real_T(dn, i0);
    for (i0 = 0; i0 <= varargin_1; i0++) {
      dn->data[i0] = 0.0;
    }

    //  my change >
    for (ixLead = 0; ixLead <= varargin_1; ixLead++) {
      if (d->data[ixLead] < 0.0) {
        dn->data[(int)ind1 - 1] = d->data[ixLead];
        ind1++;
      }
    }

    i0 = b_dn->size[0];
    b_dn->size[0] = (int)ind1;
    emxEnsureCapacity_real_T(b_dn, i0);
    aoffset = (int)ind1;
    for (i0 = 0; i0 < aoffset; i0++) {
      b_dn->data[i0] = dn->data[i0];
    }

    tmp1 = mean(b_dn);
    i0 = b_dn->size[0];
    b_dn->size[0] = (int)ind1;
    emxEnsureCapacity_real_T(b_dn, i0);
    aoffset = (int)ind1;
    for (i0 = 0; i0 < aoffset; i0++) {
      b_dn->data[i0] = dn->data[i0];
    }

    tmp2 = b_std(b_dn);

    //  my change <
    tmp1 = 2.0 * tmp2 - tmp1;
    i0 = d->size[0];
    emxEnsureCapacity_real_T(d, i0);
    aoffset = d->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      d->data[i0] = 2.0 * (d->data[i0] - tmp1) / tmp2;
    }

    b_exp(d);
    i0 = b_dn->size[0];
    b_dn->size[0] = d->size[0];
    emxEnsureCapacity_real_T(b_dn, i0);
    aoffset = d->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      b_dn->data[i0] = 1.0 + d->data[i0];
    }

    rdivide_helper(b_dn, d);

    // check exit condition and backup
    i0 = b_dn->size[0];
    b_dn->size[0] = weights->size[0];
    emxEnsureCapacity_real_T(b_dn, i0);
    aoffset = weights->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      b_dn->data[i0] = weights->data[i0] - d->data[i0];
    }

    if ((b_norm(b_dn) / b_norm(weights) < ratio) || (*iter >= maxIter)) {
      exitg1 = 1;
    } else {
      i0 = weights->size[0];
      weights->size[0] = d->size[0];
      emxEnsureCapacity_real_T(weights, i0);
      aoffset = d->size[0];
      for (i0 = 0; i0 < aoffset; i0++) {
        weights->data[i0] = d->data[i0];
      }

      (*iter)++;
    }
  } while (exitg1 == 0);

  emxFree_real_T(&b_dn);
  emxFree_real_T(&dn);

  //  Set w=1 for ends
  if (includeEndsNb != 0.0) {
    if (1.0 > includeEndsNb) {
      aoffset = 0;
    } else {
      aoffset = (int)includeEndsNb;
    }

    emxInit_int32_T(&r0, 2);
    i0 = r0->size[0] * r0->size[1];
    r0->size[0] = 1;
    r0->size[1] = aoffset;
    emxEnsureCapacity_int32_T(r0, i0);
    for (i0 = 0; i0 < aoffset; i0++) {
      r0->data[i0] = i0;
    }

    aoffset = r0->size[0] * r0->size[1];
    for (i0 = 0; i0 < aoffset; i0++) {
      d->data[r0->data[i0]] = 1.0;
    }

    tmp1 = (double)y->size[0] - includeEndsNb;
    if (tmp1 > y->size[0]) {
      i0 = 0;
      k = 0;
    } else {
      i0 = (int)tmp1 - 1;
      k = y->size[0];
    }

    ySize_idx_1 = r0->size[0] * r0->size[1];
    r0->size[0] = 1;
    aoffset = k - i0;
    r0->size[1] = aoffset;
    emxEnsureCapacity_int32_T(r0, ySize_idx_1);
    for (k = 0; k < aoffset; k++) {
      r0->data[k] = i0 + k;
    }

    aoffset = r0->size[0] * r0->size[1];
    for (i0 = 0; i0 < aoffset; i0++) {
      d->data[r0->data[i0]] = 1.0;
    }

    emxFree_int32_T(&r0);
    i0 = weights->size[0];
    weights->size[0] = d->size[0];
    emxEnsureCapacity_real_T(weights, i0);
    aoffset = d->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      weights->data[i0] = d->data[i0];
    }

    // Cholesky decomposition
    diag(d, W);
    i0 = W->size[0] * W->size[1];
    k = W->size[0] * W->size[1];
    emxEnsureCapacity_real_T(W, k);
    aoffset = i0 - 1;
    for (i0 = 0; i0 <= aoffset; i0++) {
      W->data[i0] += H->data[i0];
    }

    n = W->size[1];
    if (W->size[1] != 0) {
      iyLead = W->size[0];
      dimSize = -1;
      if (W->size[0] != 0) {
        orderForDim = 0;
        j = 0;
        exitg2 = false;
        while ((!exitg2) && (j <= iyLead - 1)) {
          ySize_idx_0 = orderForDim + j;
          tmp1 = 0.0;
          if (j >= 1) {
            newDimSize = orderForDim;
            iy = orderForDim;
            for (k = 0; k < j; k++) {
              tmp1 += W->data[newDimSize] * W->data[iy];
              newDimSize++;
              iy++;
            }
          }

          tmp1 = W->data[ySize_idx_0] - tmp1;
          if (tmp1 > 0.0) {
            tmp1 = std::sqrt(tmp1);
            W->data[ySize_idx_0] = tmp1;
            if (j + 1 < iyLead) {
              ySize_idx_1 = (iyLead - j) - 2;
              aoffset = ySize_idx_0 + iyLead;
              ixLead = (orderForDim + iyLead) + 1;
              if ((j == 0) || (ySize_idx_1 + 1 == 0)) {
              } else {
                iy = aoffset;
                i0 = ixLead + n * ySize_idx_1;
                for (ySize_idx_0 = ixLead; n < 0 ? ySize_idx_0 >= i0 :
                     ySize_idx_0 <= i0; ySize_idx_0 += n) {
                  newDimSize = orderForDim;
                  tmp2 = 0.0;
                  k = (ySize_idx_0 + j) - 1;
                  for (nv = ySize_idx_0; nv <= k; nv++) {
                    tmp2 += W->data[nv - 1] * W->data[newDimSize];
                    newDimSize++;
                  }

                  W->data[iy] += -tmp2;
                  iy += n;
                }
              }

              tmp1 = 1.0 / tmp1;
              i0 = (aoffset + iyLead * ySize_idx_1) + 1;
              for (k = aoffset + 1; iyLead < 0 ? k >= i0 : k <= i0; k += iyLead)
              {
                W->data[k - 1] *= tmp1;
              }

              orderForDim = ixLead - 1;
            }

            j++;
          } else {
            W->data[ySize_idx_0] = tmp1;
            dimSize = j;
            exitg2 = true;
          }
        }
      }

      if (dimSize + 1 == 0) {
        ySize_idx_0 = n;
      } else {
        ySize_idx_0 = dimSize;
      }

      for (j = 0; j < ySize_idx_0; j++) {
        i0 = j + 2;
        for (ixLead = i0; ixLead <= ySize_idx_0; ixLead++) {
          W->data[(ixLead + W->size[0] * j) - 1] = 0.0;
        }
      }
    }

    i0 = bkg->size[0];
    bkg->size[0] = d->size[0];
    emxEnsureCapacity_real_T(bkg, i0);
    aoffset = d->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      bkg->data[i0] = d->data[i0] * y->data[i0];
    }

    i0 = b_W->size[0] * b_W->size[1];
    b_W->size[0] = W->size[1];
    b_W->size[1] = W->size[0];
    emxEnsureCapacity_real_T(b_W, i0);
    aoffset = W->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      ySize_idx_0 = W->size[1];
      for (k = 0; k < ySize_idx_0; k++) {
        b_W->data[k + b_W->size[0] * i0] = W->data[i0 + W->size[0] * k];
      }
    }

    mldivide(b_W, bkg);
    mldivide(W, bkg);
  }

  //  refine Weights
  if (threshold != 0.0) {
    ySize_idx_0 = d->size[0];
    for (ixLead = 0; ixLead < ySize_idx_0; ixLead++) {
      if (d->data[ixLead] > threshold) {
        d->data[ixLead] = 1.0;
      }
    }

    ySize_idx_0 = d->size[0];
    for (ixLead = 0; ixLead < ySize_idx_0; ixLead++) {
      if (d->data[ixLead] < threshold) {
        d->data[ixLead] = 0.0;
      }
    }

    i0 = weights->size[0];
    weights->size[0] = d->size[0];
    emxEnsureCapacity_real_T(weights, i0);
    aoffset = d->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      weights->data[i0] = d->data[i0];
    }

    // Cholesky decomposition
    diag(d, W);
    i0 = W->size[0] * W->size[1];
    k = W->size[0] * W->size[1];
    emxEnsureCapacity_real_T(W, k);
    aoffset = i0 - 1;
    for (i0 = 0; i0 <= aoffset; i0++) {
      W->data[i0] += H->data[i0];
    }

    n = W->size[1];
    if (W->size[1] != 0) {
      iyLead = W->size[0];
      dimSize = -1;
      if (W->size[0] != 0) {
        orderForDim = 0;
        j = 0;
        exitg2 = false;
        while ((!exitg2) && (j <= iyLead - 1)) {
          ySize_idx_0 = orderForDim + j;
          tmp1 = 0.0;
          if (j >= 1) {
            newDimSize = orderForDim;
            iy = orderForDim;
            for (k = 0; k < j; k++) {
              tmp1 += W->data[newDimSize] * W->data[iy];
              newDimSize++;
              iy++;
            }
          }

          tmp1 = W->data[ySize_idx_0] - tmp1;
          if (tmp1 > 0.0) {
            tmp1 = std::sqrt(tmp1);
            W->data[ySize_idx_0] = tmp1;
            if (j + 1 < iyLead) {
              ySize_idx_1 = (iyLead - j) - 2;
              aoffset = ySize_idx_0 + iyLead;
              ixLead = (orderForDim + iyLead) + 1;
              if ((j == 0) || (ySize_idx_1 + 1 == 0)) {
              } else {
                iy = aoffset;
                i0 = ixLead + n * ySize_idx_1;
                for (ySize_idx_0 = ixLead; n < 0 ? ySize_idx_0 >= i0 :
                     ySize_idx_0 <= i0; ySize_idx_0 += n) {
                  newDimSize = orderForDim;
                  tmp2 = 0.0;
                  k = (ySize_idx_0 + j) - 1;
                  for (nv = ySize_idx_0; nv <= k; nv++) {
                    tmp2 += W->data[nv - 1] * W->data[newDimSize];
                    newDimSize++;
                  }

                  W->data[iy] += -tmp2;
                  iy += n;
                }
              }

              tmp1 = 1.0 / tmp1;
              i0 = (aoffset + iyLead * ySize_idx_1) + 1;
              for (k = aoffset + 1; iyLead < 0 ? k >= i0 : k <= i0; k += iyLead)
              {
                W->data[k - 1] *= tmp1;
              }

              orderForDim = ixLead - 1;
            }

            j++;
          } else {
            W->data[ySize_idx_0] = tmp1;
            dimSize = j;
            exitg2 = true;
          }
        }
      }

      if (dimSize + 1 == 0) {
        ySize_idx_0 = n;
      } else {
        ySize_idx_0 = dimSize;
      }

      for (j = 0; j < ySize_idx_0; j++) {
        i0 = j + 2;
        for (ixLead = i0; ixLead <= ySize_idx_0; ixLead++) {
          W->data[(ixLead + W->size[0] * j) - 1] = 0.0;
        }
      }
    }

    i0 = bkg->size[0];
    bkg->size[0] = d->size[0];
    emxEnsureCapacity_real_T(bkg, i0);
    aoffset = d->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      bkg->data[i0] = d->data[i0] * y->data[i0];
    }

    i0 = b_W->size[0] * b_W->size[1];
    b_W->size[0] = W->size[1];
    b_W->size[1] = W->size[0];
    emxEnsureCapacity_real_T(b_W, i0);
    aoffset = W->size[0];
    for (i0 = 0; i0 < aoffset; i0++) {
      ySize_idx_0 = W->size[1];
      for (k = 0; k < ySize_idx_0; k++) {
        b_W->data[k + b_W->size[0] * i0] = W->data[i0 + W->size[0] * k];
      }
    }

    mldivide(b_W, bkg);
    mldivide(W, bkg);
  }

  emxFree_real_T(&b_W);
  emxFree_real_T(&d);
  emxFree_real_T(&W);
  emxFree_real_T(&H);
}

//
// File trailer for arPLS2Ver2.cpp
//
// [EOF]
//
