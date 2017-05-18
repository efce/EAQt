#include "eaqtaveragedialog.h"
#include "curvecollection.h"
#include "eaqtdata.h"

EAQtAverageDialog::EAQtAverageDialog(EAQtUIInterface* pui) : QObject()
{
    CurveCollection* cc = EAQtData::getInstance().getCurves();
    if ( cc->count() <= 0 ) {
        //TODO: error
        return;
    }
    _pUI = pui;
    _dialog = new QDialog();
    _dialog->setWindowTitle(tr("Average curves"));
    QGridLayout *gl = new QGridLayout();
    _butAverage = new QPushButton(tr("Average selected"));
    connect(_butAverage,SIGNAL(clicked(bool)),this,SLOT(average()));
    _butClose = new QPushButton(tr("Close"));
    connect(_butClose,SIGNAL(clicked(bool)),_dialog,SLOT(close()));
    _butSelectAll = new QPushButton(tr("Select all"));
    connect(_butSelectAll,SIGNAL(clicked(bool)),this,SLOT(selectAll()));
    _butSelectNone = new QPushButton(tr("Select none"));
    connect(_butSelectNone,SIGNAL(clicked(bool)),this,SLOT(selectNone()));

    _listBox = new QVBoxLayout();
    //_listBox->setMargin(1);
    _listBox->addStretch(0);
    _sa = new QScrollArea();
    _scrollAreaWidget = new QWidget();
    _scrollAreaWidget->setFixedWidth(250);
    _scrollAreaWidget->setLayout(_listBox);
    _sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _sa->setFixedWidth(400);
    _sa->setMaximumHeight(600);
    _sa->setLayout(new QVBoxLayout);
    _sa->setWidget(_scrollAreaWidget);
    _sa->setWidgetResizable(true);

    generateList();

    gl->addWidget(_sa,0,0,4,1);
    gl->addWidget(_butAverage,0,1,1,1);
    gl->addWidget(_butSelectAll,1,1,1,1);
    gl->addWidget(_butSelectNone,2,1,1,1);
    gl->addWidget(_butClose,3,1,1,1);
    _dialog->setLayout(gl);
}

EAQtAverageDialog::~EAQtAverageDialog()
{
   delete _dialog;
}

void EAQtAverageDialog::exec()
{
    _dialog->exec();
}

void EAQtAverageDialog::generateList()
{
    CurveCollection* cc = EAQtData::getInstance().getCurves();
    if ( _list.size() > 0 ) {
        for ( int i =0; i<_list.size(); ++i ) {
            _listBox->removeWidget(_list[i]);
            delete _list[i];
        }
    }
    _list.clear();
    _list.resize(cc->count());
    for ( uint i=0; i<cc->count(); ++i) {
        _list[i] = new QCheckBox( cc->get(i)->CName() );
        _list[i]->setChecked(false);
        _listBox->addWidget(_list[i]);
    }
    _listBox->update();
    _scrollAreaWidget->update();
    _sa->layout()->update();
    _sa->update();
}

void EAQtAverageDialog::selectAll()
{
    if ( _list.size() > 0 ) {
        for ( int i=0; i<_list.size(); ++i) {
            _list[i]->setChecked(true);
        }
    }
}

void EAQtAverageDialog::selectNone()
{

    if ( _list.size() > 0 ) {
        for ( int i=0; i<_list.size(); ++i) {
            _list[i]->setChecked(false);
        }
    }
}

void EAQtAverageDialog::average()
{
    if ( _list.size() > 0 ) {
        CurveCollection* cc = EAQtData::getInstance().getCurves();
        QVector<uint32_t> toAvg(0);
        uint32_t dataSize = 0;
        uint32_t nonavgSize = 0;
        bool error = false;
        bool avgNonavg = true;
        for ( int i =0; i<_list.size(); ++i ) {
            if ( _list[i]->isChecked() ) {
                if ( dataSize == 0 ) {
                    dataSize = cc->get(i)->getNrOfDataPoints();
                    nonavgSize = cc->get(i)->getNumberOfProbingPoints();
                }
                if ( dataSize != cc->get(i)->getNrOfDataPoints() ) {
                    error = true;
                    break;
                }
                if ( nonavgSize != cc->get(i)->getNumberOfProbingPoints() ) {
                    avgNonavg = false;
                }
                toAvg.push_back(i);
            }
            if ( nonavgSize == 0 ) {
                avgNonavg = false;
            }
            if ( avgNonavg == false ) {
                nonavgSize = 0;
            }
        }
        if ( !error && toAvg.size() > 1 ) {
            QVector<double> current(dataSize,0.0);
            QVector<double> nonavg(nonavgSize,0.0);
            for ( int i = 0; i<toAvg.size(); ++i ) {
                QVector<double> *cv = cc->get(toAvg[i])->getCurrentVector();
                for ( uint32_t ii=0; ii<dataSize; ++ii) {
                    current[ii] += cv->at(ii);
                }
            }
            for ( int i = 0; i<dataSize; ++i ) {
                current[i] /= toAvg.size();
            }
            if ( avgNonavg ) {
                for ( int i = 0; i<toAvg.size(); ++i ) {
                    QVector<double> *cv = cc->get(toAvg[i])->getProbingData();
                    for ( uint32_t ii=0; ii<nonavgSize; ++ii) {
                        nonavg[ii] += cv->at(ii);
                    }
                }
                for ( int i = 0; i<nonavgSize; ++i ) {
                    nonavg[i] /= toAvg.size();
                }
            }
            Curve* avgCurve = new Curve(cc->get(toAvg[0]));
            avgCurve->FName(tr("(not saved)"));
            avgCurve->CName(avgCurve->CName() + "(AVG)");
            for ( uint32_t i =0; i<dataSize; ++i) {
                avgCurve->Result(i,current[i]);
            }
            if ( avgNonavg ) {
                avgCurve->setProbingData(nonavg);
            }
            for ( int i =toAvg.size()-1; i>=0; --i) {
                cc->remove(toAvg[i]);
            }
            avgCurve->setPlot(_pUI->PlotAddGraph());
            avgCurve->changeToRegularPlot();
            cc->append(avgCurve);
        } else {
            showError();
        }
        generateList();
        EAQtData::getInstance().Act(SELECT::none);
        _pUI->updateAll(false);
    } else {
        showError();
    }
}

void EAQtAverageDialog::showError()
{
    _pUI->showMessageBox(tr("Could not average selected curves"), tr("Error"));
}
