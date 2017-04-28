
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
#include <QTimer>
#include "ui_mainwindow.h"
#include "eaqtmainwindow.h"
#include "eaqtopenfiledialog.h"
#include "eaqtparamdialog.h"
#include "eaqtaccessoriesdialog.h"
#include "eaqtadvancedsmoothdialog.h"
#include "eaqttestcgmdedialog.h"

EAQtMainWindow::EAQtMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->retranslateUi(this);
    //ui->statusBar->hide();
    setGeometry(400, 250, 1100, 800);
    this->_timeOfMouse = new QTime();
    this->_mainLayout = createLayout();
    ui->centralWidget->setLayout(this->_mainLayout);
    this->createActionsTopMenu();
    this->createMenusTopMenu();
    this->setWindowTitle(tr("EAQt - electrochemical analyzer software"));
    _isRectangleZoom = false;
    _PathInUse = "";
    new QShortcut( QKeySequence(Qt::Key_Escape), this, SLOT(userStopsMeasurement()));
    new QShortcut( QKeySequence(Qt::Key_F5), this, SLOT(userStartsMeasurement()));
    new QShortcut( QKeySequence(Qt::Key_Delete), this, SLOT(deleteActive()));
    new QShortcut( QKeySequence(Qt::CTRL + Qt::Key_X), this, SLOT(deleteActive()));
    new QShortcut( QKeySequence(Qt::CTRL + Qt::Key_A), this,SLOT(selectAll()));
}

void EAQtMainWindow::InitialUpdate(EAQtData& d)
{
    this->_pEAQtData = &d;
    this->_mouseHandler = new EAQtMouseHandler(this->_pEAQtData, this);
    this->_mouseHandler->ChangeMouseMode(EAQtMouseHandler::mm_normal, EAQtMouseHandler::uf_none);
    connect(_pEAQtData,SIGNAL(actChanged(int)),_mouseHandler,SLOT(onSelectionChanged()));
}

void EAQtMainWindow::PlotConnectMouse()
{
    connect(this->_plotMain,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(handleMouseMoved(QMouseEvent*)), Qt::UniqueConnection);
    connect(this->_plotMain,SIGNAL(mousePress(QMouseEvent*)), this,SLOT(handleMousePress(QMouseEvent*)), Qt::UniqueConnection);
    connect(this->_plotMain,SIGNAL(mouseRelease(QMouseEvent*)),this,SLOT(handleMouseReleased(QMouseEvent*)), Qt::UniqueConnection);
}

void EAQtMainWindow::PlotDisconnectMouse()
{
    disconnect(this->_plotMain,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(handleMouseMoved(QMouseEvent*)));
    disconnect(this->_plotMain,SIGNAL(mousePress(QMouseEvent*)), this,SLOT(handleMousePress(QMouseEvent*)));
    disconnect(this->_plotMain,SIGNAL(mouseRelease(QMouseEvent*)),this,SLOT(handleMouseReleased(QMouseEvent*)));
}

void EAQtMainWindow::userStopsMeasurement()
{
    this->_pEAQtData->MesStop();
}

EAQtUIInterface* EAQtMainWindow::getUIInterface()
{
    return this;
}

EAQtMainWindow::~EAQtMainWindow()
{
    delete ui;
    delete _timeOfMouse;
    delete _mouseHandler;
}

void EAQtMainWindow::handleMouseDoubleClick(QMouseEvent *me)
{
    me->accept();
    double x = this->_plotMain->xAxis->pixelToCoord(me->pos().x());
    double y = this->_plotMain->yAxis->pixelToCoord(me->pos().y());
    this->_mouseHandler->ForwardClick(x,y);
}

void EAQtMainWindow::handleMouseMoved(QMouseEvent *me)
{
    if ( _isRectangleZoom ) {
         me->accept();
         _rectZoom->bottomRight->setCoords(_plotMain->xAxis->pixelToCoord(me->pos().x())
                                       ,_plotMain->yAxis->pixelToCoord(me->pos().y()));
         _plotLayers.Markers->replot();
         return;
    }
    me->accept();
    double x = this->_plotMain->xAxis->pixelToCoord(me->pos().x());
    double y = this->_plotMain->yAxis->pixelToCoord(me->pos().y());
    this->_mouseHandler->UpdateLocation(&x,&y);
}

void EAQtMainWindow::handleMousePress(QMouseEvent *me)
{
    if ( _isRectangleZoom ) {
         me->accept();
        _butZoom->setDown(true);
         _plotMain->setInteractions(NULL);
         _rectZoom->topLeft->setCoords(_plotMain->xAxis->pixelToCoord(me->pos().x())
                                       ,_plotMain->yAxis->pixelToCoord(me->pos().y()));
         _rectZoom->bottomRight->setCoords(_plotMain->xAxis->pixelToCoord(me->pos().x())
                                       ,_plotMain->yAxis->pixelToCoord(me->pos().y()));
         _rectZoom->setVisible(true);
         _plotMain->replot();
         return;
    } else {
        this->_timeOfMouse->restart();
        me->ignore();
    }
}

void EAQtMainWindow::handleMouseReleased(QMouseEvent *me)
{
    if ( _isRectangleZoom ) {
        me->accept();
        _isRectangleZoom = false;
        _butZoom->setDown(false);
        _plotMain->setInteractions(_mouseHandler->_plotInteractions.plotDefaultInteractions);
         _rectZoom->bottomRight->setCoords(_plotMain->xAxis->pixelToCoord(me->pos().x())
                                       ,_plotMain->yAxis->pixelToCoord(me->pos().y()));
        _rectZoom->setVisible(false);
        _plotMain->xAxis->setRangeUpper(_rectZoom->bottomRight->coords().x());
        _plotMain->xAxis->setRangeLower(_rectZoom->topLeft->coords().x());
        _plotMain->yAxis->setRangeUpper(_rectZoom->topLeft->coords().y());
        _plotMain->yAxis->setRangeLower(_rectZoom->bottomRight->coords().y());
        PlotReplot();
        return;
    }
    int mouseElapsed = this->_timeOfMouse->elapsed();
    if ( Qt::RightButton == me->button() ) {
        me->accept();
        this->_mouseHandler->BackClick();
        return;
    } else {
        if ( mouseElapsed > 300 ) {
            // 300 ms from press -- maybe its pan not click ?
            me->ignore();
            return;
        } else {
            if ( !this->_mouseHandler->wantsClicks() ) {
                me->ignore();
                return;
            }
            me->accept();
            double x = this->_plotMain->xAxis->pixelToCoord(me->pos().x());
            double y = this->_plotMain->yAxis->pixelToCoord(me->pos().y());
            this->_mouseHandler->ForwardClick(x,y);
        }
    }
}

