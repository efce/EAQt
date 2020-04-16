//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: xgeqp3.cpp
//
// MATLAB Coder version            : 4.1
// C/C++ source code generated on  : 16-Apr-2020 13:35:30
//

// Include Files
#include <cmath>
#include "rt_nonfinite.h"
#include "arPLS2.h"
#include "xgeqp3.h"
#include "arPLS2_emxutil.h"
#include "xnrm2.h"
#include "xscal.h"
#include "colon.h"

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
  int itemp;
  int mn;
  int i7;
  emxArray_real_T *work;
  emxArray_real_T *vn1;
  emxArray_real_T *vn2;
  int k;
  int nmi;
  int i;
  int i_i;
  int mmi;
  int ix;
  int pvt;
  double smax;
  int iy;
  double atmp;
  double s;
  int jA;
  int lastv;
  int lastc;
  boolean_T exitg2;
  int exitg1;
  m = A->size[0];
  n = A->size[1];
  itemp = A->size[0];
  mn = A->size[1];
  if (itemp < mn) {
    mn = itemp;
  }

  i7 = tau->size[0];
  tau->size[0] = mn;
  emxEnsureCapacity_real_T(tau, i7);
  eml_signed_integer_colon(A->size[1], jpvt);
  if ((A->size[0] != 0) && (A->size[1] != 0)) {
    emxInit_real_T(&work, 1);
    itemp = A->size[1];
    i7 = work->size[0];
    work->size[0] = itemp;
    emxEnsureCapacity_real_T(work, i7);
    for (i7 = 0; i7 < itemp; i7++) {
      work->data[i7] = 0.0;
    }

    emxInit_real_T(&vn1, 1);
    emxInit_real_T(&vn2, 1);
    itemp = A->size[1];
    i7 = vn1->size[0];
    vn1->size[0] = itemp;
    emxEnsureCapacity_real_T(vn1, i7);
    i7 = vn2->size[0];
    vn2->size[0] = vn1->size[0];
    emxEnsureCapacity_real_T(vn2, i7);
    k = 1;
    for (nmi = 0; nmi < n; nmi++) {
      vn1->data[nmi] = xnrm2(m, A, k);
      vn2->data[nmi] = vn1->data[nmi];
      k += m;
    }

    for (i = 0; i < mn; i++) {
      i_i = i + i * m;
      nmi = n - i;
      mmi = (m - i) - 1;
      if (nmi < 1) {
        itemp = 0;
      } else {
        itemp = 1;
        if (nmi > 1) {
          ix = i;
          smax = std::abs(vn1->data[i]);
          for (k = 2; k <= nmi; k++) {
            ix++;
            s = std::abs(vn1->data[ix]);
            if (s > smax) {
              itemp = k;
              smax = s;
            }
          }
        }
      }

      pvt = (i + itemp) - 1;
      if (pvt + 1 != i + 1) {
        ix = m * pvt;
        iy = m * i;
        for (k = 0; k < m; k++) {
          smax = A->data[ix];
          A->data[ix] = A->data[iy];
          A->data[iy] = smax;
          ix++;
          iy++;
        }

        itemp = jpvt->data[pvt];
        jpvt->data[pvt] = jpvt->data[i];
        jpvt->data[i] = itemp;
        vn1->data[pvt] = vn1->data[i];
        vn2->data[pvt] = vn2->data[i];
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
              itemp = -1;
              do {
                itemp++;
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
              for (k = 0; k <= itemp; k++) {
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
          itemp = i_i + mmi;
          while ((lastv + 1 > 0) && (A->data[itemp] == 0.0)) {
            lastv--;
            itemp--;
          }

          lastc = nmi - 2;
          exitg2 = false;
          while ((!exitg2) && (lastc + 1 > 0)) {
            itemp = jA + lastc * m;
            k = itemp;
            do {
              exitg1 = 0;
              if (k <= itemp + lastv) {
                if (A->data[k - 1] != 0.0) {
                  exitg1 = 1;
                } else {
                  k++;
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
            i7 = jA + m * lastc;
            for (itemp = jA; m < 0 ? itemp >= i7 : itemp <= i7; itemp += m) {
              ix = i_i;
              smax = 0.0;
              pvt = itemp + lastv;
              for (k = itemp; k <= pvt; k++) {
                smax += A->data[k - 1] * A->data[ix];
                ix++;
              }

              work->data[iy] += smax;
              iy++;
            }
          }

          if (!(-tau->data[i] == 0.0)) {
            itemp = 0;
            for (nmi = 0; nmi <= lastc; nmi++) {
              if (work->data[itemp] != 0.0) {
                smax = work->data[itemp] * -tau->data[i];
                ix = i_i;
                i7 = lastv + jA;
                for (pvt = jA; pvt <= i7; pvt++) {
                  A->data[pvt - 1] += A->data[ix] * smax;
                  ix++;
                }
              }

              itemp++;
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
