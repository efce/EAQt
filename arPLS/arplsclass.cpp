#include "arplsclass.h"
#include "Eigen/Dense"
#include <chrono>
#include <iostream>

arPLSClass::arPLSClass()
{

}

template <typename MATTYPE, typename VECTYPE, typename SCALARTYPE>
void arPLSClass::_compute(
        QVector<double>* yV, int32_t lambda, double ratio, int32_t maxIter, int32_t includeEnds,
        double threshold, QVector<double>* rBkg, QVector<double>* rWeights, int* rIter)
{
    int32_t N = yV->size();

    MATTYPE y(N,1);
    for (int i = 0; i < N; i++)
    {
        y(i,0) = yV[0][i];
    }

    // init variables
    MATTYPE D(N-2,N);
    MATTYPE H(N,N);
    MATTYPE weights(N,1);
    MATTYPE W(N,N);
    MATTYPE C(N,N);
    MATTYPE bkg(N,1);
    MATTYPE wy(N,1);
    //MATTYPE Cti(N,N);
    MATTYPE Ci(N,N);
    MATTYPE d(N,1);
    MATTYPE wt(N,1);

    // init operations
    D = _diff<MATTYPE>(_diff<MATTYPE>(MATTYPE::Identity(N,N)));
    H = lambda * (D.transpose() * D);
    weights = MATTYPE::Ones(N,1);

    int iter = 0;
    // main loop
    while(1)
    {

        W = weights.asDiagonal();

        // Cholesky decomposition
        Eigen::LLT<MATTYPE, Eigen::Upper> Chol(W + H);
        C = Chol.matrixU();

        wy = weights.cwiseProduct(y);
        Ci = C.inverse();
        //Cti = Ci.transpose();
        bkg = Ci * (Ci.transpose() * wy);
        d = y - bkg;

        int32_t ind1 = 0;

        //MATTYPE dn = (d.array() < 0).select(d, 0.0);
        MATTYPE dn = MATTYPE::Zero(N,1);
        for(int i = 0; i < N; i++)
        {
            if( d(i,0) < 0 )
            {
                dn(ind1,0) = d(i,0);
                ind1++;
            }
        }
        MATTYPE dns = dn.block(0,0,ind1,1);
        SCALARTYPE m = dns.mean();
        // std
        SCALARTYPE s = _std<MATTYPE, SCALARTYPE>(dns);

        wt = 1 / (1 + ((2*(d - (2*s-m)*MATTYPE::Ones(N,1)))/s).array().exp());

        //if( ((weights-wt).norm()/weights.norm() < ratio) || (iter >= maxIter) )
        if( (abs((weights-wt).sum())/abs(weights.sum()) < ratio) || (iter >= maxIter) )
        {
            // Set w=1 for ends
            if(includeEnds)
            {
                for(int i = 0; i < includeEnds; i++)
                {
                    wt(i,0) = 1;
                    wt(N-1-i,0) = 1;
                }

            }

            // refine Weights
            if(threshold)
            {
                for(int i = 0; i < N; i++)
                {
                    if(wt(i,0) > threshold)
                    {
                        wt(i,0) = 1;
                    }
                    else
                    {
                        wt(i,0) = 0;
                    }
                }
            }

            weights = wt;
            W = weights.asDiagonal();
            // Cholesky decomposition
            Eigen::LLT<MATTYPE, Eigen::Upper> Chol(W + H);
            C = Chol.matrixU();
            wy = weights.cwiseProduct(y);
            Ci = C.inverse();
            //Cti = Ci.transpose();
            bkg = Ci * (Ci.transpose() * wy);

            for(int i = 0; i < N; i++)
            {
                rBkg[0][i] = bkg(i,0);
                rWeights[0][i] = weights(i,0);
                *rIter = iter;
            }

            break;
        }
        else
        {
            weights = wt;
        }

        iter++;
    }

}

void arPLSClass::compute(
        QVector<double>* yV, int32_t lambda, double ratio, int32_t maxIter, int32_t includeEnds,
        double threshold, QVector<double>* rBkg, QVector<double>* rWeights, int* rIter, bool compute_in_float)
{
    if (compute_in_float) {
        this->_compute<Eigen::MatrixXf, Eigen::VectorXf, float>
                (yV, lambda, ratio, maxIter, includeEnds, threshold, rBkg, rWeights, rIter);
    }
    else
    {
        this->_compute<Eigen::MatrixXd, Eigen::VectorXd, double>
                (yV, lambda, ratio, maxIter, includeEnds, threshold, rBkg, rWeights, rIter);
    }

}


template <typename MATTYPE>
MATTYPE arPLSClass::_diff(MATTYPE mat)
{
    auto res = mat.block(1, 0, mat.rows()-1, mat.cols()) - mat.block(0, 0, mat.rows()-1, mat.cols());
    return res;
}

template<typename MATTYPE, typename SCALARTYPE>
SCALARTYPE arPLSClass::_std(MATTYPE mat)
{
    SCALARTYPE M1 = mat.innerSize() - 1.;
    SCALARTYPE std_dev = sqrt((mat.array() - mat.mean()).pow(2.0).sum()/(M1));
    return std_dev;
}
