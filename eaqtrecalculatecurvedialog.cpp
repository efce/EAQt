#include "eaqtrecalculatecurvedialog.h"
#include "eaqtdata.h"

EAQtRecalculateCurveDialog::EAQtRecalculateCurveDialog(EAQtUIInterface *pui) : QObject()
{
    _pUI = pui;
    CurveCollection* cc = EAQtData::getInstance().getCurves();
    if ( EAQtData::getInstance().Act() == SELECT::none
    || EAQtData::getInstance().Act() > cc->count()
    || cc->count() < 1 ) {
        //TODO: error
        return;
    }

    _tpplustw = 0;

    if ( EAQtData::getInstance().Act() == SELECT::all  ) {
        bool allHaveNonAvg = true;
        _tpplustw = cc->get(0)->Param(PARAM::tp) + cc->get(0)->Param(PARAM::tw);
        for ( uint32_t i =0; i<cc->count(); ++i ) {
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
        _leTP->setValidator(new QIntValidator(0,_tpplustw));
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
}

EAQtRecalculateCurveDialog::~EAQtRecalculateCurveDialog()
{
    delete _dialog;
}

void EAQtRecalculateCurveDialog::exec()
{
    _dialog->exec();
}

void EAQtRecalculateCurveDialog::recalculate()
{
    int newTp = _leTP->text().toInt();
    int newTw = _leTW->text().toInt();
    if ( (newTp+newTw) > _tpplustw ) {
        //TODO: error
        return;
    }
    if ( EAQtData::getInstance().Act() == SELECT::all ) {
        CurveCollection *cc = EAQtData::getInstance().getCurves();
        for ( uint32_t nc = 0; nc<cc->count(); ++nc ) {
            Curve *c = EAQtData::getInstance().getCurves()->get(nc);
            double wrk = 0;
            QVector<double>* v = c->getProbingData();
            for ( uint32_t i =0; i<c->getNrOfDataPoints(); ++i ) {
                wrk = 0;
                for ( int ii=0; ii<newTp; ++ii ) {
                    wrk += v->at(ii+newTw+_tpplustw*i);
                }
                wrk /= newTp;
                c->Result(i,wrk);
            }
        }
    } else {
        Curve *c = EAQtData::getInstance().getCurves()->get(EAQtData::getInstance().Act());
        double wrk = 0;
        QVector<double>* v = c->getProbingData();
        for ( uint32_t i =0; i<c->getNrOfDataPoints(); ++i ) {
            wrk = 0;
            for ( int ii=0; ii<newTp; ++ii ) {
                wrk += v->at(ii+newTw+_tpplustw*i);
            }
            wrk /= newTp;
            c->Result(i,wrk);
        }
    }
    _pUI->updateAll(false);
}
