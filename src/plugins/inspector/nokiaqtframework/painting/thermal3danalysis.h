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
class QCheckBox;
class vtkCommand;
class vtkObject;

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
    Thermal3DAnalysis(PaintingModule *, bool useDepthPeeling, QWidget *parent = 0);
    ~Thermal3DAnalysis();

private slots:
    void slotRefreshRendering();
    void slotContextMenu(vtkObject *obj, unsigned long, void *client_data, void *, vtkCommand *command);
    void slotContextAction(QAction *);

private:
    VtkPrivate *v;
    PaintingModule *m_paintingModule;
    DataSetTreeWidget *m_dataSetWidget;
    QCheckBox *m_altColorsCheck;
    QCheckBox *m_smoothCheck;
};

class DataSetTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    DataSetTreeWidget(ThermalModel *sourceModel, QWidget *parent = 0);

    void render(VtkPrivate *, int colorMode, bool smoothing) const;

public slots:
    void slotAppendFiltered();
    void slotRemoveSelected();
    void slotColorizeSelected();

signals:
    void changed();
    void itemSelected(bool);
    void topItemSelected(bool);

private slots:
    void slotSourceRowsAdded(const QModelIndex &,int,int);
    void slotSourceRowsRemoved(const QModelIndex &,int,int);
    void slotItemSelectionChanged();

private:
    enum DataItemType {
        DataOriginal = QTreeWidgetItem::UserType + 1,
        DataFiltered = DataOriginal + 1
    };
    ThermalModel *m_sourceModel;
};

} // namespace Internal
} // namespace Inspector

#endif // THERMAL3DANALYSIS_H
