#include "eaqtcurverenamedialog.h"

EAQtCurveRenameDialog::EAQtCurveRenameDialog(Curve *c) : QObject()
{
    _curve = c;
    _dialog = new QDialog();
    _dialog->setWindowTitle(tr("Edit curve name/comment"));
    _butOK = new QPushButton(tr("OK"));
    connect(_butOK,SIGNAL(clicked(bool)),this,SLOT(OK()));
    _butCancel = new QPushButton(tr("Cancel"));
    connect(_butCancel,SIGNAL(clicked(bool)),_dialog,SLOT(close()));
    _leName = new QLineEdit();
    _leName->setText(_curve->CName());
    QLabel *lName = new QLabel(tr("Name:"));
    _pteComment = new QPlainTextEdit();
    _pteComment->document()->setPlainText(_curve->Comment());
    QLabel *lComment = new QLabel(tr("Comment:"));

    QGridLayout *gl = new QGridLayout();
    gl->addWidget(lName,0,0,1,2);
    gl->addWidget(_leName,1,0,1,2);
    gl->addWidget(lComment, 2,0,1,2);
    gl->addWidget(_pteComment,3,0,1,2);
    gl->addWidget(_butOK,4,0,1,1);
    gl->addWidget(_butCancel,4,1,1,1);
    _dialog->setLayout(gl);
}

EAQtCurveRenameDialog::~EAQtCurveRenameDialog()
{
    delete _dialog;
}

void EAQtCurveRenameDialog::exec()
{
    _dialog->exec();
}

void EAQtCurveRenameDialog::OK()
{
   QString newName = _leName->text();
   QString newComment = _pteComment->document()->toPlainText();
   _curve->CName(newName);
   _curve->Comment(newComment);
   _curve->FName(tr("(not saved)"));
   _dialog->close();
}
