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

#include "thermal3danalysis.h"
#include "paintingmodule.h"
#include "thermalmodel.h"

#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QCursor>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QSplitter>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>

#include <QVTKWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkEventQtSlotConnect.h>

#include <vtkImageDataGeometryFilter.h>
#include <vtkImageToStructuredPoints.h>
#include <vtkStructuredGridSource.h>
#include <vtkStructuredGrid.h>
#include <vtkImageData.h>
#include <vtkImageReader.h>
#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkDataSetMapper.h>

using namespace Inspector::Internal;

//
// VtkPrivate
//
class Thermal3DAnalysis::VtkPrivate
{
public:
    VtkPrivate();
    ~VtkPrivate();

    void initPipeline(QObject *connTarget);
    void disposePipeline();

    void clearContents();
    void addRegularMesh(const Probe::RegularMeshRealData &, bool transp, bool smooth);

    QVTKWidget *widget;
    vtkRenderer *renderer;
    vtkEventQtSlotConnect *connections;

private:
    QList<vtkActor *> m_addedActors;
};

Thermal3DAnalysis::VtkPrivate::VtkPrivate()
  : widget(0)
  , renderer(0)
  , connections(0)
{
    widget = new QVTKWidget;
    widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    widget->setAttribute(Qt::WA_DontCreateNativeAncestors);
}

Thermal3DAnalysis::VtkPrivate::~VtkPrivate()
{
    if (renderer)
        disposePipeline();
}

void Thermal3DAnalysis::VtkPrivate::initPipeline(QObject *connTarget)
{
    // create a window to make it stereo capable and give it to QVTKWidget
    {
        vtkRenderWindow *renderWindow = vtkRenderWindow::New();
        renderWindow->StereoCapableWindowOn();
        widget->SetRenderWindow(renderWindow);
        renderWindow->Delete();
    }

    // create the Renderer and connect it to the RenderWindow
    renderer = vtkRenderer::New();
    widget->GetRenderWindow()->AddRenderer(renderer);

    // connect Vtk Events to Qt Slots
    connections = vtkEventQtSlotConnect::New();

    // get right mouse pressed with high priority
    connections->Connect(widget->GetRenderWindow()->GetInteractor(), vtkCommand::RightButtonPressEvent,
                         connTarget, SLOT(slotContextMenu(vtkObject*, unsigned long, void*, void*, vtkCommand*)),
                         0, 1.0);

    // update coords as we move through the window
    connections->Connect(widget->GetRenderWindow()->GetInteractor(), vtkCommand::MouseMoveEvent,
                         connTarget, SLOT(slotUpdateCoords(vtkObject*)));
}

void Thermal3DAnalysis::VtkPrivate::disposePipeline()
{
    if (renderer) {
        clearContents();
        renderer->Delete();
        renderer = 0;
    }
    if (connections) {
        connections->Delete();
        connections = 0;
    }
}

void Thermal3DAnalysis::VtkPrivate::clearContents()
{
    foreach (vtkActor *actor, m_addedActors) {
        actor->Delete();
        renderer->RemoveActor(actor);
    }
    m_addedActors.clear();
}