bool EAQtMainWindow::PlotRemoveGraph(QCPGraph* graph)
{
    return this->_plotMain->removeGraph(graph);
}

void EAQtMainWindow::updateAll(bool rescale)
{
    switch ( this->_pEAQtData->getXAxis() ) {
    default:
    case XAXIS::potential:
        this->_plotMain->xAxis->setLabel("E / mV");
        break;
    case XAXIS::time:
        this->_plotMain->xAxis->setLabel("t / ms");
        break;
    case XAXIS::nonaveraged:
        this->_plotMain->xAxis->setLabel("sample no.");
        break;
    }
    setLowLabelText(3,tr("curves: %1").arg(_pEAQtData->getCurves()->count()));
    this->_plotMain->yAxis->setLabel("i / µA");
    this->_comboOnXAxis->blockSignals(true);
    this->_comboOnXAxis->setCurrentIndex(this->_pEAQtData->getXAxis());
    this->_comboOnXAxis->blockSignals(false);
    this->TableRegenerate();
    this->PlotRegenerate();
    this->TableDrawSelection();
    this->PlotDrawSelection();
    if ( rescale ) {
        PlotRescaleAxes();
    } else {
        PlotReplot();
    }
}

void EAQtMainWindow::TableRegenerate()
{
    this->_tableCurveMain->setUpdatesEnabled(false);
    this->_tableCurveMain->blockSignals(true);
    this->_tableCurveMain->clear();
    _tableCurveMain->setRowCount(this->_pEAQtData->getCurves()->count());
    _tableCurveMain->setColumnCount(2);
    Curve* curve;
    int i = 0;
    QFont striked = _tableCurveMain->font();
    striked.setStrikeOut(true);
    while ( (curve=this->_pEAQtData->getCurves()->get(i)) != NULL ) {
        QTableWidgetItem *qtifile = new QTableWidgetItem(curve->FName());
        qtifile->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        qtifile->setSelected(false);
        qtifile->setTextAlignment(Qt::AlignRight + Qt::AlignCenter);
        QTableWidgetItem *qtiname = new QTableWidgetItem(curve->CName());
        qtiname->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        qtiname->setSelected(false);
        qtiname->setTextAlignment(Qt::AlignRight + Qt::AlignCenter);
        if ( _pEAQtData->getXAxis() == XAXIS::nonaveraged ) {
            if ( curve->Param(PARAM::nonaveragedsampling) == 0 ) {
                qtifile->setFont(striked);
                qtiname->setFont(striked);
            }
        }
        this->_tableCurveMain->setItem(i,0,qtifile);
        this->_tableCurveMain->setItem(i,1,qtiname);
        ++i;
    }
    _tableCurveMain->blockSignals(false);
    _tableCurveMain->setUpdatesEnabled(true);
}

void EAQtMainWindow::PlotRegenerate()
{
    int i = 0;
    this->_plotMain->setUpdatesEnabled(false);
    this->_plotMain->blockSignals(true);
    Curve* curve;
    while ( (curve=this->_pEAQtData->getCurves()->get(i)) != NULL ) {
        curve->getPlot()->setData(curve->getXVector(), curve->getYVector());
        curve->getPlot()->setPen(QPen(COLOR::regular));
        curve->getPlot()->setLayer(_plotLayers.NonActive);
        curve->getPlot()->setSelection(QCPDataSelection());
        ++i;
    }
    this->_plotMain->blockSignals(false);
    this->_plotMain->setUpdatesEnabled(true);
}

void EAQtMainWindow::TableRowSelected()
{
    int row;
    int rowcount = this->_tableCurveMain->rowCount();
    int selitems = this->_tableCurveMain->selectedItems().size();
    if ( selitems == 0 ) {
        if ( rowcount > this->_pEAQtData->Act() ) {
            row = this->_pEAQtData->Act();
            this->updateAll();
        } else {
            return;
        }
    } else {
        row = this->_tableCurveMain->selectedItems().at(0)->row();
    }
    this->_pEAQtData->Act(row);
    this->PlotDrawSelection();
    this->TableDrawSelection();
}

void EAQtMainWindow::PlotDrawSelection()
{
    int i = 0;
    this->_plotMain->setUpdatesEnabled(false);
    this->_plotMain->blockSignals(true);
    Curve* curve;
    while ( (curve=this->_pEAQtData->getCurves()->get(i)) != NULL ) {
        if ( this->_pEAQtData->Act() == i
        || this->_pEAQtData->Act() == SELECT::all ) {
            curve->getPlot()->setLayer(_plotLayers.Active);
            curve->getPlot()->setPen(QPen(COLOR::active));
            curve->getPlot()->setSelection(QCPDataSelection(curve->getPlot()->data()->dataRange()));
        } else {
            curve->getPlot()->setPen(QPen(COLOR::regular));
            curve->getPlot()->setLayer(_plotLayers.NonActive);
            curve->getPlot()->setSelection(QCPDataSelection());
        }
        ++i;
    }
    this->_plotMain->blockSignals(false);
    this->_plotMain->setUpdatesEnabled(true);
    this->updateCurveInfo();
    this->_plotMain->replot();
}

