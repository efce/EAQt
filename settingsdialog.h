#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include "eaqtdata.h"


class SettingsDialog : public QObject
{
    Q_OBJECT
public:
    SettingsDialog(QSettings* pSettings);
    ~SettingsDialog();

private:
    void save();

    QDialog *_dialog;
    QLineEdit *_leFontMain;
    QLineEdit *_leFontPlotLabel;
    QLineEdit *_leFontPlotTick;
    QLineEdit *_leLineWidth;
    QPushButton *_butCancel;
    QPushButton *_butOK;
    QSettings *_settings;

public slots:
    void show();
    void close();
    void OK();

};

#endif // SETTINGSDIALOG_H