#include <vtkImageDataGeometryFilter.h>
#include <vtkWarpScalar.h>
#include <vtkProperty.h>
void Thermal3DAnalysis::VtkPrivate::addRegularMesh(const Probe::RegularMeshRealData &mesh, bool transp, bool smooth)
{
    if (!mesh.rows || !mesh.columns || mesh.data.isEmpty())
        return;

    // create image data
    vtkImageData *imageData = vtkImageData::New();
    imageData->SetDimensions(mesh.columns, mesh.rows, 1);
    imageData->SetOrigin(-(double)mesh.physicalSize.width() / 2.0, -(double)mesh.physicalSize.height() / 2.0, 0.0);
    imageData->SetNumberOfScalarComponents(1);
    imageData->SetScalarTypeToDouble();
    imageData->SetSpacing(
            (double)mesh.physicalSize.width() / (double)(mesh.columns - 1),
            (double)mesh.physicalSize.height() / (double)(mesh.rows - 1),
            1.0);

    int meshIdx = 0;
    for (int row = mesh.rows - 1; row >= 0; --row) {
        for (int col = 0; col < mesh.columns; ++col) {
            double* pixel = static_cast<double*>(imageData->GetScalarPointer(col, row, 0));
            pixel[0] = mesh.data[meshIdx++];
        }
    }

    // filter height
    vtkImageDataGeometryFilter *igFilter = vtkImageDataGeometryFilter::New();
    igFilter->SetInput(imageData);

    vtkWarpScalar *warp = vtkWarpScalar::New();
    warp->SetInput(igFilter->GetOutput());
    warp->SetNormal(0, 0, 1);
    warp->SetUseNormal(true);
    warp->SetXYPlane(false);
    warp->SetScaleFactor(255);

    vtkPolyDataNormals *normals = 0;
    if (smooth) {
        normals = vtkPolyDataNormals::New();
        normals->SetInput(warp->GetOutput());
    }

    vtkDataSetMapper *mapper = vtkDataSetMapper::New();
    if (normals)
        mapper->SetInput(normals->GetOutput());
    else
        mapper->SetInput(warp->GetOutput());

    vtkProperty *property = vtkProperty::New();
    property->SetOpacity(0.5);

    vtkActor* actor = vtkActor::New();
    if (transp)
        actor->SetProperty(property);
    actor->SetMapper(mapper);
    m_addedActors.append(actor);

    renderer->AddViewProp(actor);

    property->Delete();
    mapper->Delete();
    if (normals)
        normals->Delete();
    warp->Delete();
    igFilter->Delete();
    imageData->Delete();
}

//
// ThermalTreeWidgetItem
//
class Inspector::Internal::ThermalTreeWidgetItem : public QTreeWidgetItem
{
public:
    ThermalTreeWidgetItem(QTreeWidget *parent, const QString &string, int type);

    void setMesh(const Probe::RegularMeshRealData &mesh);
    Probe::RegularMeshRealData mesh() const;

private:
    Probe::RegularMeshRealData m_mesh;
};

ThermalTreeWidgetItem::ThermalTreeWidgetItem(QTreeWidget *parent, const QString &string, int type)
  : QTreeWidgetItem(parent, QStringList() << string, type)
{
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
    setData(0, Qt::SizeHintRole, QSize(150, 30));
}

void ThermalTreeWidgetItem::setMesh(const Probe::RegularMeshRealData &mesh)
{
    m_mesh = mesh;
    emitDataChanged();
}

Inspector::Probe::RegularMeshRealData ThermalTreeWidgetItem::mesh() const
{
    return m_mesh;
}

//
// DataSetTreeWidget
//
DataSetTreeWidget::DataSetTreeWidget(ThermalModel *sourceModel, QWidget *parent)
  : QTreeWidget(parent)
  , m_sourceModel(sourceModel)
{
    setEditTriggers(NoEditTriggers);
    setHeaderLabel(tr("Data sets"));
    setSelectionMode(ExtendedSelection);
    setVerticalScrollMode(ScrollPerPixel);

    // monitor source model for insertions so we can mirror them
    connect(m_sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsAdded(QModelIndex,int,int)));
    QModelIndex sourceIndex = m_sourceModel->resultsTableIndex();
    int initialRowCount = m_sourceModel->rowCount(sourceIndex);
    if (initialRowCount > 0)
        slotSourceRowsAdded(sourceIndex, 0, initialRowCount - 1);

    // monitor local items for changes (checkstates or mesh changes)
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*)));
}

QList<Inspector::Probe::RegularMeshRealData> DataSetTreeWidget::meshes() const
{
    QList<Inspector::Probe::RegularMeshRealData> meshList;

    QTreeWidgetItem *rootItem = invisibleRootItem();
    int firstLevelCount = rootItem->childCount();

    for (int idx = 0; idx < firstLevelCount; ++idx) {
        ThermalTreeWidgetItem *thermalItem = static_cast<ThermalTreeWidgetItem *>(rootItem->child(idx));
        if (thermalItem && thermalItem->checkState(0) != Qt::Unchecked)
            meshList.append(thermalItem->mesh());
    }

    return meshList;
}

