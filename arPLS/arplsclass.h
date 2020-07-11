#ifndef ARPLSCLASS_H
#define ARPLSCLASS_H

#include "Eigen/Dense"
#include <QVector>

class arPLSClass
{
public:
    arPLSClass();
    void compute(QVector<double>* yV, int32_t lambda, double ratio, int32_t maxIter, int32_t includeEnds,
                 double threshold, QVector<double>* rBkg, QVector<double>* rWeights, int* rIter, bool compute_in_float=false);

private:
    template <typename MATTYPE, typename VECTYPE, typename SCALARTYPE>
    void _compute(QVector<double>* yV, int32_t lambda, double ratio, int32_t maxIter, int32_t includeEnds,
                 double threshold, QVector<double>* rBkg, QVector<double>* rWeights, int* rIter);
    template<typename MATTYPE>
    MATTYPE _diff(MATTYPE);
    template<typename MATTYPE, typename SCALARTYPE>
    SCALARTYPE _std(MATTYPE);

};

#endif // ARPLSCLASS_H
