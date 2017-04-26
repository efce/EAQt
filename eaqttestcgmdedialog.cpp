#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include "eaqtdata.h"
#include "eaqttestcgmdedialog.h"


EAQtTestCGMDEDialog::EAQtTestCGMDEDialog() : QObject()
{
    EAQtData::TestCGMDE *values = &EAQtData::getInstance()._testCGMDE;
    _dialog = new QDialog();
    _dialog->setWindowTitle(tr("CGMDE test"));
    _dialog->setModal(true);
    QGroupBox *params = new QGroupBox();
    QGridLayout *glparams = new QGridLayout();
    QLabel *lValveTime = new QLabel(tr("Valve time [ms]: "));
    _leValveTime = new QLineEdit();
    _leValveTime->setText(tr("%1").arg(values->GP));
    _leValveTime->setValidator(new QIntValidator(0,999));
    glparams->addWidget(lValveTime,0,0,1,1);
    glparams->addWidget(_leValveTime,0,1,1,1);
    QLabel *lBreakTime = new QLabel(tr("Break time [ms]: "));
    _leBreakTime = new QLineEdit();
    _leBreakTime->setText(tr("%1").arg(values->BT));
    _leBreakTime->setValidator(new QIntValidator(0,9999));
    glparams->addWidget(lBreakTime,1,0,1,1);
    glparams->addWidget(_leBreakTime,1,1,1,1);
    QLabel *lPotential = new QLabel(tr("Potential [mV]: "));
    _lePotential = new QLineEdit();
    _lePotential->setText(tr("%1").arg(values->ElPot));
    lPotential->setDisabled(values->ChPot);
    _cbUsePotential = new QCheckBox("");
    _cbUsePotential->setChecked(values->ChPot);
}
