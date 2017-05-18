#ifndef EAQTRECALCULATECURVEDIALOG_H
#define EAQTRECALCULATECURVEDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include "eaqtuiinterface.h"

class EAQtRecalculateCurveDialog : public QObject
{
    Q_OBJECT

public:
    EAQtRecalculateCurveDialog(EAQtUIInterface *pui);
    ~EAQtRecalculateCurveDialog();
    void exec();
private:
    QDialog *_dialog;
    QPushButton *_butRecalculate;
    QPushButton *_butClose;
    QLineEdit *_leTP;
    QLineEdit *_leTW;
    EAQtUIInterface* _pUI;
    int _tpplustw;
private slots:
    void recalculate();
};

#endif // EAQTRECALCULATECURVEDIALOG_H
