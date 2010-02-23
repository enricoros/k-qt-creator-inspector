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
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QCursor>
#include <QtGui/QDockWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QRadioButton>
#include <QtGui/QSplitter>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>

#include <QVTKWidget.h>
#include <vtkDataSetMapper.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkImageData.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkWarpScalar.h>

#include <vtkLookupTable.h>
#include <vtkImageGaussianSmooth.h>

using namespace Inspector::Internal;

//
// VtkPrivate
//
class Inspector::Internal::VtkPrivate
{
public:
    VtkPrivate();
    ~VtkPrivate();

    void initPipeline(QObject *connTarget);
    void disposePipeline();

    void setBackground(const QColor &backColor);
    void toggleStereo();
    void refresh();

    void clearContents();
    void addRegularMesh(const Inspector::Probe::RegularMeshRealData &, int colorMode,
                        bool drawTransparent, bool drawSmooth, double smoothRadius = 0);

    QWidget *widget() const { return m_widget; }

private:
    QVTKWidget *m_widget;
    vtkRenderer *m_renderer;
    vtkEventQtSlotConnect *m_connections;
    QList<vtkActor *> m_addedActors;
};

VtkPrivate::VtkPrivate()
  : m_widget(0)
  , m_renderer(0)
  , m_connections(0)
{
    m_widget = new QVTKWidget;
    m_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_widget->setAttribute(Qt::WA_DontCreateNativeAncestors);
}

VtkPrivate::~VtkPrivate()
{
    if (m_renderer)
        disposePipeline();
}

void VtkPrivate::initPipeline(QObject *connTarget)
{
    // create a window to make it stereo capable and give it to QVTKWidget
    {
        vtkRenderWindow *renderWindow = vtkRenderWindow::New();
        renderWindow->StereoCapableWindowOn();
        m_widget->SetRenderWindow(renderWindow);
        renderWindow->Delete();
    }

    // create the Renderer and connect it to the RenderWindow
    m_renderer = vtkRenderer::New();
    m_widget->GetRenderWindow()->AddRenderer(m_renderer);

    // connect Vtk Events to Qt Slots
    m_connections = vtkEventQtSlotConnect::New();

    // get right mouse pressed with high priority
    m_connections->Connect(m_widget->GetRenderWindow()->GetInteractor(), vtkCommand::RightButtonPressEvent,
                         connTarget, SLOT(slotContextMenu(vtkObject*, unsigned long, void*, void*, vtkCommand*)),
                         0, 1.0);

    // update coords as we move through the window
    m_connections->Connect(m_widget->GetRenderWindow()->GetInteractor(), vtkCommand::MouseMoveEvent,
                         connTarget, SLOT(slotUpdateCoords(vtkObject*)));
}

void VtkPrivate::disposePipeline()
{
    if (m_renderer) {
        clearContents();
        m_renderer->Delete();
        m_renderer = 0;
    }
    if (m_connections) {
        m_connections->Delete();
        m_connections = 0;
    }
    delete m_widget;
    m_widget = 0;
}

void VtkPrivate::setBackground(const QColor &color)
{
    m_renderer->SetBackground(color.redF(), color.greenF(), color.blueF());
    refresh();
}

void VtkPrivate::toggleStereo()
{
    m_renderer->GetRenderWindow()->SetStereoRender(!m_renderer->GetRenderWindow()->GetStereoRender());
}

void VtkPrivate::refresh()
{
    if (!m_addedActors.isEmpty())
        m_renderer->ResetCamera();
    m_renderer->Render();
    m_widget->update();
}

void VtkPrivate::clearContents()
{
    foreach (vtkActor *actor, m_addedActors) {
        m_renderer->RemoveActor(actor);
        actor->Delete();
    }
    m_addedActors.clear();
}

