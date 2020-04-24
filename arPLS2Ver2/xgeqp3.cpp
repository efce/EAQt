//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: xgeqp3.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 21-Apr-2020 13:14:26
//

// Include Files
#include <cmath>
#include "rt_nonfinite.h"
#include "arPLS2Ver2.h"
#include "xgeqp3.h"
#include "arPLS2Ver2_emxutil.h"
#include "xnrm2.h"
#include "xscal.h"

// Function Declarations
static double rt_hypotd_snf(double u0, double u1);

// Function Definitions

//
// Arguments    : double u0
//                double u1
// Return Type  : double
//
static double rt_hypotd_snf(double u0, double u1)
{
  double y;
  double a;
  double b;
  a = std::abs(u0);
  b = std::abs(u1);
  if (a < b) {
    a /= b;
    y = b * std::sqrt(a * a + 1.0);
  } else if (a > b) {
    b /= a;
    y = a * std::sqrt(b * b + 1.0);
  } else if (rtIsNaN(b)) {
    y = b;
  } else {
    y = a * 1.4142135623730951;
  }

  return y;
}

//
// Arguments    : emxArray_real_T *A
//                emxArray_real_T *tau
//                emxArray_int32_T *jpvt
// Return Type  : void
//
void xgeqp3(emxArray_real_T *A, emxArray_real_T *tau, emxArray_int32_T *jpvt)
{
  int m;
  int n;
  int yk;
  int mn;
  int i4;
  int b_n;
  emxArray_real_T *work;
  int lastc;
  emxArray_real_T *vn1;
  emxArray_real_T *vn2;
  int nmi;
  int i;
  int i_i;
  int mmi;
  int ix;
  double smax;
  int iy;
  double atmp;
  double s;
  int jA;
  int lastv;
  boolean_T exitg2;
  int exitg1;
  m = A->size[0];
  n = A->size[1];
  yk = A->size[0];
  mn = A->size[1];
  if (yk < mn) {
    mn = yk;
  }

  i4 = tau->size[0];
  tau->size[0] = mn;
  emxEnsureCapacity_real_T(tau, i4);
  if (A->size[1] < 1) {
    b_n = 0;
  } else {
    b_n = A->size[1];
  }

  i4 = jpvt->size[0] * jpvt->size[1];
  jpvt->size[0] = 1;
  jpvt->size[1] = b_n;
  emxEnsureCapacity_int32_T(jpvt, i4);
  if (b_n > 0) {
    jpvt->data[0] = 1;
    yk = 1;
    for (lastc = 2; lastc <= b_n; lastc++) {
      yk++;
      jpvt->data[lastc - 1] = yk;
    }
  }

  if ((A->size[0] != 0) && (A->size[1] != 0)) {
    emxInit_real_T(&work, 1);
    yk = A->size[1];
    i4 = work->size[0];
    work->size[0] = yk;
    emxEnsureCapacity_real_T(work, i4);
    for (i4 = 0; i4 < yk; i4++) {
      work->data[i4] = 0.0;
    }

    emxInit_real_T(&vn1, 1);
    emxInit_real_T(&vn2, 1);
    yk = A->size[1];
    i4 = vn1->size[0];
    vn1->size[0] = yk;
    emxEnsureCapacity_real_T(vn1, i4);
    i4 = vn2->size[0];
    vn2->size[0] = vn1->size[0];
    emxEnsureCapacity_real_T(vn2, i4);
    lastc = 1;
    for (nmi = 0; nmi < n; nmi++) {
      vn1->data[nmi] = xnrm2(m, A, lastc);
      vn2->data[nmi] = vn1->data[nmi];
      lastc += m;
    }

    for (i = 0; i < mn; i++) {
      i_i = i + i * m;
      nmi = n - i;
      mmi = (m - i) - 1;
      if (nmi < 1) {
        yk = 0;
      } else {
        yk = 1;
        if (nmi > 1) {
          ix = i;
          smax = std::abs(vn1->data[i]);
          for (lastc = 2; lastc <= nmi; lastc++) {
            ix++;
            s = std::abs(vn1->data[ix]);
            if (s > smax) {
              yk = lastc;
              smax = s;
            }
          }
        }
      }

      b_n = (i + yk) - 1;
      if (b_n + 1 != i + 1) {
        ix = m * b_n;
        iy = m * i;
        for (lastc = 0; lastc < m; lastc++) {
          smax = A->data[ix];
          A->data[ix] = A->data[iy];
          A->data[iy] = smax;
          ix++;
          iy++;
        }

        yk = jpvt->data[b_n];
        jpvt->data[b_n] = jpvt->data[i];
        jpvt->data[i] = yk;
        vn1->data[b_n] = vn1->data[i];
        vn2->data[b_n] = vn2->data[i];
      }

      if (i + 1 < m) {
        atmp = A->data[i_i];
        tau->data[i] = 0.0;
        if (1 + mmi > 0) {
          smax = xnrm2(mmi, A, i_i + 2);
          if (smax != 0.0) {
            s = rt_hypotd_snf(A->data[i_i], smax);
            if (A->data[i_i] >= 0.0) {
              s = -s;
            }

            if (std::abs(s) < 1.0020841800044864E-292) {
              yk = -1;
              do {
                yk++;
                xscal(mmi, 9.9792015476736E+291, A, i_i + 2);
                s *= 9.9792015476736E+291;
                atmp *= 9.9792015476736E+291;
              } while (!(std::abs(s) >= 1.0020841800044864E-292));

              s = rt_hypotd_snf(atmp, xnrm2(mmi, A, i_i + 2));
              if (atmp >= 0.0) {
                s = -s;
              }

              tau->data[i] = (s - atmp) / s;
              xscal(mmi, 1.0 / (atmp - s), A, i_i + 2);
              for (lastc = 0; lastc <= yk; lastc++) {
                s *= 1.0020841800044864E-292;
              }

              atmp = s;
            } else {
              tau->data[i] = (s - A->data[i_i]) / s;
              smax = 1.0 / (A->data[i_i] - s);
              xscal(mmi, smax, A, i_i + 2);
              atmp = s;
            }
          }
        }

        A->data[i_i] = atmp;
      } else {
        tau->data[i] = 0.0;
      }

      if (i + 1 < n) {
        atmp = A->data[i_i];
        A->data[i_i] = 1.0;
        jA = (i + (i + 1) * m) + 1;
        if (tau->data[i] != 0.0) {
          lastv = mmi;
          yk = i_i + mmi;
          while ((lastv + 1 > 0) && (A->data[yk] == 0.0)) {
            lastv--;
            yk--;
          }

          lastc = nmi - 2;
          exitg2 = false;
          while ((!exitg2) && (lastc + 1 > 0)) {
            yk = jA + lastc * m;
            nmi = yk;
            do {
              exitg1 = 0;
              if (nmi <= yk + lastv) {
                if (A->data[nmi - 1] != 0.0) {
                  exitg1 = 1;
                } else {
                  nmi++;
                }
              } else {
                lastc--;
                exitg1 = 2;
              }
            } while (exitg1 == 0);

            if (exitg1 == 1) {
              exitg2 = true;
            }
          }
        } else {
          lastv = -1;
          lastc = -1;
        }

        if (lastv + 1 > 0) {
          if (lastc + 1 != 0) {
            for (iy = 0; iy <= lastc; iy++) {
              work->data[iy] = 0.0;
            }

            iy = 0;
            i4 = jA + m * lastc;
            for (yk = jA; m < 0 ? yk >= i4 : yk <= i4; yk += m) {
              ix = i_i;
              smax = 0.0;
              b_n = yk + lastv;
              for (nmi = yk; nmi <= b_n; nmi++) {
                smax += A->data[nmi - 1] * A->data[ix];
                ix++;
              }

              work->data[iy] += smax;
              iy++;
            }
          }

          if (!(-tau->data[i] == 0.0)) {
            yk = 0;
            for (nmi = 0; nmi <= lastc; nmi++) {
              if (work->data[yk] != 0.0) {
                smax = work->data[yk] * -tau->data[i];
                ix = i_i;
                i4 = lastv + jA;
                for (b_n = jA; b_n <= i4; b_n++) {
                  A->data[b_n - 1] += A->data[ix] * smax;
                  ix++;
                }
              }

              yk++;
              jA += m;
            }
          }
        }

        A->data[i_i] = atmp;
      }

      for (nmi = i + 2; nmi <= n; nmi++) {
        if (vn1->data[nmi - 1] != 0.0) {
          smax = std::abs(A->data[i + A->size[0] * (nmi - 1)]) / vn1->data[nmi -
            1];
          smax = 1.0 - smax * smax;
          if (smax < 0.0) {
            smax = 0.0;
          }

          s = vn1->data[nmi - 1] / vn2->data[nmi - 1];
          s = smax * (s * s);
          if (s <= 1.4901161193847656E-8) {
            if (i + 1 < m) {
              vn1->data[nmi - 1] = xnrm2(mmi, A, (i + m * (nmi - 1)) + 2);
              vn2->data[nmi - 1] = vn1->data[nmi - 1];
            } else {
              vn1->data[nmi - 1] = 0.0;
              vn2->data[nmi - 1] = 0.0;
            }
          } else {
            vn1->data[nmi - 1] *= std::sqrt(smax);
          }
        }
      }
    }

    emxFree_real_T(&vn2);
    emxFree_real_T(&vn1);
    emxFree_real_T(&work);
  }
}

//
// File trailer for xgeqp3.cpp
//
// [EOF]
//