void EAQtMainWindow::TableDrawSelection()
{
    int i = 0;
    Curve* curve;
    if ( this->_pEAQtData->Act() > this->_tableCurveMain->rowCount() ) {
        throw 1;
    }
    _tableCurveMain->setUpdatesEnabled(false);
    _tableCurveMain->blockSignals(true);
    if ( this->_pEAQtData->Act() == SELECT::all ) {
        this->_tableCurveMain->selectAll();
    } else {
        while ( (curve=this->_pEAQtData->getCurves()->get(i)) != NULL ) {
            if ( i == this->_pEAQtData->Act() ) {
                this->_tableCurveMain->item(i,0)->setSelected(true);
                this->_tableCurveMain->item(i,1)->setSelected(true);
                _tableCurveMain->scrollToItem(_tableCurveMain->item(i,0));
                _tableCurveMain->setCurrentItem(_tableCurveMain->item(i,0));
            } else {
                this->_tableCurveMain->item(i,0)->setSelected(false);
                this->_tableCurveMain->item(i,1)->setSelected(false);
            }
            ++i;
        }
    }
    _tableCurveMain->resizeColumnToContents(0);
    _tableCurveMain->horizontalScrollBar()->setValue(_tableCurveMain->horizontalScrollBar()->maximum());
    this->_tableCurveMain->blockSignals(false);
    _tableCurveMain->setUpdatesEnabled(true);
}

void EAQtMainWindow::PlotSelectionChanged()
{
    int selectedGraphID = -1;
    QList<QCPGraph*> ql =  this->_plotMain->selectedGraphs();
    Curve *curve;

    foreach (QCPGraph* gg, ql) {
        int i = 0;
        while ( (curve=this->_pEAQtData->getCurves()->get(i)) != NULL ) {
            if ( gg == curve->getPlot() ) {
                selectedGraphID = i;
                break;
            }
            ++i;
        }
    }
    if ( selectedGraphID == -1 ) {
        return;
    }
    this->_pEAQtData->Act(selectedGraphID);
    this->PlotDrawSelection();
    this->TableDrawSelection();
}

void EAQtMainWindow::selectAll()
{
    this->_pEAQtData->Act(SELECT::all);
    this->TableDrawSelection();
    this->PlotDrawSelection();
}

