#include "arPLSEigenD.h"

using namespace std;
using namespace Eigen;

// DOUBLE
void arPLSVd(QVector<double>* yV, int32_t lambda, double ratio, int32_t maxIter, int32_t includeEnds, double threshold,
             QVector<double>* rBkg, QVector<double>* rWeights, int* rIter)
{
    int32_t N = yV->size();

    MatrixXd y(N,1);
    for (int i = 0; i < N; i++)
    {
        y(i,0) = yV[0][i];
    }

    // init variables
    MatrixXd D(N-2,N);
    MatrixXd H(N,N);
    VectorXd weights(N,1);
    MatrixXd W(N,N);
    MatrixXd C(N,N);
    VectorXd bkg(N,1);
    VectorXd wy(N,1);
    MatrixXd Cti(N,N);
    MatrixXd Ci(N,N);
    VectorXd d(N,1);
    VectorXd wt(N,1);

    // init operations
    D = diffMatrixXd(diffMatrixXd(MatrixXd::Identity(N,N)));
    H = lambda * (D.transpose() * D);
    weights = VectorXd::Ones(N,1);

    int iter = 0;
    // main loop
    while(1)
    {

        W = weights.asDiagonal();

        // Cholesky decomposition
        LLT<MatrixXd,Upper> Chol(W + H);
        C = Chol.matrixU();

        wy = weights.cwiseProduct(y);
        Ci = C.inverse();
        Cti = Ci.transpose();
        bkg = Ci * (Cti * wy);
        d = y - bkg;

        int32_t ind1 = 0;
        VectorXd dn = VectorXd::Zero(N,1);
        for(int i = 0; i < N; i++)
        {
            if( d(i,0) < 0 )
            {
                dn(ind1,0) = d(i,0);
                ind1++;
            }
        }
        // mean
        VectorXd dns = dn.block(0,0,ind1,1);
        double m = dns.mean();
        // std
        double s = stdMatrixXd(dns);

        wt = 1 / (1 + ((2*(d - (2*s-m)*VectorXd::Ones(N,1)))/s).array().exp());

//        cout << iter << endl;
//        cout << "ratio = " << (weights-wt).norm()/weights.norm() << endl;

        if( ((weights-wt).norm()/weights.norm() < ratio) || (iter >= maxIter) )
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
            LLT<MatrixXd,Upper> Chol(W + H);
            C = Chol.matrixU();
            wy = weights.cwiseProduct(y);
            Ci = C.inverse();
            Cti = Ci.transpose();
            bkg = Ci * (Cti * wy);

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

MatrixXd diffMatrixXd(MatrixXd y)
{
    int32_t m = y.innerSize();
    int32_t n = y.outerSize();
    MatrixXd results(m-1,n);

    for(int i=1; i < m; i++)
    {
        results.row(i-1) = y.row(i) - y.row(i-1);
    }

    return results;
}

double stdMatrixXd(MatrixXd y)
{
    int32_t m = y.innerSize();
    double mean = y.mean();
    double standardDeviation = 0.0;

    for(int i = 0; i < m; i++)
    {
        standardDeviation += pow(y(i,0) - mean, 2);
    }

    return sqrt(standardDeviation / (m-1));
}
