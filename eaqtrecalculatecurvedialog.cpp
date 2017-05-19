#include "eaqtrecalculatecurvedialog.h"
#include "eaqtdata.h"

EAQtRecalculateCurveDialog::EAQtRecalculateCurveDialog(EAQtUIInterface *pui) : QObject()
{
    _wasInitialized = false;
    _pUI = pui;
    CurveCollection* cc = EAQtData::getInstance().getCurves();
    int curveCount = cc->count();

    if ( EAQtData::getInstance().Act() == SELECT::none
    || EAQtData::getInstance().Act() > curveCount
    || curveCount < 1 ) {
        _pUI->showMessageBox(tr("Cannot select curves for recalculation."),tr("Error"));
        return;
    }

    _tpplustw = 0;

    if ( EAQtData::getInstance().Act() == SELECT::all  ) {
        bool allHaveNonAvg = true;
        _tpplustw = cc->get(0)->Param(PARAM::tp) + cc->get(0)->Param(PARAM::tw);
        for ( uint32_t i =0; i<curveCount; ++i ) {
            if ( cc->get(i)->getNumberOfProbingPoints() == 0 ) {
                allHaveNonAvg = false;
            }
            if ( _tpplustw != cc->get(i)->Param(PARAM::tp) + cc->get(i)->Param(PARAM::tw) ) {
                allHaveNonAvg = false;
            }
        }

        if ( !allHaveNonAvg ) {
            //TODO: error
            return;
        }

        _leTP = new QLineEdit();
        _leTP->setValidator(new QIntValidator(1,_tpplustw));
        _leTW = new QLineEdit();
        _leTW->setValidator(new QIntValidator(0,_tpplustw));

    } else {
        Curve *c = cc->get(EAQtData::getInstance().Act());
        if ( c == NULL || c->getNumberOfProbingPoints() == 0 ) {
            //TODO: error
            return;
        }
        _tpplustw = c->Param(PARAM::tp) + c->Param(PARAM::tw);

        _leTP = new QLineEdit();
        _leTP->setValidator(new QIntValidator(0,_tpplustw));
        _leTW = new QLineEdit();
        _leTW->setValidator(new QIntValidator(0,_tpplustw));
    }

    _dialog = new QDialog();
    QLabel *lDesc = new QLabel(tr("tp + tw has to be lower than: %1 ms").arg(_tpplustw));
    QLabel *lTp = new QLabel(tr("New tp value [ms]: "));
    QLabel *lTw = new QLabel(tr("New tw value [ms]: "));
    _butClose = new QPushButton(tr("Close"));
    connect(_butClose,SIGNAL(clicked(bool)),_dialog,SLOT(close()));
    _butRecalculate = new QPushButton(tr("Recalculate"));
    connect(_butRecalculate,SIGNAL(clicked(bool)),this,SLOT(recalculate()));
    QGridLayout *gl = new QGridLayout();
    gl->addWidget(lDesc,0,0,1,2);
    gl->addWidget(lTp,1,0,1,1);
    gl->addWidget(_leTP,1,1,1,1);
    gl->addWidget(lTw,2,0,1,1);
    gl->addWidget(_leTW,2,1,1,1);
    gl->addWidget(_butRecalculate,3,0,1,1);
    gl->addWidget(_butClose,3,1,1,1);
    _dialog->setLayout(gl);
    _wasInitialized = true;
}

EAQtRecalculateCurveDialog::~EAQtRecalculateCurveDialog()
{
    if ( _wasInitialized ) {
        delete _dialog;
    }
}

void EAQtRecalculateCurveDialog::exec()
{
    if ( _wasInitialized ) {
        _dialog->exec();
    }
}

void EAQtRecalculateCurveDialog::recalculate()
{
    int newTp = _leTP->text().toInt();
    int newTw = _leTW->text().toInt();
    if ( (newTp+newTw) > _tpplustw ) {
        _pUI->showMessageBox(tr("tp + tw has to be lower than %1 ms").arg(_tpplustw),tr("Warning"));
        return;
    }
    if ( EAQtData::getInstance().Act() == SELECT::all ) {
        CurveCollection *cc = EAQtData::getInstance().getCurves();
        for ( uint32_t nc = 0; nc<cc->count(); ++nc ) {
            Curve *c = EAQtData::getInstance().getCurves()->get(nc);
            recalculateCurve(c,newTp,newTw);
            c->FName(tr("not saved)"));
        }
    } else {
        Curve *c = EAQtData::getInstance().getCurves()->get(EAQtData::getInstance().Act());
        recalculateCurve(c,newTp,newTw);
        c->FName(tr("not saved)"));
    }
    _pUI->updateAll(true);
}

void EAQtRecalculateCurveDialog::recalculateCurve(Curve *c, int tp, int tw)
{
    double wrk = 0;
    QVector<double>* v = c->getProbingData();
    for ( uint32_t i =0; i<c->getNrOfDataPoints(); ++i ) {
        switch (c->Param(PARAM::method)) {
        case PARAM::method_dpv:
        case PARAM::method_npv:
            for ( uint32_t p=0; p<c->getNrOfDataPoints(); ++p ) {
                wrk = 0;
                for ( int i=0; i<tp; ++i ) {
                    wrk += v->at(i+tw+_tpplustw+(2*_tpplustw*p));
                    wrk -= v->at(i+tw+(2*_tpplustw*p));
                }
                wrk /= (double)tp;
                c->Result(p,wrk);
            }
            break;
        default:
            _pUI->showMessageBox(tr("Only supported methods are DPV and NPV"), tr("Error"));
            break;
        }

    }
}
