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
#include <QtGui/QColorDialog>
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
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>
#include <vtkTextureMapToPlane.h>
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

    void initPipeline(QObject *connTarget, bool useDepthPeeling);
    void disposePipeline();

    void setBackground(const QColor &backColor);
    void toggleStereo();
    void refresh();

    void clearContents();
    void addRegularMesh(const Inspector::Probe::RegularMeshRealData &, const QColor &,
                        int colorMode, bool zeroPlane, bool translucent, bool smooth,
                        double filterRadius, const QImage &texture);

    QWidget *widget() const { return m_widget; }

private:
    QVTKWidget *m_widget;
    vtkRenderer *m_renderer;
    vtkEventQtSlotConnect *m_connections;
    QList<vtkActor *> m_addedActors;
    bool m_depthPeeling;
};

VtkPrivate::VtkPrivate()
  : m_widget(0)
  , m_renderer(0)
  , m_connections(0)
  , m_depthPeeling(false)
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

void VtkPrivate::initPipeline(QObject *connTarget, bool useDepthPeeling)
{
    m_depthPeeling = useDepthPeeling;

    // create a window to make it stereo capable and give it to QVTKWidget
    {
        vtkRenderWindow *renderWindow = vtkRenderWindow::New();
        if (m_depthPeeling) {
            renderWindow->SetAlphaBitPlanes(1);
            renderWindow->SetMultiSamples(0);
        }
        renderWindow->StereoCapableWindowOn();
        m_widget->SetRenderWindow(renderWindow);
        renderWindow->Delete();
    }

    // create the Renderer and connect it to the RenderWindow
    m_renderer = vtkRenderer::New();
    if (m_depthPeeling) {
        m_renderer->SetUseDepthPeeling(1);
        m_renderer->SetMaximumNumberOfPeels(100);
        m_renderer->SetOcclusionRatio(0.1);
    }
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
    refresh();
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
                                const QColor &color, int colorMode, bool zeroPlane,
                                bool translucent, bool smooth, double filterRadius,
                                const QImage &textureImage)
{
    if (!mesh.rows || !mesh.columns || mesh.data.isEmpty())
        return;

    // TODO: add RMS-normalization multiplier (10.0 plays well on enrico's notebook...)
    const double physicalWidth = mesh.physicalSize.width();
    const double physicalHeight = mesh.physicalSize.height();
    double rmsNormalizer = 10.0;

    // create the image data scaled to physical size
    vtkImageData *imageData = vtkImageData::New();
    imageData->SetDimensions(mesh.columns, mesh.rows, 1);
    imageData->SetOrigin(-physicalWidth / 2.0, -physicalHeight / 2.0, 0.0);
    imageData->SetNumberOfScalarComponents(1);
    imageData->SetScalarTypeToDouble();
    imageData->SetSpacing(
            (double)physicalWidth / (double)(mesh.columns - 1),
            (double)physicalHeight / (double)(mesh.rows - 1),
            1.0);
    int meshIdx = 0;
    for (int row = mesh.rows - 1; row >= 0; --row) {
        for (int col = 0; col < mesh.columns; ++col) {
            double* pixel = static_cast<double*>(imageData->GetScalarPointer(col, row, 0));
            pixel[0] = mesh.data[meshIdx++] * rmsNormalizer;
        }
    }
    vtkDataSet *inputData = imageData;

    vtkTextureMapToPlane *texturePlane = 0;
    if (!textureImage.isNull() && textureImage.format() == QImage::Format_ARGB32) {
        texturePlane = vtkTextureMapToPlane::New();
        texturePlane->SetInput(imageData);
        inputData = texturePlane->GetOutput();
    }

    vtkImageGaussianSmooth *gaussianFilter = 0;
    if (filterRadius > 0.1) {
        gaussianFilter = vtkImageGaussianSmooth::New();
        gaussianFilter->SetRadiusFactor(filterRadius);
        gaussianFilter->SetInput(inputData);
        inputData = gaussianFilter->GetOutput();
    }

    vtkImageDataGeometryFilter *geometry = vtkImageDataGeometryFilter::New();
    geometry->SetInput(inputData);

    vtkWarpScalar *warp = vtkWarpScalar::New();
    warp->SetInput(geometry->GetOutput());
    warp->SetNormal(0, 0, 1);
    warp->SetUseNormal(true);
    warp->SetXYPlane(false);
    warp->SetScaleFactor(255);

    vtkPolyDataNormals *normals = 0;
    if (smooth) {
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
    mapper->SetInput(normals ? (vtkPointSet*)normals->GetOutput() : warp->GetOutput());

    bool colorize = color != Qt::white;
    if (colorMode != -1 || colorize || texturePlane) {
        vtkLookupTable *lut = vtkLookupTable::New();
        lut->SetRange(0, 1);
        if (texturePlane) {
            lut->SetHueRange(0, 0);
            lut->SetSaturationRange(0, 0);
            lut->SetValueRange(0.8, 1);
            lut->SetAlphaRange(1, 1);
        } else if (colorize) {
            double hue = color.hueF();
            lut->SetHueRange(hue, hue);
            lut->SetSaturationRange(0, color.saturationF());
            lut->SetValueRange(0, color.valueF());
            lut->SetAlphaRange(1, 1);
        } else {
            lut->SetHueRange(0.667, 0);
            lut->SetSaturationRange(colorMode == 1 ? 0 : 1, 1);
            lut->SetValueRange(1, 1);
            lut->SetAlphaRange(colorMode == 1 ? 0 : 1, 1);
        }
        lut->Build();
        mapper->SetLookupTable(lut);
        lut->Delete();
    }

    vtkActor* actor = vtkActor::New();
    actor->SetMapper(mapper);

    if (texturePlane) {
        const int textureWidth = textureImage.width();
        const int textureHeight = textureImage.height();
        vtkImageData *textureData = vtkImageData::New();
        textureData->SetDimensions(textureWidth, textureHeight, 1);
        textureData->SetNumberOfScalarComponents(4);
        textureData->SetScalarTypeToUnsignedChar();
        for (int row = 0; row < textureHeight; ++row) {
            const quint32 *sourcePixels = (const quint32 *)textureImage.scanLine(row);
            quint32 *destPixels = (quint32 *)textureData->GetScalarPointer(0, textureHeight - row - 1, 0);
            for (int col = 0; col < textureWidth; ++col) {
                quint32 pixel = sourcePixels[col];
                destPixels[col] = (pixel & 0xFF00FF00) | ((pixel & 0x00FF0000) >> 16) | ((pixel & 0x000000FF) << 16);
            }
        }
        vtkTexture *texture = vtkTexture::New();
        texture->SetInput(textureData);
        actor->SetTexture(texture);
        texture->Delete();
        textureData->Delete();
    }

    if (translucent) {
        vtkProperty *property = vtkProperty::New();
        property->SetOpacity(0.5);
        actor->SetProperty(property);
        property->Delete();
    }

    m_renderer->AddViewProp(actor);
    m_addedActors.append(actor);

    if (zeroPlane) {
        vtkPlaneSource *backPlaneSource = vtkPlaneSource::New();
        backPlaneSource->SetNormal(0.0, 0.0, 1.0);
        backPlaneSource->SetCenter(-physicalWidth / 2, -physicalHeight / 2, 0.0);
        backPlaneSource->SetPoint1(physicalWidth / 2, -physicalHeight / 2, 0);
        backPlaneSource->SetPoint2(-physicalWidth / 2,  physicalHeight / 2, 0);

        vtkTextureMapToPlane *backTexPlane = vtkTextureMapToPlane::New();
        backTexPlane->SetInput(backPlaneSource->GetOutput());

        vtkPolyDataMapper *backMapper = vtkPolyDataMapper::New();
        backMapper->SetInputConnection(backTexPlane->GetOutputPort());

        vtkActor *backActor = vtkActor::New();
        backActor->SetMapper(backMapper);

        m_renderer->AddViewProp(backActor);
        m_addedActors.append(backActor);

        backMapper->Delete();
        backTexPlane->Delete();
        backPlaneSource->Delete();
    }

    mapper->Delete();
    if (normals)
        normals->Delete();
    warp->Delete();
    geometry->Delete();
    if (gaussianFilter)
        gaussianFilter->Delete();
    if (texturePlane)
        texturePlane->Delete();
    imageData->Delete();
}

//
// Thermal3DAnalysis
//
Thermal3DAnalysis::Thermal3DAnalysis(PaintingModule *module, bool useDepthPeeling, QWidget *parent)
  : QWidget(parent)
  , v(new VtkPrivate)
  , m_paintingModule(module)
{
    QVBoxLayout *vLay = new QVBoxLayout(this);
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    vLay->addWidget(splitter);

    splitter->addWidget(v->widget());

    QSplitter *rightSplitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(rightSplitter);

    m_dataSetWidget = new DataSetTreeWidget(module->thermalModel());
    rightSplitter->addWidget(m_dataSetWidget);

    QWidget *optionsPanel = new QWidget;
    QVBoxLayout *oLay = new QVBoxLayout(optionsPanel);
    oLay->setMargin(0);

    QPushButton *filterButton = new QPushButton(tr("Add Filtered"));
    filterButton->setEnabled(false);
    connect(m_dataSetWidget, SIGNAL(topItemSelected(bool)),
            filterButton, SLOT(setEnabled(bool)));
    connect(filterButton, SIGNAL(clicked()),
            m_dataSetWidget, SLOT(slotAppendFiltered()));
    oLay->addWidget(filterButton);

    QPushButton *removeButton = new QPushButton(tr("Remove Selected"));
    removeButton->setEnabled(false);
    connect(m_dataSetWidget, SIGNAL(itemSelected(bool)),
            removeButton, SLOT(setEnabled(bool)));
    connect(removeButton, SIGNAL(clicked()),
            m_dataSetWidget, SLOT(slotRemoveSelected()));
    oLay->addWidget(removeButton);

    QPushButton *colorButton = new QPushButton(tr("Set Surface Color"));
    colorButton->setEnabled(false);
    connect(m_dataSetWidget, SIGNAL(itemSelected(bool)),
            colorButton, SLOT(setEnabled(bool)));
    connect(colorButton, SIGNAL(clicked()),
            m_dataSetWidget, SLOT(slotColorizeSelected()));
    oLay->addWidget(colorButton);

    QGroupBox *styleGroup = new QGroupBox(tr("Drawing Options"));
    QVBoxLayout *styleLay = new QVBoxLayout(styleGroup);
    m_texturesCheck = new QCheckBox(tr("Overlay Source Image"));
    connect(m_texturesCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotRefreshRendering()));
    styleLay->addWidget(m_texturesCheck);
    m_zeroPlanesCheck = new QCheckBox(tr("Show Zero Plane"));
    connect(m_zeroPlanesCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotRefreshRendering()));
    styleLay->addWidget(m_zeroPlanesCheck);
    m_altColorsCheck = new QCheckBox(tr("Alternate Colors"));
    connect(m_altColorsCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotRefreshRendering()));
    connect(m_texturesCheck, SIGNAL(toggled(bool)),
            m_altColorsCheck, SLOT(setDisabled(bool)));
    styleLay->addWidget(m_altColorsCheck);
    m_smoothCheck = new QCheckBox(tr("Smooth Colors"));
    connect(m_smoothCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotRefreshRendering()));
    styleLay->addWidget(m_smoothCheck);
    oLay->addWidget(styleGroup);

    oLay->addStretch(1);

    rightSplitter->addWidget(optionsPanel);

    v->initPipeline(this, useDepthPeeling);
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
    bool useTextures = m_texturesCheck->isChecked();
    bool zeroPlane = m_zeroPlanesCheck->isChecked();
    int colorMode = m_altColorsCheck->isChecked() ? 1 : 0;
    bool smoothing = m_smoothCheck->isChecked();
    m_dataSetWidget->render(v, useTextures, zeroPlane, colorMode, smoothing);
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

    void setImage(const QImage &image);
    QImage image() const;

    void setFilterRadius(qreal radius);
    qreal filterRadius() const;

    void setSurfaceColor(const QColor &);
    QColor surfaceColor() const;

