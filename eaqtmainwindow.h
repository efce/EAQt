
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
    explicit EAQtMainWindow(QWidget *parent = 0);
    ~EAQtMainWindow();
    void InitialUpdate(EAQtData& d);
     void updateAll(bool rescale = true);
     EAQtUIInterface* getUIInterface();
     QCPGraph* PlotAddGraph();
     bool PlotRemoveGraph(QCPGraph* graph);
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
     void MeasurementUpdate(uint32_t curveNr, uint32_t pointNr);
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
public slots:
     void PlotRescaleAxes();

private slots:
    void handleMouseMoved(QMouseEvent*);
    void handleMouseDoubleClick(QMouseEvent*);
    void handleMouseReleased(QMouseEvent*);
    void handleMousePress(QMouseEvent*);

private:
    Ui::MainWindow *ui;
    QGridLayout *_mainLayout;
    EAQtData *_pEAQtData;
    EAQtMouseHandler *_mouseHandler;
    QTableWidget *_tableCurveMain;
    QCustomPlot *_plotMain;
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
    QTextEdit *_curveInfoData;
    EAQtUIInterface::PlotLayer _plotLayers;
    QString _PathInUse;
    EAQtTestCGMDEDialog *_dialogTestCGMDE;

private:
    void createActionsTopMenu();
    void createMenusTopMenu();
    QWidget* createCurveInfo();
    void updateCurveInfo();
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

    QAction *_actAverage;
    QAction *_actDataCursor;
    QAction *_actBkgCorrection;
    QAction *_actRelativeValues;
    QAction *_actMoveUpDown;
    QAction *_actSmooth;
    QAction *_actAdvSmooth;
    QAction *_actRecalculateCurve; // Change tp and tw based on nonaveraged measurement

    QAction *_actCalibrationData;
    QAction *_actCalibrationLoad;
    QAction *_actCalibrationResult;
    QAction *_actCalibrationShow;
    QAction *_actCalibrationClear;

    QAction *_actSoftware;
    QAction *_actReportIssues;
    QAction *_actSourceCode;

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
    void openFile();
    void userStopsMeasurement();
    void userStartsMeasurement();
    void saveCurve();
    void paramCopy();
    void paramSave();
    void paramLoad();
    void exportCurve();
    void showAverage();
    void showDataCursor();
    void startBackgroundCorrection();
    void startRelativeValues();
    void startMoveUpDown();
    void startSmooth();
    void showAdvancedSmooth();
    void showRecalculateCurve();
    void startCalibration();
    void loadCalibration();
    void clearCalibration();
    void showCalibration();
    void resultCalibration();
    void showAboutSoftware();
    void showGithubIssues();
    void showGithub();
};

#endif // MAINWINDOW_H
