#ifndef EAQTPARAMPOTENTIALPROGRAM_H
#define EAQTPARAMPOTENTIALPROGRAM_H

#include <QPlainTextEdit>
#include <QLabel>
#include <QDialog>

class EAQtParamPotentialProgram : public QDialog
{
    Q_OBJECT

public:
    explicit EAQtParamPotentialProgram(QWidget *parent = 0);

private:
    QLabel* summary;
    QPlainTextEdit* potentialProgramTextArea;
    QVector<int16_t> newPotentialList;
    QVector<int16_t> oldPotentialList;
    bool point_is_integer(const std::string& s);
    bool checkPotentialProgramArea();
    int separatorNum;
    QVector<char> separatorList;

private slots:
    void updateSummary();
    void clickedOK();
    void clickedCancel();
};

#endif // EAQTPARAMPOTENTIALPROGRAM_H
