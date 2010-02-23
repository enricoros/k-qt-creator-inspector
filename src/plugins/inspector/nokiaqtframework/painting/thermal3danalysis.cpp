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
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QVBoxLayout>

#include <QVTKWidget.h>
#include <vtkDataSetMapper.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkImageData.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkWarpScalar.h>

// include Probe data types
#include "../../../../share/qtcreator/gdbmacros/probedata.h"

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

    QSplitter *rightSplitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(rightSplitter);

    m_dataSetWidget = new DataSetTreeWidget(parentModule->thermalModel());
    rightSplitter->addWidget(m_dataSetWidget);

    QWidget *optionsPanel = new QWidget;
    QVBoxLayout *oLay = new QVBoxLayout(optionsPanel);
    oLay->setMargin(0);

    QPushButton *removeButton = new QPushButton(tr("Remove Selected"));
    removeButton->setEnabled(false);
    connect(m_dataSetWidget, SIGNAL(itemSelected(bool)),
            removeButton, SLOT(setEnabled(bool)));
    connect(removeButton, SIGNAL(clicked()),
            m_dataSetWidget, SLOT(slotRemoveSelected()));
    oLay->addWidget(removeButton);

    QPushButton *filterButton = new QPushButton(tr("Add Filtered"));
    filterButton->setEnabled(false);
    connect(m_dataSetWidget, SIGNAL(topItemSelected(bool)),
            filterButton, SLOT(setEnabled(bool)));
    connect(filterButton, SIGNAL(clicked()),
            m_dataSetWidget, SLOT(slotAppendFiltered()));
    oLay->addWidget(filterButton);

    QGroupBox *styleGroup = new QGroupBox(tr("Surface Options"));
    QVBoxLayout *styleLay = new QVBoxLayout(styleGroup);
    m_altColorsCheck = new QCheckBox(tr("Alternate Colors"));
    connect(m_altColorsCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotRefreshRendering()));
    styleLay->addWidget(m_altColorsCheck);
    m_smoothCheck = new QCheckBox(tr("Smoother display"));
    connect(m_smoothCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotRefreshRendering()));
    styleLay->addWidget(m_smoothCheck);
    oLay->addWidget(styleGroup);

    oLay->addStretch(1);

    rightSplitter->addWidget(optionsPanel);

    v->initPipeline(this);
    v->setBackground(QApplication::palette().color(QPalette::Window));

    connect(m_dataSetWidget, SIGNAL(changed()), this, SLOT(slotRefreshRendering()));
    slotRefreshRendering();
}

Thermal3DAnalysis::~Thermal3DAnalysis()
{
    delete v;
}

void Thermal3DAnalysis::slotRefreshRendering()
{
    int colorMode = m_altColorsCheck->isChecked() ? 1 : 0;
    bool smoothing = m_smoothCheck->isChecked();
    m_dataSetWidget->render(v, colorMode, smoothing);
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

//
// DataSetTreeItem
//
class Inspector::Internal::DataSetTreeItem : public QTreeWidgetItem
{
public:
    DataSetTreeItem(QTreeWidget *parent, const QString &string, int type);
    DataSetTreeItem(QTreeWidgetItem *parent, const QString &string, int type);

    void setMesh(const Probe::RegularMeshRealData &mesh);
    Probe::RegularMeshRealData mesh() const;

    void setFilterRadius(qreal radius);
    qreal filterRadius() const;

private:
    void init();
    Probe::RegularMeshRealData m_mesh;
    qreal m_filterRadius;
};

DataSetTreeItem::DataSetTreeItem(QTreeWidget *parent, const QString &string, int type)
  : QTreeWidgetItem(parent, QStringList() << string, type)
{
    init();
}

DataSetTreeItem::DataSetTreeItem(QTreeWidgetItem *parent, const QString &string, int type)
  : QTreeWidgetItem(parent, QStringList() << string, type)
{
    init();
    QFont font;
    font.setItalic(true);
    setData(0, Qt::FontRole, font);
}

void DataSetTreeItem::init()
{
    m_filterRadius = 0;
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

void DataSetTreeItem::setFilterRadius(qreal radius)
{
    m_filterRadius = qMax(radius, (qreal)0);
    emitDataChanged();
}

qreal DataSetTreeItem::filterRadius() const
{
    return m_filterRadius;
}

//
// DataSetTreeWidget
//
DataSetTreeWidget::DataSetTreeWidget(ThermalModel *sourceModel, QWidget *parent)
  : QTreeWidget(parent)
  , m_sourceModel(sourceModel)
{
    setEditTriggers(NoEditTriggers);
    setHeaderLabel(tr("Data Sets"));
    setSelectionMode(SingleSelection);
    setSelectionBehavior(SelectRows);
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
            this, SIGNAL(changed()));

    // monitor for selection
    connect(this, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
}

static QList<DataSetTreeItem *> s_checkedItems(QTreeWidgetItem *root)
{
    QList<DataSetTreeItem *> items;
    int count = root->childCount();
    for (int idx = 0; idx < count; ++idx) {
        DataSetTreeItem *item = static_cast<DataSetTreeItem *>(root->child(idx));
        if (item->checkState(0) != Qt::Unchecked)
            items.append(item);
        if (item->childCount())
            items.append(s_checkedItems(item));
    }
    return items;
}

void DataSetTreeWidget::render(VtkPrivate *v, int colorMode, bool smoothing) const
{
    v->clearContents();

    QList<DataSetTreeItem *> checkedItems = s_checkedItems(invisibleRootItem());
    bool translucentDrawing = checkedItems.count() > 1;
    foreach (DataSetTreeItem *item, checkedItems)
        v->addRegularMesh(item->mesh(), colorMode, translucentDrawing, smoothing, item->filterRadius());

    v->refresh();
}

void DataSetTreeWidget::slotAppendFiltered()
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.size() != 1)
        return;
    DataSetTreeItem *parent = dynamic_cast<DataSetTreeItem *>(items.first());
    if (!parent || parent->type() != DataOriginal)
        return;

    // ask for filter radius
    bool ok = false;
    double radius = QInputDialog::getDouble(this, tr("Filter Radius"),
                                            tr("Set the Gaussian Filter radius:"),
                                            1.5, 0.0, 100.0, 1, &ok);
    if (!ok || radius < 0.1)
        return;

    // add an entry here to mirror the standarditem
    QString text = tr("%1 (Gaussian Filtered %2)").arg(parent->text(0)).arg(radius);
    DataSetTreeItem *twi = new DataSetTreeItem(parent, text, DataFiltered);
    twi->setMesh(parent->mesh());
    twi->setFilterRadius(radius);

    // expand parent
    setItemExpanded(parent, true);
}

void DataSetTreeWidget::slotRemoveSelected()
{
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (!selected.isEmpty()) {
        qDeleteAll(selected);
        emit changed();
    }
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

void DataSetTreeWidget::slotItemSelectionChanged()
{
    QList<QTreeWidgetItem*> items = selectedItems();
    emit itemSelected(!items.isEmpty());
    if (items.size() != 1) {
        emit topItemSelected(false);
    } else {
       DataSetTreeItem *item = dynamic_cast<DataSetTreeItem *>(items.first());
       bool firstLevel = item && item->type() == DataOriginal;
       emit topItemSelected(firstLevel);
    }
}
