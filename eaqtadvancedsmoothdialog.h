#ifndef EAQTADVANCEDSMOOTHDIALOG_H
#define EAQTADVANCEDSMOOTHDIALOG_H

#include <QObject>
#include <QDialog>
#include <QRadioButton>
#include <QPushButton>
#include "./Qcustomplot/qcustomplot.h"

class EAQtAdvancedSmoothDialog : public QObject
{
    Q_OBJECT

private:
    QDialog *_dialog;
    QVector<QRadioButton*> _radMethod;
    QLineEdit *_leSGSpan;
    QLineEdit *_leSGOrder;
    QLineEdit *_leFTreshhold;
    QPushButton *_butSmooth;
    QPushButton *_butCancel;
    QPushButton *_butShowFrequency;
    QCustomPlot *_plotFreq;
    QCPGraph *_graph;
    std::vector<double> _params;
    enum method {
        method_sg,
        method_spline,
        method_fourier
    };
    enum paramsList {
        pl_order,
        pl_span,
        pl_treshhold,
    };

    QGridLayout* generateLayout(int select);
    void updateFrequencyPlot();

public:
    EAQtAdvancedSmoothDialog(int lastUsed, const std::vector<double>& params);
    void exec();
    void hide();

public slots:
    void methodChanged();

};

#endif // EAQTADVANCEDSMOOTHDIALOG_H
