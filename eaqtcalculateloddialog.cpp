#include "eaqtcalculateloddialog.h"

EAQtCalculateLODDialog::EAQtCalculateLODDialog(CalibrationData *calibration)
{
    _calibration = calibration;
    _dialog = new QDialog();
    _dialog->setWindowTitle(tr("Calculation of LOD"));
    _dialog->setModal(true);
    QVBoxLayout *vb = new QVBoxLayout();
    QHBoxLayout *bl = new QHBoxLayout();
    _leSTDBlank = new QLineEdit();
    _leSTDBlank->setValidator(new QDoubleValidator());
    _lTXTBlank = new QLabel(tr("Standard deviation of blank [µA]: "));
    _butCalculate = new QPushButton(tr("Calculate LOD"));
    _lResult = new QLabel();
    this->connect(_butCalculate, SIGNAL(clicked(bool)), this, SLOT(calculate()));
    bl->addWidget(_lTXTBlank);
    bl->addWidget(_leSTDBlank);
    vb->addLayout(bl);
    vb->addWidget(_lResult);
    vb->addWidget(_butCalculate);
    _dialog->setLayout(vb);
}

EAQtCalculateLODDialog::~EAQtCalculateLODDialog()
{
    delete _dialog;
}

void EAQtCalculateLODDialog::exec()
{
    _dialog->exec();
}

void EAQtCalculateLODDialog::calculate()
{
    double blank_std = _leSTDBlank->text().toDouble();
    double slope = _calibration->slope;
    double inter = _calibration->intercept;
    double slope_std = _calibration->slopeStdDev;
    double inter_std = _calibration->interceptStdDev;
    double k = 3; // Limit of determination //
    double LOD = EAQtSignalProcessing::calcLOD(slope, slope_std, inter, inter_std, blank_std, k);
    _lResult->setText(tr("LOD: %1 %2").arg(LOD).arg(_calibration->xUnits));
}
