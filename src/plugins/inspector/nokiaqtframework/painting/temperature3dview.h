/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009-2010 Enrico Ros
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#ifndef TEMPERATURE3DVIEW_H
#define TEMPERATURE3DVIEW_H

#include <QListView>
#include <QWidget>
class QLabel;
class vtkCommand;
class vtkObject;

namespace Inspector {
namespace Internal {

class DataSetTreeView;
class PaintingModel;
class PaintingModule;

class Temperature3DView : public QWidget
{
    Q_OBJECT

public:
    Temperature3DView(PaintingModule *parentModule, QWidget *parent = 0);
    ~Temperature3DView();

private slots:
    void slotContextMenu(vtkObject *obj, unsigned long, void *client_data,
                         void *, vtkCommand *command);
    void slotContextAction(QAction *);
    void slotUpdateCoords(vtkObject *);

private:
    class VtkPrivate;
    VtkPrivate *v;

    PaintingModule *m_paintingModule;
    DataSetTreeView *m_dataSetView;
    QLabel *m_coordLabel;
};

class DataSetTreeView : public QListView
{
    Q_OBJECT

public:
    DataSetTreeView(PaintingModel *model, QWidget *parent = 0);


};

} // namespace Internal
} // namespace Inspector

#endif // TEMPERATUREPANEL_H
