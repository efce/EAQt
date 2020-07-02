#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include <Eigen/Dense>

using namespace Eigen;

// DOUBLE
void arPLSVd(QVector<double>* y, int32_t lambda, double ratio, int32_t maxIter, int32_t includeEnds, double threshold,
             QVector<double>* rBkg, QVector<double>* rWeights, int* rIter);
MatrixXd diffMatrixXd(MatrixXd y);
double stdMatrixXd(MatrixXd y);