void VtkPrivate::addRegularMesh(const Inspector::Probe::RegularMeshRealData &mesh,
                                int colorMode, bool drawTransparent, bool drawSmooth, double smoothRadius)
{
    if (!mesh.rows || !mesh.columns || mesh.data.isEmpty())
        return;

    // create the image data scaled to physical size
    vtkImageData *imageData = vtkImageData::New();
    imageData->SetDimensions(mesh.columns, mesh.rows, 1);
    imageData->SetOrigin(-(double)mesh.physicalSize.width() / 2.0, -(double)mesh.physicalSize.height() / 2.0, 0.0);
    imageData->SetNumberOfScalarComponents(1);
    imageData->SetScalarTypeToDouble();
    imageData->SetSpacing(
            (double)mesh.physicalSize.width() / (double)(mesh.columns - 1),
            (double)mesh.physicalSize.height() / (double)(mesh.rows - 1),
            1.0);
    // TODO: add RMS-normalization multiplier
    int meshIdx = 0;
    for (int row = mesh.rows - 1; row >= 0; --row) {
        for (int col = 0; col < mesh.columns; ++col) {
            double* pixel = static_cast<double*>(imageData->GetScalarPointer(col, row, 0));
            pixel[0] = mesh.data[meshIdx++] * 10;
        }
    }

    vtkImageGaussianSmooth *smooth = 0;
    if (smoothRadius > 0.1) {
        smooth = vtkImageGaussianSmooth::New();
        smooth->SetRadiusFactor(smoothRadius);
        smooth->SetInput(imageData);
    }

    vtkImageDataGeometryFilter *geometry = vtkImageDataGeometryFilter::New();
    geometry->SetInput(smooth ? smooth->GetOutput() : imageData);

    vtkWarpScalar *warp = vtkWarpScalar::New();
    warp->SetInput(geometry->GetOutput());
    warp->SetNormal(0, 0, 1);
    warp->SetUseNormal(true);
    warp->SetXYPlane(false);
    warp->SetScaleFactor(255);

    vtkPolyDataNormals *normals = 0;
    if (drawSmooth) {
        normals = vtkPolyDataNormals::New();
        normals->SetInput(warp->GetOutput());
    }
/*
    vtkElevationFilter *elevation = vtkElevationFilter::New();
    elevation->SetLowPoint(0, 0, 0);
    elevation->SetHighPoint(0, 0, 1000.0);
    elevation->SetScalarRange(0, 1);
    if (normals)
        elevation->SetInput(normals->GetOutput());
    else
        elevation->SetInput(warp->GetOutput());
*/
    vtkDataSetMapper *mapper = vtkDataSetMapper::New();
    if (normals)
        mapper->SetInput(normals->GetOutput());
    else
        mapper->SetInput(warp->GetOutput());

    {
        vtkLookupTable *lut = vtkLookupTable::New();
        lut->SetRange(0, 1);
        lut->SetHueRange(0.667, 0);
        lut->SetSaturationRange(colorMode == 1 ? 0 : 1, 1);
        lut->SetValueRange(1, 1);
        lut->SetAlphaRange(colorMode == 1 ? 0 : 1, 1);
        lut->Build();
        mapper->SetLookupTable(lut);
        lut->Delete();
    }

    vtkProperty *property = vtkProperty::New();
    property->SetOpacity(0.5);

    vtkActor* actor = vtkActor::New();
    if (drawTransparent)
        actor->SetProperty(property);
    actor->SetMapper(mapper);

    m_renderer->AddViewProp(actor);
    m_addedActors.append(actor);

    property->Delete();
    mapper->Delete();
    if (normals)
        normals->Delete();
    warp->Delete();
    geometry->Delete();
    if (smooth)
        smooth->Delete();
    imageData->Delete();
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

    splitter->addWidget(v->widget());

    QWidget *rightPanel = new QWidget;
    QVBoxLayout *rLay = new QVBoxLayout(rightPanel);
    rLay->setMargin(0);

    m_dataSetWidget = new DataSetTreeWidget(parentModule->thermalModel());
    rLay->addWidget(m_dataSetWidget);

    QGroupBox *styleGroup = new QGroupBox(tr("Surface Color"));
    QVBoxLayout *styleLay = new QVBoxLayout(styleGroup);
    m_colorGroup = new QButtonGroup(styleGroup);
    connect(m_colorGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotDataSetChanged()));
    QRadioButton *color1 = new QRadioButton(tr("Default"));
    color1->setProperty("scheme", (int)0);
    color1->setChecked(true);
    m_colorGroup->addButton(color1);
    styleLay->addWidget(color1);
    QRadioButton *color2 = new QRadioButton(tr("Variation"));
    color2->setProperty("scheme", (int)1);
    m_colorGroup->addButton(color2);
    styleLay->addWidget(color2);
    rLay->addWidget(styleGroup);

    m_smoothCheck = new QCheckBox(tr("Smoother display"));
    m_smoothCheck->setChecked(true);
    connect(m_smoothCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotDataSetChanged()));
    rLay->addWidget(m_smoothCheck);

    m_filterCheck = new QCheckBox(tr("Filter display"));
    m_filterCheck->setChecked(true);
    connect(m_filterCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotDataSetChanged()));
    rLay->addWidget(m_filterCheck);

    m_coordLabel = new QLabel;
    m_coordLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    rLay->addWidget(m_coordLabel);

    splitter->addWidget(rightPanel);

    v->initPipeline(this);
    v->setBackground(QApplication::palette().color(QPalette::Window));

    connect(m_dataSetWidget, SIGNAL(changed()), this, SLOT(slotDataSetChanged()));
    slotDataSetChanged();
}

