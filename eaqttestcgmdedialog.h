#ifndef EAQTTESTCGMDEDIALOG_H
#define EAQTTESTCGMDEDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>

class EAQtTestCGMDEDialog : public QObject
{
    Q_OBJECT

public:
    EAQtTestCGMDEDialog();
    void exec();

private:
    QDialog *_dialog;
    QLineEdit *_leValveTime;
    QLineEdit *_leBreakTime;
    QLineEdit *_lePotential;

    QLineEdit *_leTestsNum;
    QLineEdit *_leAverageResult;
    QLineEdit *_leStandardDev;
    QLineEdit *_lePulseNum;
    QCheckBox *_cbUsePotential;

    int _testsNum;
    int _averageResult;
    int _stardardDev;
    int _pulseNum;

    void resetResults();

};

#endif // EAQTTESTCGMDEDIALOG_H
