#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include <Eigen/Dense>

using namespace Eigen;

// FLOAT
void arPLSVf(QVector<double>* y, int32_t lambda, float ratio, int32_t maxIter, int32_t includeEnds, float threshold,
             QVector<double>* rBkg, QVector<double>* rWeights, int* rIter);
MatrixXf diffMatrixXf(MatrixXf y);
float stdMatrixXf(MatrixXf y);
