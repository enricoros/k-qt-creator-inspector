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

#ifndef THERMAL3DANALYSIS_H
#define THERMAL3DANALYSIS_H

#include <QtCore/QModelIndex>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>
class QButtonGroup;
class QCheckBox;
class QDockWidget;
class QLabel;
class QStandardItem;
class QVBoxLayout;
class vtkCommand;
class vtkObject;

// include Probe data types
#include "../../../../share/qtcreator/gdbmacros/probedata.h"

namespace Inspector {
namespace Internal {

class DataSetTreeWidget;
class DataSetTreeItem;
class ThermalModel;
class PaintingModule;
class VtkPrivate;


class Thermal3DAnalysis : public QWidget
{
    Q_OBJECT

public:
    Thermal3DAnalysis(PaintingModule *, QWidget *parent = 0);
    ~Thermal3DAnalysis();

private slots:
    void slotDataSetChanged();

    void slotContextMenu(vtkObject *obj, unsigned long, void *client_data, void *, vtkCommand *command);
    void slotContextAction(QAction *);
    void slotUpdateCoords(vtkObject *);

private:
    VtkPrivate *v;
    PaintingModule *m_paintingModule;
    DataSetTreeWidget *m_dataSetWidget;
    QButtonGroup *m_colorGroup;
    QCheckBox *m_smoothCheck;
    QCheckBox *m_filterCheck;
    QLabel *m_coordLabel;
};

class DataSetTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    DataSetTreeWidget(ThermalModel *sourceModel, QWidget *parent = 0);

    QList<Probe::RegularMeshRealData> meshes() const;

signals:
    void changed();

private slots:
    void slotSourceRowsAdded(const QModelIndex &,int,int);
    void slotSourceRowsRemoved(const QModelIndex &,int,int);
    void slotItemChanged(QTreeWidgetItem*);

private:
    enum DataItemType {
        DataOriginal = QTreeWidgetItem::UserType + 1,
        //DataFiltered = DataOriginal + 1,
        //DataCombined = DataFiltered + 1,
    };
    ThermalModel *m_sourceModel;
};

} // namespace Internal
} // namespace Inspector

#endif // THERMAL3DANALYSIS_H
