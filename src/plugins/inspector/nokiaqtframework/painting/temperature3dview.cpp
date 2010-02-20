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

#include "temperature3dview.h"
#include "paintingmodel.h"
#include "paintingmodule.h"

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

#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>

using namespace Inspector::Internal;

//
// VtkPrivate
//
class Temperature3DView::VtkPrivate
{
public:
    VtkPrivate();
    ~VtkPrivate();

    void initPipeline(QObject *connTarget);
    void disposePipeline();

    QVTKWidget *widget;
    vtkRenderer *renderer;
    vtkEventQtSlotConnect *connections;
};

Temperature3DView::VtkPrivate::VtkPrivate()
  : widget(0)
  , renderer(0)
  , connections(0)
{
    widget = new QVTKWidget;
    widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    widget->setAttribute(Qt::WA_DontCreateNativeAncestors);
}

Temperature3DView::VtkPrivate::~VtkPrivate()
{
    if (renderer)
        disposePipeline();
}

void Temperature3DView::VtkPrivate::initPipeline(QObject *connTarget)
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

    // put a sample cone in the window
    {
        vtkConeSource *cone = vtkConeSource::New();

        vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
        mapper->SetInput(cone->GetOutput());

        vtkActor* actor = vtkActor::New();
        actor->SetMapper(mapper);

        renderer->AddViewProp(actor);
        actor->Delete();
        mapper->Delete();
        cone->Delete();
    }

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

void Temperature3DView::VtkPrivate::disposePipeline()
{
    if (renderer) {
        renderer->Delete();
        renderer = 0;
    }
    if (connections) {
        connections->Delete();
        connections = 0;
    }
}

//
// DataSetTreeView
//
DataSetTreeView::DataSetTreeView(PaintingModel *model, QWidget *parent)
  : QListView(parent)
{
    setEditTriggers(NoEditTriggers);
    setSelectionMode(ExtendedSelection);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionRectVisible(true);
    setModel(model);
    setRootIndex(model->resultsTableIndex());
}

//
// Temperature3DView
//
Temperature3DView::Temperature3DView(PaintingModule *parentModule, QWidget *parent)
  : QWidget(parent)
  , v(new VtkPrivate)
  , m_paintingModule(parentModule)
{
    QVBoxLayout *vLay = new QVBoxLayout(this);
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    vLay->addWidget(splitter);

    splitter->addWidget(v->widget);

    m_dataSetView = new DataSetTreeView(parentModule->model());
    splitter->addWidget(m_dataSetView);

    m_coordLabel = new QLabel;
    m_coordLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    vLay->addWidget(m_coordLabel);

    v->initPipeline(this);
}

Temperature3DView::~Temperature3DView()
{
    delete v;
}

void Temperature3DView::slotContextMenu(vtkObject *, unsigned long, void *, void *, vtkCommand *command)
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

void Temperature3DView::slotContextAction(QAction *action)
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

void Temperature3DView::slotUpdateCoords(vtkObject *object)
{
    vtkRenderWindowInteractor *interactor = vtkRenderWindowInteractor::SafeDownCast(object);
    int event_pos[2];
    interactor->GetEventPosition(event_pos);
    m_coordLabel->setText(tr("x=%1 : y=%2").arg(event_pos[0]).arg(event_pos[1]));
}
