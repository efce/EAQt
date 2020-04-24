//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: mldivide.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//

// Include Files
#include <cmath>
#include "rt_nonfinite.h"
#include "arPLS2Ver2.h"
#include "mldivide.h"
#include "arPLS2Ver2_emxutil.h"
#include "xgeqp3.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *A
//                emxArray_real_T *B
// Return Type  : void
//
void mldivide(const emxArray_real_T *A, emxArray_real_T *B)
{
  emxArray_real_T *b_A;
  emxArray_real_T *tau;
  emxArray_int32_T *jpvt;
  emxArray_real_T *b_B;
  unsigned int unnamed_idx_0;
  int i3;
  int n;
  int minmn;
  int rankR;
  int mn;
  int maxmn;
  double tol;
  int ldap1;
  int u1;
  int j;
  int mmj_tmp;
  int jj;
  int i;
  int ix;
  double s;
  emxInit_real_T(&b_A, 2);
  emxInit_real_T(&tau, 1);
  emxInit_int32_T(&jpvt, 2);
  emxInit_real_T(&b_B, 1);
  if ((A->size[0] == 0) || (A->size[1] == 0) || (B->size[0] == 0)) {
    unnamed_idx_0 = (unsigned int)A->size[1];
    i3 = B->size[0];
    B->size[0] = (int)unnamed_idx_0;
    emxEnsureCapacity_real_T(B, i3);
    minmn = (int)unnamed_idx_0;
    for (i3 = 0; i3 < minmn; i3++) {
      B->data[i3] = 0.0;
    }
  } else if (A->size[0] == A->size[1]) {
    n = A->size[1];
    i3 = b_A->size[0] * b_A->size[1];
    b_A->size[0] = A->size[0];
    b_A->size[1] = A->size[1];
    emxEnsureCapacity_real_T(b_A, i3);
    minmn = A->size[0] * A->size[1];
    for (i3 = 0; i3 < minmn; i3++) {
      b_A->data[i3] = A->data[i3];
    }

    mn = A->size[1];
    i3 = jpvt->size[0] * jpvt->size[1];
    jpvt->size[0] = 1;
    jpvt->size[1] = mn;
    emxEnsureCapacity_int32_T(jpvt, i3);
    jpvt->data[0] = 1;
    minmn = 1;
    for (rankR = 2; rankR <= mn; rankR++) {
      minmn++;
      jpvt->data[rankR - 1] = minmn;
    }

    ldap1 = A->size[1] + 1;
    maxmn = A->size[1] - 1;
    u1 = A->size[1];
    if (maxmn < u1) {
      u1 = maxmn;
    }

    for (j = 0; j < u1; j++) {
      mmj_tmp = n - j;
      mn = j * (n + 1);
      jj = j * ldap1;
      maxmn = mn + 2;
      if (mmj_tmp < 1) {
        minmn = -1;
      } else {
        minmn = 0;
        if (mmj_tmp > 1) {
          ix = mn;
          tol = std::abs(b_A->data[mn]);
          for (rankR = 2; rankR <= mmj_tmp; rankR++) {
            ix++;
            s = std::abs(b_A->data[ix]);
            if (s > tol) {
              minmn = rankR - 1;
              tol = s;
            }
          }
        }
      }

      if (b_A->data[jj + minmn] != 0.0) {
        if (minmn != 0) {
          minmn += j;
          jpvt->data[j] = minmn + 1;
          ix = j;
          for (rankR = 0; rankR < n; rankR++) {
            tol = b_A->data[ix];
            b_A->data[ix] = b_A->data[minmn];
            b_A->data[minmn] = tol;
            ix += n;
            minmn += n;
          }
        }

        i3 = jj + mmj_tmp;
        for (i = maxmn; i <= i3; i++) {
          b_A->data[i - 1] /= b_A->data[jj];
        }
      }

      minmn = mn + n;
      maxmn = jj + ldap1;
      for (mn = 0; mn <= mmj_tmp - 2; mn++) {
        tol = b_A->data[minmn];
        if (b_A->data[minmn] != 0.0) {
          ix = jj + 1;
          i3 = maxmn + 1;
          rankR = mmj_tmp + maxmn;
          for (i = i3; i < rankR; i++) {
            b_A->data[i - 1] += b_A->data[ix] * -tol;
            ix++;
          }
        }

        minmn += n;
        maxmn += n;
      }
    }

    i3 = A->size[1];
    for (minmn = 0; minmn <= i3 - 2; minmn++) {
      if (jpvt->data[minmn] != minmn + 1) {
        tol = B->data[minmn];
        B->data[minmn] = B->data[jpvt->data[minmn] - 1];
        B->data[jpvt->data[minmn] - 1] = tol;
      }
    }

    for (rankR = 0; rankR < n; rankR++) {
      minmn = n * rankR;
      if (B->data[rankR] != 0.0) {
        for (i = rankR + 2; i <= n; i++) {
          B->data[i - 1] -= B->data[rankR] * b_A->data[(i + minmn) - 1];
        }
      }
    }

    for (rankR = n; rankR >= 1; rankR--) {
      minmn = n * (rankR - 1);
      if (B->data[rankR - 1] != 0.0) {
        B->data[rankR - 1] /= b_A->data[(rankR + minmn) - 1];
        for (i = 0; i <= rankR - 2; i++) {
          B->data[i] -= B->data[rankR - 1] * b_A->data[i + minmn];
        }
      }
    }
  } else {
    i3 = b_A->size[0] * b_A->size[1];
    b_A->size[0] = A->size[0];
    b_A->size[1] = A->size[1];
    emxEnsureCapacity_real_T(b_A, i3);
    minmn = A->size[0] * A->size[1];
    for (i3 = 0; i3 < minmn; i3++) {
      b_A->data[i3] = A->data[i3];
    }

    xgeqp3(b_A, tau, jpvt);
    rankR = 0;
    if (b_A->size[0] < b_A->size[1]) {
      minmn = b_A->size[0];
      maxmn = b_A->size[1];
    } else {
      minmn = b_A->size[1];
      maxmn = b_A->size[0];
    }

    if (minmn > 0) {
      tol = 2.2204460492503131E-15 * (double)maxmn;
      if (1.4901161193847656E-8 < tol) {
        tol = 1.4901161193847656E-8;
      }

      tol *= std::abs(b_A->data[0]);
      while ((rankR < minmn) && (!(std::abs(b_A->data[rankR + b_A->size[0] *
                rankR]) <= tol))) {
        rankR++;
      }
    }

    i3 = b_B->size[0];
    b_B->size[0] = B->size[0];
    emxEnsureCapacity_real_T(b_B, i3);
    minmn = B->size[0];
    for (i3 = 0; i3 < minmn; i3++) {
      b_B->data[i3] = B->data[i3];
    }

    minmn = b_A->size[1];
    i3 = B->size[0];
    B->size[0] = minmn;
    emxEnsureCapacity_real_T(B, i3);
    for (i3 = 0; i3 < minmn; i3++) {
      B->data[i3] = 0.0;
    }

    minmn = b_A->size[0];
    maxmn = b_A->size[0];
    mn = b_A->size[1];
    if (maxmn < mn) {
      mn = maxmn;
    }

    for (j = 0; j < mn; j++) {
      if (tau->data[j] != 0.0) {
        tol = b_B->data[j];
        i3 = j + 2;
        for (i = i3; i <= minmn; i++) {
          tol += b_A->data[(i + b_A->size[0] * j) - 1] * b_B->data[i - 1];
        }

        tol *= tau->data[j];
        if (tol != 0.0) {
          b_B->data[j] -= tol;
          i3 = j + 2;
          for (i = i3; i <= minmn; i++) {
            b_B->data[i - 1] -= b_A->data[(i + b_A->size[0] * j) - 1] * tol;
          }
        }
      }
    }

    for (i = 0; i < rankR; i++) {
      B->data[jpvt->data[i] - 1] = b_B->data[i];
    }

    for (j = rankR; j >= 1; j--) {
      B->data[jpvt->data[j - 1] - 1] /= b_A->data[(j + b_A->size[0] * (j - 1)) -
        1];
      for (i = 0; i <= j - 2; i++) {
        B->data[jpvt->data[i] - 1] -= B->data[jpvt->data[j - 1] - 1] * b_A->
          data[i + b_A->size[0] * (j - 1)];
      }
    }
  }

  emxFree_real_T(&b_B);
  emxFree_int32_T(&jpvt);
  emxFree_real_T(&tau);
  emxFree_real_T(&b_A);
}

//
// File trailer for mldivide.cpp
//
// [EOF]
//
