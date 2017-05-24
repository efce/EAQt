#ifndef EAQTCURVERENAMEDIALOG_H
#define EAQTCURVERENAMEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include "curve.h"

class EAQtCurveRenameDialog : QObject
{
    Q_OBJECT

public:
    EAQtCurveRenameDialog(Curve* c);
    ~EAQtCurveRenameDialog();
    void exec();
private:
    QDialog *_dialog;
    QPushButton *_butOK;
    QPushButton *_butCancel;
    QLineEdit *_leName;
    QPlainTextEdit *_pteComment;
    Curve *_curve;
private slots:
    void OK();
};

#endif // EAQTCURVERENAMEDIALOG_H