QGridLayout* EAQtMainWindow::createLayout()
{
    QVBoxLayout *mainButtonLayout = new QVBoxLayout();

    _tableCurveMain = new QTableWidget(this);
    _tableCurveMain->setUpdatesEnabled(true);
    _tableCurveMain->setColumnCount(2);
    _tableCurveMain->setRowCount(0);
    _tableCurveMain->horizontalHeader()->hide();
    _tableCurveMain->verticalHeader()->hide();
    _tableCurveMain->setAutoScroll(false);
    _tableCurveMain->setColumnWidth(1,100);
    _tableCurveMain->setFixedWidth(257);
    _tableCurveMain->setShowGrid(false);
    _tableCurveMain->setAlternatingRowColors(true);
    _tableCurveMain->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _tableCurveMain->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _tableCurveMain->setSelectionBehavior(QAbstractItemView::SelectRows);
    QString tabelStyle = "QTableWidget::item:selected{ background-color: rgb(%1, %2, %3); color: white } ";
    tabelStyle = tabelStyle.arg(COLOR::active.red()).arg(COLOR::active.green()).arg(COLOR::active.blue());
    _tableCurveMain->setStyleSheet(tabelStyle);

    mainButtonLayout->addWidget(_tableCurveMain,4);
    connect(_tableCurveMain,SIGNAL(itemSelectionChanged()),this,SLOT(TableRowSelected()));

    QPushButton *butDeleteActive;
    butDeleteActive = new QPushButton(tr("Delete active"));
    butDeleteActive->setStatusTip(tr("Delete active curve from plot"));
    this->_vecButtonsDisablable.append( butDeleteActive );
    connect(butDeleteActive, SIGNAL(clicked(bool)), this, SLOT(deleteActive()));

    QPushButton *butDeleteAll;
    butDeleteAll = new QPushButton(tr("Delete all"));
    butDeleteAll->setStatusTip(tr("Delete all curves from plot"));
    this->_vecButtonsDisablable.append( butDeleteAll );
    connect(butDeleteAll,SIGNAL(clicked(bool)),this,SLOT(deleteAll()));

    QPushButton *butDeleteNonactive;
    butDeleteNonactive = new QPushButton(tr("Delete non-active"));
    butDeleteNonactive->setStatusTip(tr("Delete all non active curves from plot"));
    this->_vecButtonsDisablable.append( butDeleteNonactive );
    connect(butDeleteNonactive,SIGNAL(clicked(bool)),this,SLOT(deleteNonactive()));

    QPushButton *butSelectAll;
    butSelectAll = new QPushButton(tr("Select all"));
    butSelectAll->setStatusTip(tr("Activate all curves on plot"));
    this->_vecButtonsDisablable.append( butSelectAll );
    connect(butSelectAll, SIGNAL(clicked(bool)), this, SLOT(selectAll()));

    QPushButton *butOpenFile;
    butOpenFile = new QPushButton(tr("Load"));
    butOpenFile->setStatusTip(tr("Load curves from file"));
    this->_vecButtonsDisablable.append( butOpenFile );
    connect(butOpenFile,SIGNAL(clicked(bool)),this,SLOT(openFile()));

    this->_butStartMes = new QPushButton(tr("Start PV"));
    _butStartMes->setStatusTip(tr("Start measurement with the same settings as previuos"));
    this->_vecButtonsDisablable.append( _butStartMes );
    connect(_butStartMes,SIGNAL(clicked(bool)),this,SLOT(userStartsMeasurement()));

    QPushButton *butStopMes = new QPushButton(tr("Stop"));
    butStopMes->setStatusTip(tr("Stop the measurement"));
    //this->vecButtonsDisablable.append( butStartMes );
    connect(butStopMes,SIGNAL(clicked(bool)),this,SLOT(userStopsMeasurement()));

    QPushButton *butParamPV;
    butParamPV = new QPushButton(tr("PV param"));
    butParamPV->setStatusTip(tr("Show dialog with PV settigns"));
    this->_vecButtonsDisablable.append( butParamPV );
    connect(butParamPV,SIGNAL(clicked(bool)),this,SLOT(showParamDialogPV()));

    QPushButton *butParamLSV;
    butParamLSV = new QPushButton(tr("LSV param"));
    this->_vecButtonsDisablable.append( butParamLSV );
    butParamLSV->setStatusTip(tr("Show dialog with LSV settings"));
    connect(butParamLSV,SIGNAL(clicked(bool)),this,SLOT(showParamDialogLSV()));

    QWidget *comboWidget = new QWidget();
    QHBoxLayout *combo = new QHBoxLayout();
    QLabel *lcombo = new QLabel(tr("On X axis:"));
    combo->addWidget(lcombo);
    this->_comboOnXAxis = new QComboBox();
    QList<QString> comboList;
    comboList.reserve(3);
    comboList.insert(XAXIS::potential, tr("Potential"));
    comboList.insert(XAXIS::time, tr("Time"));
    comboList.insert(XAXIS::nonaveraged, tr("Sample no"));
    QStringList comboSl = QStringList(comboList);
    _comboOnXAxis->addItems(comboSl);
    connect(_comboOnXAxis, SIGNAL(activated(int)), this, SLOT(ComboOnAxisSelected(int)));
    comboWidget->setLayout(combo);
    combo->addWidget(_comboOnXAxis);

    _butZoom = new QPushButton();
    QIcon izoom = QIcon(":/icons/icon_zoom");
    _butZoom->setIcon(izoom);
    _butZoom->setFixedWidth(30);
    _butZoom->setFixedHeight(30);
    connect(_butZoom,SIGNAL(clicked(bool)),this,SLOT(PlotRectangleZoom()));

    QPushButton *butRescale = new QPushButton();
    QIcon irescale = QIcon(":/icons/icon_rescale");
    butRescale->setIcon(irescale);
    butRescale->setFixedWidth(30);
    butRescale->setFixedHeight(30);
    connect(butRescale,SIGNAL(clicked(bool)),this,SLOT(PlotRescaleAxes()));

    mainButtonLayout->addWidget(butDeleteActive);
    mainButtonLayout->addWidget(butDeleteAll);
    mainButtonLayout->addWidget(butDeleteNonactive);
    mainButtonLayout->addWidget(butSelectAll);
    this->ui->mainToolBar->addWidget(butOpenFile);
    this->ui->mainToolBar->addWidget(_butStartMes);
    this->ui->mainToolBar->addWidget(butStopMes);
    this->ui->mainToolBar->addWidget(butParamPV);
    this->ui->mainToolBar->addWidget(butParamLSV);
    this->ui->mainToolBar->addWidget(comboWidget);
    this->ui->mainToolBar->addWidget(_butZoom);
    this->ui->mainToolBar->addWidget(butRescale);

    this->_plotMain = new QCustomPlot();
    _plotMain->setNoAntialiasingOnDrag(true);
    this->_plotMain->addLayer("Nonactive");
    _plotLayers.NonActive = _plotMain->layer("Nonactive");
    this->_plotMain->addLayer("Active");
    _plotLayers.Active = _plotMain->layer("Active");
    this->_plotMain->addLayer("Markers");
    _plotLayers.Markers = _plotMain->layer("Markers");
    this->_plotMain->addLayer("Measurement");
    _plotLayers.Measurement = _plotMain->layer("Measurement");
    _plotLayers.Measurement->setMode(QCPLayer::lmBuffered);
    _plotLayers.Markers->setMode(QCPLayer::lmBuffered);
    this->_plotMain->setInteractions( this->_plotDefaultInteractions );
    this->_rectZoom = new QCPItemRect(_plotMain);
    _rectZoom->setPen(QPen(QColor(50,50,50),2,Qt::DashLine));
    _rectZoom->setVisible(false);
    _rectZoom->setLayer(_plotLayers.Markers);
    _rectZoom->setSelectable(false);
    QGridLayout* mainLayout = new QGridLayout();

    mainLayout->addWidget(this->_plotMain,0,1,1,1);
    mainLayout->addItem(mainButtonLayout,0,2,1,1);
    mainLayout->setColumnStretch(1,5);

    QHBoxLayout *labelsBox = new QHBoxLayout();
    this->_vecLowText.resize(5);
    for (int i=0; i<_vecLowText.size(); ++i) {
        this->_vecLowText[i] = new QLabel(tr("%1").arg(i));
        labelsBox->addWidget(_vecLowText[i]);
    }
    mainLayout->addLayout(labelsBox,3,1,1,2);

    _curveInfoMain = new QWidget();
    _curveInfoData = new QTextEdit();
    _curveInfoData->setReadOnly(true);
    _curveInfoData->setFixedWidth(250);
    _curveInfoData->setContentsMargins(0,0,0,0);
    _curveInfoData->setVisible(false);
    _curveInfoData->setStyleSheet(".curveInfo { margin: 0px; padding: 0px; border: solid 1px black; }");
    QHBoxLayout* hbl = new QHBoxLayout();
    hbl->setMargin(0);
    hbl->setSpacing(0);
    hbl->addStretch(0);
    _butCurveInfoToggle = new QPushButton();
    _butCurveInfoToggle->setText(">");
    _butCurveInfoToggle->setCheckable(true);
    _butCurveInfoToggle->setChecked(false);
    _butCurveInfoToggle->setFixedWidth(15);
    _butCurveInfoToggle->setFixedHeight(100);
    connect(_butCurveInfoToggle,SIGNAL(toggled(bool)),this,SLOT(toggleCurveInfo(bool)));
    hbl->addWidget(_curveInfoData);
    hbl->addWidget(_butCurveInfoToggle);
    _curveInfoMain->setLayout(hbl);
    mainLayout->addWidget(_curveInfoMain,0,0,2,1);

    connect(this->_plotMain, SIGNAL(selectionChangedByUser()), this, SLOT(PlotSelectionChanged()));

    return mainLayout;
}

