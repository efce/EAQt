#include "arPLSEigenF.h"

// FLOAT
void arPLSVf(QVector<double>* yV, int32_t lambda, float ratio, int32_t maxIter, int32_t includeEnds, float threshold,
             QVector<double>* rBkg, QVector<double>* rWeights, int* rIter)
{
    int32_t N = yV->size();

    MatrixXf y(N,1);
    for (int i = 0; i < N; i++)
    {
        y(i,0) = yV[0][i];
    }

    // init variables
    MatrixXf D(N-2,N);
    MatrixXf H(N,N);
    MatrixXf weights(N,1);
    MatrixXf W(N,N);
    MatrixXf C(N,N);
    MatrixXf bkg(N,1);
    MatrixXf wy(N,1);
    MatrixXf Cti(N,N);
    MatrixXf Ci(N,N);
    MatrixXf d(N,1);
    MatrixXf wt(N,1);

    // init operations
    D = diffMatrixXf(diffMatrixXf(MatrixXf::Identity(N,N)));
    H = lambda * (D.transpose() * D);
    weights = MatrixXf::Ones(N,1);

    int iter = 0;
    // main loop
    while(1)
    {

        W = weights.asDiagonal();

        // Cholesky decomposition
        LLT<MatrixXf,Upper> Chol(W + H);
        C = Chol.matrixU();

        wy = weights.cwiseProduct(y);
        Ci = C.inverse();
        Cti = Ci.transpose();
        bkg = Ci * (Cti * wy);
        d = y - bkg;

        int32_t ind1 = 0;
        MatrixXf dn = MatrixXf::Zero(N,1);
        for(int i = 0; i < N; i++)
        {
            if( d(i,0) < 0 )
            {
                dn(ind1,0) = d(i,0);
                ind1++;
            }
        }
        // mean
        MatrixXf dns = dn.block(0,0,ind1,1);
        float m = dns.mean();
        // std
        float s = stdMatrixXf(dns);

        wt = 1 / (1 + ((2*(d - (2*s-m)*MatrixXf::Ones(N,1)))/s).array().exp());

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
            LLT<MatrixXf,Upper> Chol(W + H);
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

MatrixXf diffMatrixXf(MatrixXf y)
{
    int32_t m = y.innerSize();
    int32_t n = y.outerSize();
    MatrixXf results(m-1,n);

    for(int i=1; i < m; i++)
    {
        results.row(i-1) = y.row(i) - y.row(i-1);
    }

    return results;
}

float stdMatrixXf(MatrixXf y)
{
    int32_t m = y.innerSize();
    float mean = y.mean();
    float standardDeviation = 0.0;

    for(int i = 0; i < m; i++)
    {
        standardDeviation += pow(y(i,0) - mean, 2);
    }

    return sqrt(standardDeviation / (m-1));
}