void DataSetTreeWidget::slotSourceRowsAdded(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; ++row) {
        QModelIndex index = m_sourceModel->index(row, 0, parent);
        if (!index.isValid())
            continue;

        ThermalItem *item = static_cast<ThermalItem *>(m_sourceModel->itemFromIndex(index));
        if (!item)
            continue;

        // add an entry here to mirror the standarditem
        ThermalTreeWidgetItem *twi = new ThermalTreeWidgetItem(this, item->label(), DataOriginal);
        twi->setMesh(item->mesh());
    }
}

void DataSetTreeWidget::slotItemChanged(QTreeWidgetItem *treeItem)
{
    Q_UNUSED(treeItem)
    //ThermalTreeWidgetItem *item = static_cast<ThermalTreeWidgetItem *>(treeItem);
    emit changed();
}

//
// Thermal3DAnalysis
//
Thermal3DAnalysis::Thermal3DAnalysis(PaintingModule *parentModule, QWidget *parent)
  : QWidget(parent)
  , v(new VtkPrivate)
  , m_paintingModule(parentModule)
{
    QVBoxLayout *vLay = new QVBoxLayout(this);
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    vLay->addWidget(splitter);

    splitter->addWidget(v->widget);

    QWidget *rightPanel = new QWidget;
    QVBoxLayout *rLay = new QVBoxLayout(rightPanel);
    rLay->setMargin(0);

    m_dataSetWidget = new DataSetTreeWidget(parentModule->thermalModel());
    rLay->addWidget(m_dataSetWidget);

    m_smoothCheck = new QCheckBox(tr("Smoother display"));
    m_smoothCheck->setChecked(true);
    connect(m_smoothCheck, SIGNAL(toggled(bool)), this, SLOT(slotDataSetChanged()));
    rLay->addWidget(m_smoothCheck);

    m_coordLabel = new QLabel;
    m_coordLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    rLay->addWidget(m_coordLabel);

    splitter->addWidget(rightPanel);

    v->initPipeline(this);
    QColor backCol = QApplication::palette().color(QPalette::Window);
    v->renderer->SetBackground(backCol.redF(), backCol.greenF(), backCol.blueF());

    connect(m_dataSetWidget, SIGNAL(changed()), this, SLOT(slotDataSetChanged()));
    slotDataSetChanged();
}

void Thermal3DAnalysis::slotDataSetChanged()
{
    v->clearContents();
    //v->addTexturedPlane( ... );

    QList<Probe::RegularMeshRealData> meshes = m_dataSetWidget->meshes();
    foreach (const Probe::RegularMeshRealData &mesh, meshes)
        v->addRegularMesh(mesh, meshes.count() > 1, m_smoothCheck->isChecked());

    v->renderer->Render();
    v->widget->update();
}

Thermal3DAnalysis::~Thermal3DAnalysis()
{
    delete v;
}

void Thermal3DAnalysis::slotContextMenu(vtkObject *, unsigned long, void *, void *, vtkCommand *command)
{
    // consume event so the interactor style doesn't get it
    command->AbortFlagOn();

    // connect and display a popup menu
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Background White"))->setProperty("id", (int)1);
    contextMenu->addAction(tr("Background Black"))->setProperty("id", (int)2);
    contextMenu->addAction(tr("Stereo Rendering"))->setProperty("id", (int)3);
    connect(contextMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotContextAction(QAction*)));
    contextMenu->popup(QCursor::pos());
}

void Thermal3DAnalysis::slotContextAction(QAction *action)
{
    int id = action->property("id").toInt();
    if (v->renderer && id) {
        switch (id) {
        case 1: v->renderer->SetBackground(1, 1, 1); break;
        case 2: v->renderer->SetBackground(0, 0, 0); break;
        case 3: v->renderer->GetRenderWindow()->SetStereoRender(!v->renderer->GetRenderWindow()->GetStereoRender()); break;
        }
        v->widget->update();
    }
}

void Thermal3DAnalysis::slotUpdateCoords(vtkObject *object)
{
    vtkRenderWindowInteractor *interactor = vtkRenderWindowInteractor::SafeDownCast(object);
    int event_pos[2];
    interactor->GetEventPosition(event_pos);
    m_coordLabel->setText(tr("x=%1 : y=%2").arg(event_pos[0]).arg(event_pos[1]));
}