void EAQtMainWindow::deleteActive()
{
    this->_pEAQtData->deleteActiveCurveFromGraph();
}

void EAQtMainWindow::deleteNonactive()
{
    this->_pEAQtData->deleteNonactiveCurvesFromGraph();
}

void EAQtMainWindow::deleteAll()
{
    this->_pEAQtData->deleteAllCurvesFromGraph();
}

void EAQtMainWindow::openFile()
{
    new EAQtOpenFileDialog(this->_pEAQtData);
}

QCPGraph* EAQtMainWindow::PlotAddGraph()
{
    QCPGraph* qg = this->_plotMain->addGraph();
    qg->setSelectable(QCP::stWhole);
    qg->setSelectionDecorator(NULL);
    qg->setPen(QPen(COLOR::regular));
    return qg;
}

void EAQtMainWindow::PlotRescaleAxes()
{
    this->_plotMain->rescaleAxes(true);
    this->_plotMain->replot();
}

void EAQtMainWindow::showParamDialogPV()
{
    EAQtParamDialog *pd = new EAQtParamDialog((EAQtDataInterface*)this->_pEAQtData,false);
    pd->show();
}

void EAQtMainWindow::showParamDialogLSV()
{
    EAQtParamDialog *pd = new EAQtParamDialog((EAQtDataInterface*)this->_pEAQtData,true);
    pd->show();
}

void EAQtMainWindow::disableButtonsOnly()
{
    for ( int i = 0; i<_vecButtonsDisablable.size(); ++i ) {
        _vecButtonsDisablable[i]->setDisabled(true);
    }
}

void EAQtMainWindow::disableButtonsAndTable()
{
    for ( int i = 0; i<_vecButtonsDisablable.size(); ++i ) {
        _vecButtonsDisablable[i]->setDisabled(true);
    }
    this->_tableCurveMain->setDisabled(true);
}

void EAQtMainWindow::enableAll()
{
    for ( int i = 0; i<_vecButtonsDisablable.size(); ++i ) {
        _vecButtonsDisablable[i]->setDisabled(false);
    }
    this->_tableCurveMain->setDisabled(false);
}

void EAQtMainWindow::userStartsMeasurement()
{
    this->_pEAQtData->MesStart(this->_pEAQtData->getWasLSV());
}

void EAQtMainWindow::MeasurementSetup()
{
    PlotRescaleAxes();
    this->_mouseHandler->ChangeMouseMode(EAQtMouseHandler::mm_measurement, EAQtMouseHandler::uf_none );
}

void EAQtMainWindow::MeasurementAfter()
{
    PlotRescaleAxes();
    this->_mouseHandler->ChangeMouseMode(EAQtMouseHandler::mm_normal, EAQtMouseHandler::uf_none);
}

void EAQtMainWindow::MeasurementUpdate(uint32_t curveNr, uint32_t pointNr)
{
    int i = 0;
    Curve* curve;
    curve = _pEAQtData->getMesCurves()->get(curveNr);
    setLowLabelText(0,tr("curve #: %1; point #: %2; potential: %3; current %4")
                                  .arg(curveNr)
                                  .arg(pointNr)
                                  .arg(_pEAQtData->dispE(curve->getPotentialVector()->at(pointNr)))
                                  .arg(_pEAQtData->dispI(curve->getCurrentVector()->at(pointNr))));

    switch ( this->_pEAQtData->getXAxis() ) {
    case XAXIS::potential:
        while ( (curve=this->_pEAQtData->getMesCurves()->get(i)) != NULL ) {
            curve->getPlot()->setData(
                        curve->getPotentialVector()->mid(0,curve->getNrOfDataPoints())
                        ,curve->getCurrentVector()->mid(0,curve->getNrOfDataPoints())
                        ,true
                      );
            ++i;
        }
        break;

    case XAXIS::time:
        while ( (curve=this->_pEAQtData->getMesCurves()->get(i)) != NULL ) {
            curve->getPlot()->setData(
                        curve->getTimeVector()->mid(0,curve->getNrOfDataPoints())
                        ,curve->getCurrentVector()->mid(0,curve->getNrOfDataPoints())
                        ,true
                      );
            ++i;
        }
        break;

    case XAXIS::nonaveraged:
        while ( (curve=this->_pEAQtData->getMesCurves()->get(i)) != NULL ) {
            if ( curve->getNumberOfProbingPoints() > 0 ) {
                curve->getPlot()->setData(
                            curve->getProbingDataPointNumbers()->mid(0,curve->getNumberOfProbingPoints())
                            ,curve->getProbingData()->mid(0,curve->getNumberOfProbingPoints())
                            ,true
                          );
            }
            ++i;
        }
        break;

    default:
        throw 1;
    }

    PlotRescaleAxes();
}

void EAQtMainWindow::showMessageBox(QString text, QString title)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setWindowTitle(title);
    msgBox.exec();
}

bool EAQtMainWindow::showQuestionBox(QString text, QString title, bool defaultIsYes)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    if ( defaultIsYes ) {
        msgBox.setDefaultButton(QMessageBox::Yes);
    } else {
        msgBox.setDefaultButton(QMessageBox::No);
    }
    if(msgBox.exec() == QMessageBox::Yes){
        return true;
    }else {
        return false;
    }
}

EAQtSaveFiledialog::SaveDetails EAQtMainWindow::DialogSaveInFile()
{
    static bool wasUsed = false;
    static EAQtSaveFiledialog::SaveDetails sd;
    if ( !wasUsed ) {
        wasUsed = true;
        sd.curveComment = "";
        sd.curveName = "";
        sd.wasCanceled = true;
        sd.fileName = "";
    }
    EAQtSaveFiledialog* sfd = new EAQtSaveFiledialog((QWidget*)this,sd.curveName,sd.curveComment,_PathInUse,sd.fileName);
    EAQtSaveFiledialog::SaveDetails nsd = sfd->getSaveDetails();
    if ( !nsd.wasCanceled ) {
        sd = nsd;
    }
    delete sfd;
    return nsd;
}