private:
    void init();
    Probe::RegularMeshRealData m_mesh;
    QImage m_image;
    qreal m_filterRadius;
    QColor m_surfaceColor;
};

DataSetTreeItem::DataSetTreeItem(QTreeWidget *parent, const QString &string, int type)
  : QTreeWidgetItem(QStringList() << string, type)
{
    init();
    parent->insertTopLevelItem(0, this);
}

DataSetTreeItem::DataSetTreeItem(QTreeWidgetItem *parent, const QString &string, int type)
  : QTreeWidgetItem(QStringList() << string, type)
{
    init();
    QFont font;
    font.setItalic(true);
    setData(0, Qt::FontRole, font);
    parent->insertChild(0, this);;
}

void DataSetTreeItem::init()
{
    m_filterRadius = 0;
    m_surfaceColor = Qt::white;
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
    setData(0, Qt::SizeHintRole, QSize(150, 22));
}

void DataSetTreeItem::setMesh(const Probe::RegularMeshRealData &mesh)
{
    m_mesh = mesh;
    if (checkState(0) != Qt::Unchecked)
        emitDataChanged();
}

Inspector::Probe::RegularMeshRealData DataSetTreeItem::mesh() const
{
    return m_mesh;
}

void DataSetTreeItem::setImage(const QImage &image)
{
    m_image = image;
    if (checkState(0) != Qt::Unchecked)
        emitDataChanged();
}

