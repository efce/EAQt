#ifndef EAQTTESTCGMDEDIALOG_H
#define EAQTTESTCGMDEDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include "eaqtdata.h"

class EAQtTestCGMDEDialog : public QObject
{
    Q_OBJECT

public:
    EAQtTestCGMDEDialog();
    ~EAQtTestCGMDEDialog();
public slots:
    void show();
    void close();
    void updateTest();

private:
    QDialog *_dialog;
    EAQtData::TestCGMDE *_values;

    QLineEdit *_leValveTime;
    QLineEdit *_leBreakTime;
    QLineEdit *_lePotential;

    QLineEdit *_leTestsNum;
    QLineEdit *_leAverageResult;
    QLineEdit *_leStandardDev;
    QLineEdit *_lePulseNum;
    QCheckBox *_cbUsePotential;

    QPushButton *_butTestStart;
    QPushButton *_butTestStop;
    QPushButton *_butClose;
    QPushButton *_butReset;

    int _testsNum;
    double _averageResult;
    double _stardardDev;
    int _pulseNum;
    std::vector<int> _vResults;

    void updateDialog();
    void readDialog();
    bool sendTestToEA();
    void calculateTest();

private slots:
    void resetResults();
    void TestStart();
    void TestStop();
};

#endif // EAQTTESTCGMDEDIALOG_H