void EAQtMainWindow::changeStartButtonText(QString newText)
{
    this->_butStartMes->setText(newText);
}

void EAQtMainWindow::ComboOnAxisSelected(int v)
{
    this->_pEAQtData->setXAxis(v);
}

void EAQtMainWindow::paramCopy()
{
    Curve* c = _pEAQtData->getCurves()->get(_pEAQtData->Act());
    if ( c != NULL ) {
        if ( c->Param(PARAM::method) == PARAM::method_lsv ) {
            for ( int i=0; i<PARAM::PARAMNUM; ++i ) {
                this->_pEAQtData->ParamLSV(i,c->Param(i));
            }
        } else {
            for ( int i=0; i<PARAM::PARAMNUM; ++i ) {
                this->_pEAQtData->ParamPV(i,c->Param(i));
            }
        }
    }
}

void EAQtMainWindow::paramSave()
{
    //TODO:
    this->showMessageBox(tr("Not implemented"),tr("Not implemented"));
}

void EAQtMainWindow::paramLoad()
{
    //TODO:
    this->showMessageBox(tr("Not implemented"),tr("Not implemented"));
}

void EAQtMainWindow::createActionsTopMenu()
{
    this->_actSaveCurve = new QAction(tr("&Save curve"), this);
    _actSaveCurve->setShortcuts(QKeySequence::Save);
    _actSaveCurve->setStatusTip(tr("Add curve(s) to EAQt voltammogram file (.volt)"));
    connect(_actSaveCurve, SIGNAL(triggered(bool)), this, SLOT(saveCurve()));

    this->_actLoadCurve = new QAction(tr("&Load curve"), this);
    _actLoadCurve->setShortcut(QKeySequence::Open);
    _actLoadCurve->setStatusTip(tr("Load curve(s) from *.volt or *.vol files"));
    connect(_actLoadCurve, SIGNAL(triggered(bool)), this, SLOT(openFile()));

    this->_actExportCurve = new QAction(tr("&Export curve"), this);
    _actExportCurve->setStatusTip(tr("Export curve(s) as *.txt or *.csv"));
    connect(_actExportCurve, SIGNAL(triggered(bool)), this, SLOT(exportCurve()));

    this->_actStartMeasurement = new QAction(tr("Repeat previous (PV)"), this);
    this->_actStartMeasurement->setStatusTip(tr("Start previous measurement"));
    connect(_actStartMeasurement, SIGNAL(triggered(bool)),this,SLOT(userStartsMeasurement()));

    this->_actPVMeasurement = new QAction(tr("Settings - pulse techniques"), this);
    this->_actPVMeasurement->setStatusTip(tr("Setup and start PV measurement"));
    connect(_actPVMeasurement, SIGNAL(triggered(bool)),this,SLOT(showParamDialogPV()));

    this->_actLSVMeasurement = new QAction(tr("Settings - linear technique"), this);
    this->_actLSVMeasurement->setStatusTip(tr("Setup and start LSV measurement"));
    connect(_actLSVMeasurement, SIGNAL(triggered(bool)),this,SLOT(showParamDialogLSV()));

    this->_actAccessories = new QAction(tr("Accessories"), this);
    this->_actAccessories->setStatusTip(tr("Setup accessories"));
    connect(_actAccessories, SIGNAL(triggered(bool)),this,SLOT(showAccessoriesDialog()));

    this->_actCGMDEsettings = new QAction(tr("Test CGMDE"), this);
    this->_actCGMDEsettings->setStatusTip(tr("Test CMDE"));
    connect(_actCGMDEsettings, SIGNAL(triggered(bool)),this,SLOT(showTestCGMDE()));

    this->_actParamCopy = new QAction(tr("Copy settings"), this);
    this->_actParamCopy->setStatusTip(tr("Copy selected curve measurement parameteres"));
    _actParamCopy->setShortcuts(QKeySequence::Copy);
    connect(_actParamCopy, SIGNAL(triggered(bool)),this,SLOT(paramCopy()));

    this->_actParamLoad = new QAction(tr("Load settings"), this);
    this->_actParamLoad->setStatusTip(tr("Load measurement parameters from file (.vparam)"));
    connect(_actParamLoad, SIGNAL(triggered(bool)),this,SLOT(paramLoad()));

    this->_actParamSave = new QAction(tr("Save settings"), this);
    this->_actParamSave->setStatusTip(tr("Save current measurement parameteres to file"));
    connect(_actParamSave, SIGNAL(triggered(bool)),this,SLOT(paramSave()));

    this->_actDataCursor = new QAction(tr("Data cursor"), this);
    this->_actDataCursor->setStatusTip(tr("Show data cursor"));
    connect(_actDataCursor, SIGNAL(triggered(bool)),this,SLOT(showDataCursor()));

    this->_actCalibration = new QAction(tr("Calibration"), this);
    this->_actCalibration->setStatusTip(tr("Perform calibration / standard addition analysis"));
    connect(_actCalibration, SIGNAL(triggered(bool)),this,SLOT(startCalibration()));

    this->_actBkgCorrection = new QAction(tr("Fit background"), this);
    this->_actBkgCorrection->setStatusTip(tr("Perform background correction"));
    connect(_actBkgCorrection, SIGNAL(triggered(bool)),this,SLOT(startBackgroundCorrection()));

    this->_actRelativeValues = new QAction(tr("Relative values"), this);
    this->_actRelativeValues->setStatusTip(tr("Show relative to line values"));
    connect(_actRelativeValues, SIGNAL(triggered(bool)),this,SLOT(startRelativeValues()));

    this->_actMoveUpDown = new QAction(tr("Move curve"), this);
    this->_actMoveUpDown->setStatusTip(tr("Move curve up/down"));
    connect(_actMoveUpDown, SIGNAL(triggered(bool)),this,SLOT(startMoveUpDown()));

    this->_actSmooth = new QAction(tr("SG-Smooth"), this);
    this->_actSmooth->setStatusTip(tr("Savitzky golay smoothing"));
    connect(_actSmooth, SIGNAL(triggered(bool)),this,SLOT(startSmooth()));

    this->_actAdvSmooth = new QAction(tr("Advanced smoothing"), this);
    this->_actAdvSmooth->setStatusTip(tr("Advanced smoothing dialog"));
    connect(_actAdvSmooth, SIGNAL(triggered(bool)),this,SLOT(showAdvancedSmooth()));

    _actSoftware = new QAction(tr("Software"),this);
    _actSoftware->setStatusTip(tr("Show information about the software"));
    connect(_actSoftware,SIGNAL(triggered(bool)),this,SLOT(showAboutSoftware()));

    _actReportIssues = new QAction(tr("Report issues"),this);
    _actReportIssues->setStatusTip(tr("If you found any bugs or have some feature request report it on github"));
    connect(_actReportIssues,SIGNAL(triggered(bool)),this,SLOT(showGithubIssues()));

    _actSourceCode = new QAction(tr("Source code"),this);
    _actSourceCode->setStatusTip(tr("Source code for the program is available on github"));
    connect(_actSourceCode,SIGNAL(triggered(bool)),this,SLOT(showGithub()));
}

