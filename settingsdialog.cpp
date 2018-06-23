#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QSettings* pSettings) : QObject()
{
    _settings = pSettings;
    _dialog = new QDialog();
    QHBoxLayout* mainLay = new QHBoxLayout();
    _dialog->setWindowTitle(tr("Settings"));
    _dialog->setModal(true);
    int main_font_size = pSettings->value("main_font_size", 13).toInt();
    int plot_label_font_size = pSettings->value("plot_labels_font_size", 13).toInt();
    int plot_ticks_font_size = pSettings->value("plot_ticks_font_size", 13).toInt();
    int line_width = pSettings->value("screenshot_line_width", 1).toInt();

    QGroupBox *params = new QGroupBox(tr("Settings (requires restart):"));
    QGridLayout *glparams = new QGridLayout();

    QLabel *font_plot = new QLabel(tr("Plot labels font size: "));
    _leFontPlotLabel = new QLineEdit();
    _leFontPlotLabel->setText(tr("%1").arg(plot_label_font_size));
    _leFontPlotLabel->setValidator(new QIntValidator(4, 99, _dialog));
    glparams->addWidget(font_plot,0,0,1,1);
    glparams->addWidget(_leFontPlotLabel,0,1,1,1);

    QLabel *font_plot_tick = new QLabel(tr("Plot ticks font size: "));
    _leFontPlotTick = new QLineEdit();
    _leFontPlotTick->setText(tr("%1").arg(plot_ticks_font_size));
    _leFontPlotTick->setValidator(new QIntValidator(4, 99, _dialog));
    glparams->addWidget(font_plot_tick,1,0,1,1);
    glparams->addWidget(_leFontPlotTick,1,1,1,1);

    QLabel *font_main = new QLabel(tr("Main font size: "));
    _leFontMain = new QLineEdit();
    _leFontMain->setText(tr("%1").arg(main_font_size));
    _leFontMain->setValidator(new QIntValidator(4, 99, _dialog));
    glparams->addWidget(font_main,2,0,1,1);
    glparams->addWidget(_leFontMain,2,1,1,1);

    QLabel *lw = new QLabel(tr("Line width for image: "));
    _leLineWidth = new QLineEdit();
    _leLineWidth->setText(tr("%1").arg(line_width));
    _leLineWidth->setValidator(new QIntValidator(1, 99, _dialog));
    glparams->addWidget(lw,3,0,1,1);
    glparams->addWidget(_leLineWidth,3,1,1,1);

    _butOK = new QPushButton(tr("OK"));
    this->connect(_butOK, SIGNAL(clicked(bool)), this, SLOT(OK()));
    _butCancel = new QPushButton(tr("Cancel"));
    this->connect(_butCancel, SIGNAL(clicked(bool)), this, SLOT(close()));

    glparams->addWidget(_butOK,4,0,1,1);
    glparams->addWidget(_butCancel,4,1,1,1);

    params->setLayout(glparams);
    mainLay->addWidget(params);
    _dialog->setLayout(mainLay);
}

SettingsDialog::~SettingsDialog()
{

}


void SettingsDialog::show()
{
    _dialog->show();
    return;
}


void SettingsDialog::close()
{
    _dialog->close();
    return;
}

void SettingsDialog::save()
{
    int main_font_size = _leFontMain->text().toInt();
    int plot_font_size = _leFontPlotLabel->text().toInt();
    int plot_ticks_font_size = _leFontPlotTick->text().toInt();
    int line_width = _leLineWidth->text().toInt();
    _settings->setValue("main_font_size", main_font_size);
    _settings->setValue("plot_labels_font_size", plot_font_size);
    _settings->setValue("plot_ticks_font_size", plot_ticks_font_size);
    _settings->setValue("screenshot_line_width", line_width);
}

void SettingsDialog::OK()
{
    this->save();
    this->close();
}
