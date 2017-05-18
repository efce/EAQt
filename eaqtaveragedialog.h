#ifndef EAQTAVERAGEDIALOG_H
#define EAQTAVERAGEDIALOG_H

#include <QObject>
#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include "eaqtuiinterface.h"

class EAQtAverageDialog : public QObject
{
    Q_OBJECT

public:
    EAQtAverageDialog(EAQtUIInterface* pui);
    ~EAQtAverageDialog();
    void exec();

private:
    QDialog *_dialog;
    QVector<QCheckBox*> _list;
    QPushButton *_butAverage;
    QPushButton *_butClose;
    QPushButton *_butSelectAll;
    QPushButton *_butSelectNone;
    QVBoxLayout *_listBox;
    QScrollArea *_sa;
    QWidget *_scrollAreaWidget;
    EAQtUIInterface* _pUI;
    void generateList();
    void showError();

private slots:
    void average();
    void selectNone();
    void selectAll();
};

#endif // EAQTAVERAGEDIALOG_H
