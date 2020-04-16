//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: arPLS2.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 16-Apr-2020 13:35:30
//

// Include Files
#include <cmath>
#include "rt_nonfinite.h"
#include "arPLS2.h"
#include "arPLS2_emxutil.h"
#include "mldivide.h"
#include "sparse1.h"
#include "spdiags.h"
#include "norm.h"
#include "std.h"
#include "diff.h"
#include "diag.h"

// Function Definitions

//
// Estimate basline with arPLS
// Arguments    : const emxArray_real_T *y
//                double lambda
//                double ratio
//                int maxIter
//                int includeEndsNb
//                double threshold
//                emxArray_real_T *bkg
//                emxArray_real_T *weights
// Return Type  : void
//
void arPLS2(const emxArray_real_T *y, double lambda, double ratio, int maxIter,
            int includeEndsNb, double threshold, emxArray_real_T *bkg,
            emxArray_real_T *weights)
{
  emxArray_real_T *W_d;
  int varargin_1;
  int i0;
  int coffset;
  emxArray_real_T *D;
  emxArray_real_T *H;
  emxArray_real_T *b_y;
  int boffset;
  int m;
  int k;
  int inner;
  int n;
  int j;
  int b_n;
  int aoffset;
  double temp;
  int iter;
  emxArray_int32_T *W_colidx;
  emxArray_int32_T *W_rowidx;
  emxArray_real_T *C;
  emxArray_real_T *dn;
  emxArray_int32_T *r0;
  emxArray_real_T *b_C;
  int exitg1;
  int info;
  int colj;
  boolean_T exitg2;
  int ix;
  int iy;
  double c;
  emxArray_int32_T *r1;
  emxInit_real_T(&W_d, 1);

  //  Sung-June Baek, Aaron Park, Young-Jin Ahna and Jaebum Choo, Analyst, 2015, 140, 250 
  varargin_1 = y->size[0];

  //  D = diff(speye(N),2);
  i0 = W_d->size[0];
  W_d->size[0] = y->size[0];
  emxEnsureCapacity_real_T(W_d, i0);
  coffset = y->size[0];
  for (i0 = 0; i0 < coffset; i0++) {
    W_d->data[i0] = 1.0;
  }

  emxInit_real_T(&D, 2);
  emxInit_real_T(&H, 2);
  emxInit_real_T(&b_y, 2);
  diag(W_d, H);
  diff(H, D);
  i0 = b_y->size[0] * b_y->size[1];
  b_y->size[0] = D->size[1];
  b_y->size[1] = D->size[0];
  emxEnsureCapacity_real_T(b_y, i0);
  coffset = D->size[0];
  for (i0 = 0; i0 < coffset; i0++) {
    boffset = D->size[1];
    for (k = 0; k < boffset; k++) {
      b_y->data[k + b_y->size[0] * i0] = lambda * D->data[i0 + D->size[0] * k];
    }
  }

  if ((b_y->size[1] == 1) || (D->size[0] == 1)) {
    i0 = H->size[0] * H->size[1];
    H->size[0] = b_y->size[0];
    H->size[1] = D->size[1];
    emxEnsureCapacity_real_T(H, i0);
    coffset = b_y->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      boffset = D->size[1];
      for (k = 0; k < boffset; k++) {
        H->data[i0 + H->size[0] * k] = 0.0;
        m = b_y->size[1];
        for (inner = 0; inner < m; inner++) {
          H->data[i0 + H->size[0] * k] += b_y->data[i0 + b_y->size[0] * inner] *
            D->data[inner + D->size[0] * k];
        }
      }
    }
  } else {
    m = b_y->size[0];
    inner = b_y->size[1];
    n = D->size[1];
    i0 = H->size[0] * H->size[1];
    H->size[0] = b_y->size[0];
    H->size[1] = D->size[1];
    emxEnsureCapacity_real_T(H, i0);
    for (j = 0; j < n; j++) {
      coffset = j * m;
      boffset = j * inner;
      for (b_n = 0; b_n < m; b_n++) {
        H->data[coffset + b_n] = 0.0;
      }

      for (k = 0; k < inner; k++) {
        aoffset = k * m;
        temp = D->data[boffset + k];
        for (b_n = 0; b_n < m; b_n++) {
          i0 = coffset + b_n;
          H->data[i0] += temp * b_y->data[aoffset + b_n];
        }
      }
    }
  }

  emxFree_real_T(&b_y);
  emxFree_real_T(&D);
  i0 = weights->size[0];
  weights->size[0] = y->size[0];
  emxEnsureCapacity_real_T(weights, i0);
  coffset = y->size[0];
  for (i0 = 0; i0 < coffset; i0++) {
    weights->data[i0] = 1.0;
  }

  iter = 0;
  emxInit_int32_T(&W_colidx, 1);
  emxInit_int32_T(&W_rowidx, 1);
  emxInit_real_T(&C, 2);
  emxInit_real_T(&dn, 1);
  emxInit_int32_T(&r0, 1);
  emxInit_real_T(&b_C, 2);
  do {
    exitg1 = 0;
    spdiags(weights, (double)varargin_1, (double)varargin_1, W_d, W_colidx,
            W_rowidx);

    // Cholesky decomposition
    sparse_plus(W_d, W_colidx, W_rowidx, H, C);
    n = C->size[1];
    if (C->size[1] != 0) {
      b_n = C->size[0];
      info = -1;
      if (C->size[0] != 0) {
        colj = 0;
        j = 0;
        exitg2 = false;
        while ((!exitg2) && (j <= b_n - 1)) {
          m = colj + j;
          temp = 0.0;
          if (j >= 1) {
            ix = colj;
            iy = colj;
            for (k = 0; k < j; k++) {
              temp += C->data[ix] * C->data[iy];
              ix++;
              iy++;
            }
          }

          temp = C->data[m] - temp;
          if (temp > 0.0) {
            temp = std::sqrt(temp);
            C->data[m] = temp;
            if (j + 1 < b_n) {
              coffset = (b_n - j) - 2;
              boffset = m + b_n;
              aoffset = (colj + b_n) + 1;
              if ((j == 0) || (coffset + 1 == 0)) {
              } else {
                iy = boffset;
                i0 = aoffset + n * coffset;
                for (m = aoffset; n < 0 ? m >= i0 : m <= i0; m += n) {
                  ix = colj + 1;
                  c = 0.0;
                  k = (m + j) - 1;
                  for (inner = m; inner <= k; inner++) {
                    c += C->data[inner - 1] * C->data[ix - 1];
                    ix++;
                  }

                  C->data[iy] += -c;
                  iy += n;
                }
              }

              temp = 1.0 / temp;
              i0 = (boffset + b_n * coffset) + 1;
              for (k = boffset + 1; b_n < 0 ? k >= i0 : k <= i0; k += b_n) {
                C->data[k - 1] *= temp;
              }

              colj = aoffset - 1;
            }

            j++;
          } else {
            C->data[m] = temp;
            info = j;
            exitg2 = true;
          }
        }
      }

      if (info + 1 == 0) {
        m = n;
      } else {
        m = info;
      }

      for (j = 0; j < m; j++) {
        i0 = j + 2;
        for (b_n = i0; b_n <= m; b_n++) {
          C->data[(b_n + C->size[0] * j) - 1] = 0.0;
        }
      }
    }

    i0 = bkg->size[0];
    bkg->size[0] = weights->size[0];
    emxEnsureCapacity_real_T(bkg, i0);
    coffset = weights->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      bkg->data[i0] = weights->data[i0] * y->data[i0];
    }

    i0 = b_C->size[0] * b_C->size[1];
    b_C->size[0] = C->size[1];
    b_C->size[1] = C->size[0];
    emxEnsureCapacity_real_T(b_C, i0);
    coffset = C->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      boffset = C->size[1];
      for (k = 0; k < boffset; k++) {
        b_C->data[k + b_C->size[0] * i0] = C->data[i0 + C->size[0] * k];
      }
    }

    mldivide(b_C, bkg);
    mldivide(C, bkg);

    // make d-, and get w^t with m and s
    inner = y->size[0] - 1;
    m = 0;
    for (b_n = 0; b_n <= inner; b_n++) {
      if (y->data[b_n] - bkg->data[b_n] < 0.0) {
        m++;
      }
    }

    i0 = r0->size[0];
    r0->size[0] = m;
    emxEnsureCapacity_int32_T(r0, i0);
    m = 0;
    for (b_n = 0; b_n <= inner; b_n++) {
      if (y->data[b_n] - bkg->data[b_n] < 0.0) {
        r0->data[m] = b_n + 1;
        m++;
      }
    }

    i0 = W_d->size[0];
    W_d->size[0] = y->size[0];
    emxEnsureCapacity_real_T(W_d, i0);
    coffset = y->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      W_d->data[i0] = y->data[i0] - bkg->data[i0];
    }

    i0 = dn->size[0];
    dn->size[0] = r0->size[0];
    emxEnsureCapacity_real_T(dn, i0);
    coffset = r0->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      dn->data[i0] = W_d->data[r0->data[i0] - 1];
    }

    m = dn->size[0];
    if (dn->size[0] == 0) {
      temp = 0.0;
    } else {
      temp = dn->data[0];
      for (k = 2; k <= m; k++) {
        temp += dn->data[k - 1];
      }
    }

    temp /= (double)dn->size[0];
    c = b_std(dn);
    temp = 2.0 * c - temp;
    i0 = dn->size[0];
    dn->size[0] = y->size[0];
    emxEnsureCapacity_real_T(dn, i0);
    coffset = y->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      dn->data[i0] = 2.0 * ((y->data[i0] - bkg->data[i0]) - temp) / c;
    }

    m = dn->size[0];
    for (k = 0; k < m; k++) {
      dn->data[k] = std::exp(dn->data[k]);
    }

    i0 = dn->size[0];
    emxEnsureCapacity_real_T(dn, i0);
    coffset = dn->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      dn->data[i0] = 1.0 / (1.0 + dn->data[i0]);
    }

    // check exit condition and backup
    i0 = W_d->size[0];
    W_d->size[0] = weights->size[0];
    emxEnsureCapacity_real_T(W_d, i0);
    coffset = weights->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      W_d->data[i0] = weights->data[i0] - dn->data[i0];
    }

    if ((b_norm(W_d) / b_norm(weights) < ratio) || (iter >= maxIter)) {
      exitg1 = 1;
    } else {
      i0 = weights->size[0];
      weights->size[0] = dn->size[0];
      emxEnsureCapacity_real_T(weights, i0);
      coffset = dn->size[0];
      for (i0 = 0; i0 < coffset; i0++) {
        weights->data[i0] = dn->data[i0];
      }

      iter++;
    }
  } while (exitg1 == 0);

  emxFree_int32_T(&r0);

  //  Set w=1 for ends
  if (includeEndsNb != 0) {
    if (1 > includeEndsNb) {
      coffset = 0;
    } else {
      coffset = includeEndsNb;
    }

    emxInit_int32_T(&r1, 2);
    i0 = r1->size[0] * r1->size[1];
    r1->size[0] = 1;
    r1->size[1] = coffset;
    emxEnsureCapacity_int32_T(r1, i0);
    for (i0 = 0; i0 < coffset; i0++) {
      r1->data[i0] = i0;
    }

    coffset = r1->size[0] * r1->size[1];
    for (i0 = 0; i0 < coffset; i0++) {
      dn->data[r1->data[i0]] = 1.0;
    }

    temp = (double)dn->size[0] - (double)includeEndsNb;
    if (temp < 2.147483648E+9) {
      if (temp >= -2.147483648E+9) {
        i0 = (int)temp;
      } else {
        i0 = MIN_int32_T;
      }
    } else {
      i0 = MAX_int32_T;
    }

    if (i0 > dn->size[0]) {
      i0 = 0;
      k = 0;
    } else {
      i0--;
      k = dn->size[0];
    }

    inner = r1->size[0] * r1->size[1];
    r1->size[0] = 1;
    coffset = k - i0;
    r1->size[1] = coffset;
    emxEnsureCapacity_int32_T(r1, inner);
    for (k = 0; k < coffset; k++) {
      r1->data[k] = i0 + k;
    }

    coffset = r1->size[0] * r1->size[1];
    for (i0 = 0; i0 < coffset; i0++) {
      dn->data[r1->data[i0]] = 1.0;
    }

    emxFree_int32_T(&r1);
    i0 = weights->size[0];
    weights->size[0] = dn->size[0];
    emxEnsureCapacity_real_T(weights, i0);
    coffset = dn->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      weights->data[i0] = dn->data[i0];
    }

    spdiags(dn, (double)y->size[0], (double)y->size[0], W_d, W_colidx, W_rowidx);

    // Cholesky decomposition
    sparse_plus(W_d, W_colidx, W_rowidx, H, C);
    n = C->size[1];
    if (C->size[1] != 0) {
      b_n = C->size[0];
      info = -1;
      if (C->size[0] != 0) {
        colj = 0;
        j = 0;
        exitg2 = false;
        while ((!exitg2) && (j <= b_n - 1)) {
          m = colj + j;
          temp = 0.0;
          if (j >= 1) {
            ix = colj;
            iy = colj;
            for (k = 0; k < j; k++) {
              temp += C->data[ix] * C->data[iy];
              ix++;
              iy++;
            }
          }

          temp = C->data[m] - temp;
          if (temp > 0.0) {
            temp = std::sqrt(temp);
            C->data[m] = temp;
            if (j + 1 < b_n) {
              coffset = (b_n - j) - 2;
              boffset = m + b_n;
              aoffset = (colj + b_n) + 1;
              if ((j == 0) || (coffset + 1 == 0)) {
              } else {
                iy = boffset;
                i0 = aoffset + n * coffset;
                for (m = aoffset; n < 0 ? m >= i0 : m <= i0; m += n) {
                  ix = colj + 1;
                  c = 0.0;
                  k = (m + j) - 1;
                  for (inner = m; inner <= k; inner++) {
                    c += C->data[inner - 1] * C->data[ix - 1];
                    ix++;
                  }

                  C->data[iy] += -c;
                  iy += n;
                }
              }

              temp = 1.0 / temp;
              i0 = (boffset + b_n * coffset) + 1;
              for (k = boffset + 1; b_n < 0 ? k >= i0 : k <= i0; k += b_n) {
                C->data[k - 1] *= temp;
              }

              colj = aoffset - 1;
            }

            j++;
          } else {
            C->data[m] = temp;
            info = j;
            exitg2 = true;
          }
        }
      }

      if (info + 1 == 0) {
        m = n;
      } else {
        m = info;
      }

      for (j = 0; j < m; j++) {
        i0 = j + 2;
        for (b_n = i0; b_n <= m; b_n++) {
          C->data[(b_n + C->size[0] * j) - 1] = 0.0;
        }
      }
    }

    i0 = bkg->size[0];
    bkg->size[0] = dn->size[0];
    emxEnsureCapacity_real_T(bkg, i0);
    coffset = dn->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      bkg->data[i0] = dn->data[i0] * y->data[i0];
    }

    i0 = b_C->size[0] * b_C->size[1];
    b_C->size[0] = C->size[1];
    b_C->size[1] = C->size[0];
    emxEnsureCapacity_real_T(b_C, i0);
    coffset = C->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      boffset = C->size[1];
      for (k = 0; k < boffset; k++) {
        b_C->data[k + b_C->size[0] * i0] = C->data[i0 + C->size[0] * k];
      }
    }

    mldivide(b_C, bkg);
    mldivide(C, bkg);
  }

  //  refine Weights
  if (threshold != 0.0) {
    inner = dn->size[0];
    for (b_n = 0; b_n < inner; b_n++) {
      if (dn->data[b_n] > threshold) {
        dn->data[b_n] = 1.0;
      }
    }

    inner = dn->size[0];
    for (b_n = 0; b_n < inner; b_n++) {
      if (dn->data[b_n] < threshold) {
        dn->data[b_n] = 0.0;
      }
    }

    i0 = weights->size[0];
    weights->size[0] = dn->size[0];
    emxEnsureCapacity_real_T(weights, i0);
    coffset = dn->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      weights->data[i0] = dn->data[i0];
    }

    spdiags(dn, (double)y->size[0], (double)y->size[0], W_d, W_colidx, W_rowidx);

    // Cholesky decomposition
    sparse_plus(W_d, W_colidx, W_rowidx, H, C);
    n = C->size[1];
    if (C->size[1] != 0) {
      b_n = C->size[0];
      info = -1;
      if (C->size[0] != 0) {
        colj = 0;
        j = 0;
        exitg2 = false;
        while ((!exitg2) && (j <= b_n - 1)) {
          m = colj + j;
          temp = 0.0;
          if (j >= 1) {
            ix = colj;
            iy = colj;
            for (k = 0; k < j; k++) {
              temp += C->data[ix] * C->data[iy];
              ix++;
              iy++;
            }
          }

          temp = C->data[m] - temp;
          if (temp > 0.0) {
            temp = std::sqrt(temp);
            C->data[m] = temp;
            if (j + 1 < b_n) {
              coffset = (b_n - j) - 2;
              boffset = m + b_n;
              aoffset = (colj + b_n) + 1;
              if ((j == 0) || (coffset + 1 == 0)) {
              } else {
                iy = boffset;
                i0 = aoffset + n * coffset;
                for (m = aoffset; n < 0 ? m >= i0 : m <= i0; m += n) {
                  ix = colj + 1;
                  c = 0.0;
                  k = (m + j) - 1;
                  for (inner = m; inner <= k; inner++) {
                    c += C->data[inner - 1] * C->data[ix - 1];
                    ix++;
                  }

                  C->data[iy] += -c;
                  iy += n;
                }
              }

              temp = 1.0 / temp;
              i0 = (boffset + b_n * coffset) + 1;
              for (k = boffset + 1; b_n < 0 ? k >= i0 : k <= i0; k += b_n) {
                C->data[k - 1] *= temp;
              }

              colj = aoffset - 1;
            }

            j++;
          } else {
            C->data[m] = temp;
            info = j;
            exitg2 = true;
          }
        }
      }

      if (info + 1 == 0) {
        m = n;
      } else {
        m = info;
      }

      for (j = 0; j < m; j++) {
        i0 = j + 2;
        for (b_n = i0; b_n <= m; b_n++) {
          C->data[(b_n + C->size[0] * j) - 1] = 0.0;
        }
      }
    }

    i0 = bkg->size[0];
    bkg->size[0] = dn->size[0];
    emxEnsureCapacity_real_T(bkg, i0);
    coffset = dn->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      bkg->data[i0] = dn->data[i0] * y->data[i0];
    }

    i0 = b_C->size[0] * b_C->size[1];
    b_C->size[0] = C->size[1];
    b_C->size[1] = C->size[0];
    emxEnsureCapacity_real_T(b_C, i0);
    coffset = C->size[0];
    for (i0 = 0; i0 < coffset; i0++) {
      boffset = C->size[1];
      for (k = 0; k < boffset; k++) {
        b_C->data[k + b_C->size[0] * i0] = C->data[i0 + C->size[0] * k];
      }
    }

    mldivide(b_C, bkg);
    mldivide(C, bkg);
  }

  emxFree_real_T(&b_C);
  emxFree_real_T(&dn);
  emxFree_real_T(&C);
  emxFree_int32_T(&W_rowidx);
  emxFree_int32_T(&W_colidx);
  emxFree_real_T(&W_d);
  emxFree_real_T(&H);
}

//
// File trailer for arPLS2.cpp
//
// [EOF]
//
