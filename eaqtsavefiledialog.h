
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
#ifndef EAQTSAVEFILEDIALOG_H
#define EAQTSAVEFILEDIALOG_H

#include <QString>
#include <QWidget>
#include <QFileDialog>
#include <QLineEdit>
#include <QPlainTextEdit>


class EAQtSaveFiledialog : public QObject
{
    Q_OBJECT

public:
    EAQtSaveFiledialog(QWidget* parent, QString cname, QString ccomment, QString pathToShow = "", QString file = "");
    struct SaveDetails {
        bool wasCanceled;
        QString fileName;
        QString curveName;
        QString curveComment;
    };
    SaveDetails getSaveDetails(bool allowFotNameChange = true, bool isExport = false);
private:
    SaveDetails _saveDetails;
    QWidget* _parent;
    QString _pathToShow;
    QLineEdit* _leCurveName;
    QPlainTextEdit* _leCurveComment;
    QString _cname;
    QString _ccomment;
    QString _filename;
    QString _defFilter;
    QFileDialog* _fd;
public slots:
    void updateCurveName(QString);
    void updateCurveComment();
    void updateFileName(QString);
    void setSuffix(QString);
    void setSuffixExport(QString);
    void wasCanceled();
};

#endif // EAQTSAVEFILEDIALOG_H