void EAQtMainWindow::createMenusTopMenu()
{
    _menuFile = this->menuBar()->addMenu(tr("&File"));
    _menuFile->addAction(this->_actLoadCurve);
    _menuFile->addAction(this->_actSaveCurve);
    _menuFile->addAction(this->_actExportCurve);

    _menuMeasurement = this->menuBar()->addMenu(tr("&Measurement"));
    _menuMeasurement->addAction(this->_actStartMeasurement);
    _menuMeasurement->addAction(this->_actPVMeasurement);
    _menuMeasurement->addAction(this->_actLSVMeasurement);
    _menuMeasurement->addAction(this->_actAccessories);
    _menuMeasurement->addAction(this->_actCGMDEsettings);
    _menuMeasurement->addAction(this->_actParamCopy);
    _menuMeasurement->addAction(this->_actParamLoad);
    _menuMeasurement->addAction(this->_actParamSave);

    _menuAnalysis = this->menuBar()->addMenu(tr("&Analysis"));
    _menuAnalysis->addAction(this->_actDataCursor);
    _menuAnalysis->addAction(this->_actCalibration);
    _menuAnalysis->addAction(this->_actBkgCorrection);
    _menuAnalysis->addAction(this->_actRelativeValues);
    _menuAnalysis->addAction(this->_actMoveUpDown);
    _menuAnalysis->addAction(this->_actSmooth);
    _menuAnalysis->addAction(this->_actAdvSmooth);

    _menuAbout = this->menuBar()->addMenu(tr("About"));
    _menuAbout->addAction(_actSoftware);
    _menuAbout->addAction(_actReportIssues);
    _menuAbout->addAction(_actSourceCode);
}

void EAQtMainWindow::startBackgroundCorrection()
{
    if ( _pEAQtData->getCurves()->count() > 0
    && _pEAQtData->Act() >= 0 ) {
        _mouseHandler->ChangeMouseMode(EAQtMouseHandler::mm_place4markers,
                                       EAQtMouseHandler::uf_background_correction);
    }
}

void EAQtMainWindow::startRelativeValues()
{
    if ( _pEAQtData->getCurves()->count() > 0
    && _pEAQtData->Act() >= 0 ) {
        _mouseHandler->ChangeMouseMode(EAQtMouseHandler::mm_relativeParamters,
                                   EAQtMouseHandler::uf_relative_height);
    }
}

void EAQtMainWindow::startMoveUpDown()
{
    if ( _pEAQtData->getCurves()->count() > 0
    && _pEAQtData->Act() >= 0 ) {
        _mouseHandler->ChangeMouseMode(EAQtMouseHandler::mm_nearestYonPlot,
                                   EAQtMouseHandler::uf_move_curve);
    }
}

void EAQtMainWindow::showAccessoriesDialog()
{
    EAQtAccessoriesDialog *ad = new EAQtAccessoriesDialog(this->_pEAQtData);
    ad->exec();
    delete ad;
}

void EAQtMainWindow::saveCurve()
{
    if ( _pEAQtData->getCurves()->count() > 0
    && this->_pEAQtData->Act() == SELECT::none ) {
        this->showMessageBox(tr("Cannot save - no curve(s) selected."));
        return;
    }
    QFileDialog *qfd = new QFileDialog();
    QString filter = "EAQt voltammograms file (*.volt)";
    QString savePath = qfd->getSaveFileName(this,tr("Add curve to file"),NULL,filter,&filter);
    if ( savePath.right(5).compare( ".volt", Qt::CaseInsensitive) != 0 ) {
        savePath.append(".volt");
    }
    delete qfd;
    if ( savePath.isEmpty() ) {
        return;
    }
    if ( this->_pEAQtData->Act() == SELECT::all ) {
        uint i = 0;
        int err;
        Curve* curve;
        while ( (curve=this->_pEAQtData->getCurves()->get(i)) != NULL ) {
            err = this->_pEAQtData->safeAppend(savePath, curve);
            if ( err != 0 ) {
                showMessageBox(tr("There was an error while saving the curves, please try again."),tr("Error."));
                return;
            }
            ++i;
        }
    } else {
        int act = _pEAQtData->Act();
        Curve* curve = _pEAQtData->getCurves()->get(act);
        if ( curve == NULL ) {
            showMessageBox(tr("Curve could not be selected for save, please try to select it again."),tr("Error"));
        }
        this->_pEAQtData->safeAppend(savePath, curve);
    }
    return;
}