void Thermal3DAnalysis::slotDataSetChanged()
{
    v->clearContents();
    //v->addTexturedPlane( ... );

    QAbstractButton *colorButton = m_colorGroup->checkedButton();
    int colorMode = colorButton ? colorButton->property("scheme").toInt() : 0;

    QList<Probe::RegularMeshRealData> meshes = m_dataSetWidget->meshes();
    foreach (const Probe::RegularMeshRealData &mesh, meshes)
        v->addRegularMesh(mesh, colorMode, meshes.count() > 1, m_smoothCheck->isChecked(),
                          m_filterCheck->isChecked() ? 5 : 0);

    v->refresh();
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
    switch (id) {
    case 1: v->setBackground(Qt::white); break;
    case 2: v->setBackground(Qt::black); break;
    case 3: v->toggleStereo(); break;
    }
}

void Thermal3DAnalysis::slotUpdateCoords(vtkObject *object)
{
    vtkRenderWindowInteractor *interactor = vtkRenderWindowInteractor::SafeDownCast(object);
    int event_pos[2];
    interactor->GetEventPosition(event_pos);
    m_coordLabel->setText(tr("x=%1 : y=%2").arg(event_pos[0]).arg(event_pos[1]));
}

//
// DataSetTreeItem
//
class Inspector::Internal::DataSetTreeItem : public QTreeWidgetItem
{
public:
    DataSetTreeItem(QTreeWidget *parent, const QString &string, int type);

    void setMesh(const Probe::RegularMeshRealData &mesh);
    Probe::RegularMeshRealData mesh() const;

private:
    Probe::RegularMeshRealData m_mesh;
};

DataSetTreeItem::DataSetTreeItem(QTreeWidget *parent, const QString &string, int type)
  : QTreeWidgetItem(parent, QStringList() << string, type)
{
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
    setData(0, Qt::SizeHintRole, QSize(150, 30));
}

void DataSetTreeItem::setMesh(const Probe::RegularMeshRealData &mesh)
{
    m_mesh = mesh;
    emitDataChanged();
}

Inspector::Probe::RegularMeshRealData DataSetTreeItem::mesh() const
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
        DataSetTreeItem *thermalItem = static_cast<DataSetTreeItem *>(rootItem->child(idx));
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
        DataSetTreeItem *twi = new DataSetTreeItem(this, item->label(), DataOriginal);
        twi->setMesh(item->mesh());
    }
}

void DataSetTreeWidget::slotItemChanged(QTreeWidgetItem *treeItem)
{
    Q_UNUSED(treeItem)
    //DataSetTreeItem *item = static_cast<DataSetTreeItem *>(treeItem);
    emit changed();
}
