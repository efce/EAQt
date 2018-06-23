
  /*****************************************************************************************************************
  *  Electrochmical analyzer software EAQt to be used with 8KCA and M161
  *
  *  Copyright (C) 2017  Filip Ciepiela <filip.ciepiela@agh.edu.pl> and Małgorzata Jakubowska <jakubows@agh.edu.pl>
  *  This program is free software; you can redistribute it and/or modify 
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 3 of the License, or
  *  (at your option) any later version.
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software Foundation,
  *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
  *******************************************************************************************************************/
#ifndef EAQTMAINWINDOW_H
#define EAQTMAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets>
#include <QTimer>
#include "./Qcustomplot/qcustomplot.h"
#include "eaqtuiinterface.h"
#include "eaqtdata.h"
#include "eaqtplotcursor.h"
#include "eaqtmousehandler.h"
#include "displaycurveinfo.h"
#include "eaqttestcgmdedialog.h"

namespace Ui {
class MainWindow;
}

class EAQtMainWindow : public QMainWindow, EAQtUIInterface
{
    Q_OBJECT

public:
    explicit EAQtMainWindow(QSettings *s, QTranslator *t, QWidget *parent = 0);
    ~EAQtMainWindow();
    void InitialUpdate(EAQtData& d);
     void updateAll(bool rescale = true);
     EAQtUIInterface* getUIInterface();
     QCPCurve* PlotAddQCPCurve();
     bool PlotRemoveQCPCurve(QCPCurve* graph);
     EAQtPlotCursor *PlotAddCursor();
     double PlotGetXMiddle();
     void PlotReplot();
     void PlotSetInteraction(QCP::Interactions) ;
     QCPItemStraightLine* PlotAddLine();
     EAQtUIInterface::PlotLayer* PlotGetLayers();
     void PlotSetInverted(bool);
     void PlotDisconnectMouse();
     void PlotConnectMouse();
     void MeasurementSetup();
     void MeasurementAfter();
     void MeasurementUpdate(int32_t curveNr, int32_t pointNr);
     void showMessageBox(QString text, QString title = "");
     bool showQuestionBox(QString text, QString title = "", bool defaultIsYes = true);
     EAQtSaveFiledialog::SaveDetails DialogSaveInFile();
     void changeStartButtonText(QString);
     void setLowLabelText(int n, QString text);
     void setStatusText(QString str);
     void disableButtonsOnly();
     void disableButtonsAndTable();
     void enableAll();
     void updateCGMDETest();
     void setPathInUse(QString path);
     QString getPathInUse();
public slots:
     void PlotRescaleAxes(bool manual = false);

private slots:
    void handleMouseMoved(QMouseEvent*);
    void handleMouseDoubleClick(QMouseEvent*);
    void handleMouseReleased(QMouseEvent*);
    void handleMousePress(QMouseEvent*);

private:
    Ui::MainWindow *ui;
    QGridLayout *_mainLayout;
    QSettings* _settings;
    QTranslator* _translator;
    EAQtData *_pEAQtData;
    EAQtMouseHandler *_mouseHandler;
    QTableWidget *_tableCurveMain;
    QCustomPlot *_plotMain;
    QVector<EAQtPlotCursor*> _peakMarkers;
    QVector<QPushButton*> _vecButtonsDisablable;
    QPushButton* _butStartMes;
    QVector<QLabel*> _vecLowText;
    QComboBox *_comboOnXAxis;
    QCP::Interactions _plotDefaultInteractions;
    QTime *_timeOfMouse;
    bool _isRectangleZoom;
    QCPItemRect *_rectZoom;
    QPushButton *_butZoom;
    QWidget *_curveInfoMain;
    QPushButton *_butCurveInfoToggle;
    QPushButton *_butBkg;
    QPushButton *_butUndo;
    QTextEdit *_curveInfoData;
    EAQtUIInterface::PlotLayer _plotLayers;
    QString _PathInUse;
    EAQtTestCGMDEDialog *_dialogTestCGMDE;

private:
    void createActionsTopMenu();
    void createMenusTopMenu();
    QWidget* createCurveInfo();
    void updateCurveInfo();
    void clearPeaks();
    void PlotRegenerate();
    void TableRegenerate();
    void TableDrawSelection();
    void PlotDrawSelection();

    QGridLayout *createLayout();

    QMenu *_menuFile;
    QMenu *_menuMeasurement;
    QMenu *_menuInterpretation;
    QMenu *_menuAnalysis;
    QMenu *_menuCalibration;
    QMenu *_menuLanguage;
    QMenu *_menuAbout;

    QAction *_actStartMeasurement;
    QAction *_actPVMeasurement;
    QAction *_actLSVMeasurement;
    QAction *_actAccessories;
    QAction *_actCGMDEsettings;
    QAction *_actParamCopy;
    QAction *_actParamSave;
    QAction *_actParamLoad;

    QAction *_actLoadCurve;
    QAction *_actSaveCurve;
    QAction *_actExportCurve;
    QAction *_actRenameCurve;
    QAction *_actTakeScreenshot;

    QAction *_actAverage;
    QAction *_actDataCursor;
    QAction *_actBkgCorrection;
    QAction *_actBkgCorrectionSettings;
    QAction *_actSubtractActive;
    QAction *_actRelativeValues;
    QAction *_actMoveUpDown;
    QAction *_actSmooth;
    QAction *_actAdvSmooth;
    QAction *_actFindPeaks;
    QAction *_actStats;
    QAction *_actRecalculateCurve; // Change tp and tw based on nonaveraged measurement

    QAction *_actCalibrationData;
    QAction *_actCalibrationLoad;
    QAction *_actCalibrationResult;
    QAction *_actCalibrationShow;
    QAction *_actCalibrationClear;

    QAction *_actSoftware;
    QAction *_actReportIssues;
    QAction *_actSourceCode;
    QAction *_actSettings;

    QAction *_actLangPolish;
    QAction *_actLangEnglish;
    QActionGroup *_actGrLanguages;

private slots:
    void toggleCurveInfo(bool);
    void TableRowSelected();
    void PlotSelectionChanged();
    void PlotRectangleZoom();
    void ComboOnAxisSelected(int);
    void selectAll();
    void showParamDialogPV();
    void showParamDialogLSV();
    void showAccessoriesDialog();
    void showTestCGMDE();
    void deleteActive();
    void deleteNonactive();
    void deleteAll();
    void showOpenFile();
    void userStopsMeasurement();
    void userStartsMeasurement();
    void showSaveCurve();
    void paramCopy();
    void paramSave();
    void paramLoad();
    void showExportCurve();
    void showRenameCurve();
    void showAverage();
    void showDataCursor();
    void showPeaks();
    void showBackgroundCorrectionSettings();
    void startBackgroundCorrection();
    void subtractActive();
    void startRelativeValues();
    void startMoveUpDown();
    void startSmooth();
    void showAdvancedSmooth();
    void startCurvesStats();
    void showRecalculateCurve();
    void startCalibration();
    void loadCalibration();
    void clearCalibration();
    void showCalibration();
    void resultCalibration();
    void changeLanguageToPl();
    void changeLanguageToEn();
    void changeLanguage(QAction*);
    void showAboutSoftware();
    void showGithubIssues();
    void showGithub();
    void showSettings();
    void takeScreenshot();
    void undo();
};

#endif // MAINWINDOW_H