void EAQtMainWindow::exportCurve()
{
    if ( _pEAQtData->getCurves()->count() > 0
    && this->_pEAQtData->Act() == SELECT::none ) {
        this->showMessageBox(tr("Cannot save - no curve(s) selected."));
        return;
    }
    QFileDialog *qfd = new QFileDialog();
    QString filter = "Text file (*.txt);;Comma separated values (.csv)";
    QString savePath = qfd->getSaveFileName(this,tr("Add curve to file"),NULL,filter,&filter);
    if (savePath.isEmpty() ) {
        return;
    }
    QFileInfo fi(savePath);
    if ( fi.isFile() ) {
        if ( !showQuestionBox(tr("This will overwrite existing file. Are you sure ?")
                             ,tr("Warning")
                             ,false) ) {
            return;
        }
    }
    if ( savePath.right(4).compare(".txt",Qt::CaseInsensitive) == 0 ) {
        _pEAQtData->exportToTXT(savePath);
    } else {
        _pEAQtData->exportToCSV(savePath);
    }
    delete qfd;
}

void EAQtMainWindow::setLowLabelText(int n, QString text)
{
    this->_vecLowText[n]->setText(text);
}

EAQtPlotCursor* EAQtMainWindow::PlotAddCursor()
{
    QCPItemStraightLine *sl = new QCPItemStraightLine(this->_plotMain);
    sl->setLayer(_plotLayers.Markers);
    QCPGraph *gf = PlotAddGraph();
    gf->setLayer(_plotLayers.Markers);
    return new EAQtPlotCursor(sl,gf);
}

double EAQtMainWindow::PlotGetXMiddle()
{
   QCPRange range = this->_plotMain->xAxis->range();
   return (range.upper - range.lower)/2 + range.lower;

}

void EAQtMainWindow::PlotSetInteraction(QCP::Interactions i)
{
    this->_plotMain->setInteractions(i);
}

QCPItemStraightLine* EAQtMainWindow::PlotAddLine()
{
    QCPItemStraightLine* sl = new QCPItemStraightLine(this->_plotMain);
    sl->setLayer(_plotLayers.Markers);
    return sl;
}

void EAQtMainWindow::setStatusText(QString str)
{
    this->ui->statusBar->setToolTip(str);
}

void EAQtMainWindow::showDataCursor()
{
    if ( _pEAQtData->getCurves()->count() > 0
    && this->_pEAQtData->Act() >= 0 ) {
        this->_mouseHandler->ChangeMouseMode(EAQtMouseHandler::mm_dataCursor,
                                         EAQtMouseHandler::uf_move_curve);
    }
}

void EAQtMainWindow::startCalibration()
{
    if ( _pEAQtData->getCurves()->count() > 1 ) {
        this->_mouseHandler->ChangeMouseMode(EAQtMouseHandler::mm_place2markers,
                                         EAQtMouseHandler::uf_calibration_data);
    }
}

void EAQtMainWindow::PlotReplot()
{
    _plotMain->replot();
}

void EAQtMainWindow::PlotRectangleZoom()
{
    _isRectangleZoom = true;
    _butZoom->setDown(true);
}

void EAQtMainWindow::showAboutSoftware()
{
    QFile f(":/README.md");
    f.open(QIODevice::ReadOnly);
    QTextStream stream(&f);
    QString content = stream.readAll();
    f.close();

    QVBoxLayout* vbl = new QVBoxLayout();
    QDialog *dialog = new QDialog();
    QLabel *l;
    l = new QLabel();
    //l->setTextFormat();
    l->setText(content);
    l->setFixedWidth(700);
    l->setWordWrap(true);
    vbl->addWidget(l);
    dialog->setLayout(vbl);
    dialog->exec();
}

void EAQtMainWindow::showGithubIssues()
{
    QString link = "https://github.com/efce/EAQt/issues";
    QDesktopServices::openUrl(QUrl(link));
}

void EAQtMainWindow::showGithub()
{
    QString link = "https://github.com/efce/EAQt";
    QDesktopServices::openUrl(QUrl(link));
}

void EAQtMainWindow::updateCurveInfo()
{
    Curve* c = _pEAQtData->getCurves()->get(_pEAQtData->Act());
    if ( c == NULL ) {
        _curveInfoData->setText("");
        return;
    }
    DisplayCurveInfo *dc = new DisplayCurveInfo(c);
    _curveInfoData->setText(dc->getHTMLInfo());
    delete dc;
}

void EAQtMainWindow::toggleCurveInfo(bool show)
{
    _curveInfoData->setVisible(show);
    if ( show ) {
        _butCurveInfoToggle->setText("<");
    } else {
        _butCurveInfoToggle->setText(">");

    }
}

void EAQtMainWindow::startSmooth()
{
    if ( _pEAQtData->getCurves()->count() > 0
    && this->_pEAQtData->Act() >= 0 ) {
        Curve* c = _pEAQtData->getCurves()->get(_pEAQtData->Act());
        //TODO change to Y vecotr !
        EAQtSignalProcessing::sgSmooth(c->getCurrentVector(),3,9);
        c->Param(PARAM::inf_smooth,PARAM::inf_smooth_yes);
        updateAll();
    }
}

EAQtUIInterface::PlotLayer* EAQtMainWindow::PlotGetLayers()
{
    return &_plotLayers;
}

void EAQtMainWindow::PlotSetInverted(bool inverted)
{
    _plotMain->xAxis->setRangeReversed(inverted);
}

void EAQtMainWindow::showAdvancedSmooth()
{
    std::vector<double> x;
    EAQtAdvancedSmoothDialog* adv = new EAQtAdvancedSmoothDialog(0,x);
    adv->exec();
    delete adv;
}

void EAQtMainWindow::showTestCGMDE()
{
    EAQtTestCGMDEDialog* tcd = new EAQtTestCGMDEDialog();
    tcd->exec();
    delete tcd;
}