QImage DataSetTreeItem::image() const
{
    return m_image;
}

void DataSetTreeItem::setFilterRadius(qreal radius)
{
    if (radius < 0 || radius == m_filterRadius)
        return;
    m_filterRadius = qMax(radius, (qreal)0);
    if (checkState(0) != Qt::Unchecked)
        emitDataChanged();
}

qreal DataSetTreeItem::filterRadius() const
{
    return m_filterRadius;
}

void DataSetTreeItem::setSurfaceColor(const QColor &color)
{
    if (!color.isValid() || color == m_surfaceColor)
        return;
    m_surfaceColor = color;
    if (color == Qt::white)
        setData(0, Qt::ForegroundRole, QApplication::palette().color(QPalette::WindowText));
    else
        setData(0, Qt::ForegroundRole, color);
    if (checkState(0) != Qt::Unchecked)
        emitDataChanged();
}

QColor DataSetTreeItem::surfaceColor() const
{
    return m_surfaceColor;
}

//
// DataSetTreeWidget
//
DataSetTreeWidget::DataSetTreeWidget(ThermalModel *sourceModel, QWidget *parent)
  : QTreeWidget(parent)
  , m_sourceModel(sourceModel)
{
    setAnimated(true);
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

void DataSetTreeWidget::render(VtkPrivate *v, bool useTextures, bool zeroPlane, int colorMode, bool smoothing) const
{
    v->clearContents();

    QList<DataSetTreeItem *> checkedItems = s_checkedItems(invisibleRootItem());
    bool translucentDrawing = checkedItems.count() > 1;
    if (translucentDrawing)
        zeroPlane = false;
    foreach (DataSetTreeItem *item, checkedItems)
        v->addRegularMesh(item->mesh(), item->surfaceColor(), colorMode, zeroPlane,
                          translucentDrawing, smoothing, item->filterRadius(),
                          useTextures ? item->image() : QImage());

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
    twi->setImage(parent->image());
    twi->setFilterRadius(radius);
    if (parent->surfaceColor().isValid() && parent->surfaceColor() != Qt::white)
        twi->setSurfaceColor(parent->surfaceColor());

    // focus the new child
    setItemExpanded(parent, true);
    clearSelection();
    setItemSelected(twi, true);
    twi->setCheckState(0, parent->checkState(0));
    parent->setCheckState(0, Qt::Unchecked);
}

void DataSetTreeWidget::slotRemoveSelected()
{
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (!selected.isEmpty()) {
        qDeleteAll(selected);
        emit changed();
    }
}

void DataSetTreeWidget::slotColorizeSelected()
{
    QList<QTreeWidgetItem*> selection = selectedItems();
    if (selection.size() != 1)
        return;
    if (DataSetTreeItem *item = dynamic_cast<DataSetTreeItem *>(selection.first())) {
        QColor color = QColorDialog::getColor(Qt::white, this, tr("Surface Color"));
        if (color.isValid())
            item->setSurfaceColor(color);
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
        twi->setMesh(item->originalMesh());
        twi->setImage(item->originalImage());
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
