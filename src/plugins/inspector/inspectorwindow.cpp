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

#include "inspectorwindow.h"
#include "iframework.h"
#include "inspectorplugin.h"
#include "instance.h"
#include "shareddebugger.h"
#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/session.h>
#include <utils/stylehelper.h>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#include <QtGui/QToolButton>

namespace Inspector {
namespace Internal {

const int ICON_SIZE(64);
const int ABOVE_HEADING_MARGIN(10);
const int ABOVE_CONTENTS_MARGIN(4);
const int BELOW_CONTENTS_MARGIN(16);
const int LEFT_MARGIN(16);

const int BUTTON_INSPECT_RUN = 0x0001;

class OnePixelBlackLine : public QWidget
{
public:
    OnePixelBlackLine(QWidget *parent)
        : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(1);
    }
    void paintEvent(QPaintEvent *e)
    {
        Q_UNUSED(e);
        QPainter p(this);
        p.fillRect(contentsRect(), QBrush(Utils::StyleHelper::borderColor()));
    }
};


InspectorWindow::InspectorWindow(QWidget *parent)
  : QScrollArea(parent)
  , m_root(new QWidget(this))
{
    // We want a 900px wide widget with and the scrollbar at the side of the screen.
    m_root->setFixedWidth(900);

    m_layout = new QGridLayout(m_root);
    m_layout->setColumnMinimumWidth(0, ICON_SIZE + 4);
    m_layout->setSpacing(0);

    setWidget(m_root);
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);

    // create the New Local Target widget
    {
        QWidget *panel = new QWidget;
        QGridLayout *grid = new QGridLayout(panel);
        grid->setMargin(0);
        grid->setSpacing(0);
        grid->setColumnMinimumWidth(0, LEFT_MARGIN);

        /*QLabel *desc = new QLabel;
        desc->setText(tr("Select the kind of local target you want to Inspect."));
        desc->setWordWrap(true);
        appendSubWidget(grid, desc);*/

        // run a new Instance (TODO: turn this into a parser of "Inspect %1 running %2 with %3")
        QWidget *runWidget = new QWidget;
        QHBoxLayout *runLayout = new QHBoxLayout(runWidget);
         runLayout->addWidget(new QLabel(tr("Inspect")));
        m_projectsCombo = new ProjectsComboBox;
         runLayout->addWidget(m_projectsCombo);
        m_targetsCombo = new TargetsComboBox;
         runLayout->addWidget(m_targetsCombo);
        m_runconfLabel = new QLabel(tr("running"));
         runLayout->addWidget(m_runconfLabel);
        m_runconfsCombo = new RunconfComboBox;
         runLayout->addWidget(m_runconfsCombo);
         runLayout->addWidget(new QLabel(tr("with")));
        m_frameworksCombo = new FrameworksComboBox;
         runLayout->addWidget(m_frameworksCombo);
         runLayout->addWidget(new QLabel(tr("framework")));
         runLayout->addStretch();
        m_newRunButton = newInspectButton(BUTTON_INSPECT_RUN);
         runLayout->addWidget(m_newRunButton);
        appendSubWidget(grid, runWidget, tr("Run"),
                        tr("Start a new instance of the selected project."));

        slotProjectChanged();
        connect(m_projectsCombo, SIGNAL(currentProjectChanged()),
                this, SLOT(slotProjectChanged()));
        connect(m_targetsCombo, SIGNAL(currentTargetChanged()),
                this, SLOT(slotTargetChanged()));
        connect(m_runconfsCombo, SIGNAL(currentRunconfChanged()),
                this, SLOT(slotRunconfChanged()));

        // TODO - attach to an existing run
        QWidget *instWidget = new QLabel("running opts - *WIP*");
        appendSubWidget(grid, instWidget, tr("Running Instances"),
                        tr("Attach to an existing process."));

        // TODO - use an existing debugging session
        QWidget *debugWidget = new QLabel("debugging opts - *WIP*");
        appendSubWidget(grid, debugWidget, tr("Debugging Instances"),
                        tr("Use an existing debugging session."));

        appendWrappedWidget(tr("New Local Target"),
                            QIcon(":/inspector/images/inspector-icon-32.png"),
                            panel);

        // disable the panel while the debugger is running (should be done per-runconf)
        SharedDebugger *sharedDebugger = InspectorPlugin::pluginInstance()->sharedDebugger();
        connect(sharedDebugger, SIGNAL(availableChanged(bool)), panel, SLOT(setEnabled(bool)));
        panel->setEnabled(sharedDebugger->available());
    }

    // create the Active Targets widget
    {
        QWidget *widget = new QWidget;
        QGridLayout *grid = new QGridLayout(widget);
        grid->setMargin(0);
        grid->setSpacing(0);
        grid->setColumnMinimumWidth(0, LEFT_MARGIN);

        // TODO
        QTableWidget *tw = new QTableWidget;
        tw->setRowCount(1);
        tw->setColumnCount(4);
        tw->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        tw->setFrameStyle(QFrame::NoFrame);
        tw->setFixedHeight(50);
        appendSubWidget(grid, tw);

        appendWrappedWidget(tr("Active Targets"),
                            QIcon(":/projectexplorer/images/rebuild.png"),
                            widget);
    }

    // create the Configure Frameworks widget
    {
        QWidget *widget = new QWidget;
        QGridLayout *grid = new QGridLayout(widget);
        grid->setMargin(0);
        grid->setSpacing(0);
        grid->setColumnMinimumWidth(0, LEFT_MARGIN);

        foreach (IFrameworkFactory *factory, FrameworksComboBox::allFactories()) {
            QWidget *rowWidget = new QWidget;
            QHBoxLayout *rowLay = new QHBoxLayout(rowWidget);
            rowLay->setMargin(0);

            if (!factory->icon().isNull()) {
                QLabel *iconLabel = new QLabel;
                iconLabel->setFixedSize(32, 32);
                iconLabel->setPixmap(factory->icon().pixmap(32, 32));
                rowLay->addWidget(iconLabel);
            }

            // TODO
            QLabel *modulesLabel = new QLabel;
            modulesLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
            modulesLabel->setWordWrap(true);
            modulesLabel->setText(tr("modules: %1\nrequires: %2").arg("some, hardcoded, strings, fix, this").arg("exclusive-probe"));
            rowLay->addWidget(modulesLabel);

            QToolButton *btn = new QToolButton;
            btn->setIcon(QIcon(":/projectexplorer/images/rebuild.png"));
            if (factory->isConfigurable())
                connect(btn, SIGNAL(clicked()), factory, SLOT(configure()));
            else
                btn->setEnabled(false);
            rowLay->addWidget(btn);

            appendSubWidget(grid, rowWidget, factory->displayName());
        }

        appendWrappedWidget(tr("Configure Frameworks"), QIcon(), widget);
    }
}

void InspectorWindow::newTarget(ProjectExplorer::RunConfiguration *rc, IFrameworkFactory *factory)
{
    // sanity check
    if (!factory->available()) {
        qWarning("InspectorPlugin::newTarget: can't start more instances of creator's debugger");
        return;
    }

    Instance *instance = new Instance(rc->displayName(), factory);
    if (!instance->framework()->startRunConfiguration(rc)) {
        qWarning("InspectorPlugin::newTarget: can't start the run configuration. skipping");
        delete instance;
        return;
    }

    InspectorPlugin::pluginInstance()->addInstance(instance);
}

void InspectorWindow::slotCreateTarget()
{
    int id = static_cast<QToolButton *>(sender())->property("id").toInt();
    switch (id) {
    case BUTTON_INSPECT_RUN:
        if (ProjectExplorer::RunConfiguration *rc = m_runconfsCombo->currentRunConfiguration()) {
            if (IFrameworkFactory *factory = m_frameworksCombo->currentFactory()) {
                newTarget(rc, factory);
            }
        }
        break;
    default:
        qWarning("InspectorWindow::slotNewTarget: unhandled button %d", id);
        return;
    }
    emit requestDisplay();
}

void InspectorWindow::slotProjectChanged()
{
    ProjectExplorer::Project *project = m_projectsCombo->currentProject();
    m_targetsCombo->setProject(project);
    m_targetsCombo->setVisible(m_targetsCombo->count() > 1);
    m_projectsCombo->setEnabled(project);
}

void InspectorWindow::slotTargetChanged()
{
    ProjectExplorer::Target *target = m_targetsCombo->currentTarget();
    m_runconfsCombo->setTarget(target);
    m_runconfsCombo->setVisible(m_runconfsCombo->count() > 1);
    m_runconfLabel->setVisible(m_runconfsCombo->count() > 1);
}

void InspectorWindow::slotRunconfChanged()
{
    ProjectExplorer::RunConfiguration *runconf = m_runconfsCombo->currentRunConfiguration();
    m_frameworksCombo->setEnabled(runconf);
    m_newRunButton->setEnabled(runconf);
}

QAbstractButton *InspectorWindow::newInspectButton(int id)
{
    QPushButton *b = new QPushButton;
    b->setMaximumHeight(Utils::StyleHelper::navigationWidgetHeight() - 2);
    b->setText(tr("Start"));
    b->setIcon(QIcon(":/projectexplorer/images/run.png"));
    b->setProperty("id", id);
    connect(b, SIGNAL(clicked()), this, SLOT(slotCreateTarget()));
    return b;
}

// keep this in sync with PanelsWidget::addPropertiesPanel in projectwindow.cpp
void InspectorWindow::appendWrappedWidget(const QString &title, const QIcon &icon,
                                          QWidget *widget)
{
    // icon:
    const int headerRow(m_layout->rowCount() - 1);
    m_layout->setRowStretch(headerRow, 0);

    if (!icon.isNull()) {
        QLabel *iconLabel = new QLabel(m_root);
        iconLabel->setPixmap(icon.pixmap(ICON_SIZE, ICON_SIZE));
        iconLabel->setContentsMargins(0, ABOVE_HEADING_MARGIN, 0, 0);
        m_layout->addWidget(iconLabel, headerRow, 0, 2, 1, Qt::AlignTop | Qt::AlignHCenter);
    }

    // name:
    QLabel *nameLabel = new QLabel(m_root);
    nameLabel->setText(title);
    nameLabel->setContentsMargins(0, ABOVE_HEADING_MARGIN, 0, 0);
    QFont f = nameLabel->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() * 1.4);
    nameLabel->setFont(f);
    m_layout->addWidget(nameLabel, headerRow, 1, 1, 1, Qt::AlignBottom | Qt::AlignLeft);

    // line:
    const int lineRow(headerRow + 1);
    m_layout->addWidget(new OnePixelBlackLine(m_root), lineRow, 1);

    // add the widget:
    const int widgetRow(lineRow + 1);
    widget->setContentsMargins(m_layout->columnMinimumWidth(0),
                               ABOVE_CONTENTS_MARGIN, 0,
                               BELOW_CONTENTS_MARGIN);
    widget->setParent(m_root);
    m_layout->addWidget(widget, widgetRow, 0, 1, 2);

    // stretch:
    const int stretchRow(widgetRow + 1);
    m_layout->setRowStretch(stretchRow, 10);
}

void InspectorWindow::appendSubWidget(QGridLayout *layout, QWidget *widget,
                                      const QString &title, const QString &subTitle)
{
    int insertionRow = layout->rowCount();
    if (!title.isEmpty()) {
        QLabel *label = new QLabel;
        label->setText(title);
        QFont f = label->font();
        f.setBold(true);
        f.setPointSizeF(f.pointSizeF() * 1.2);
        label->setFont(f);
        label->setContentsMargins(0, 10, 0, 0);

        layout->addWidget(label, insertionRow, 0, 1, 2);
        layout->setRowStretch(insertionRow, 0);
        ++insertionRow;

        if (!subTitle.isEmpty()) {
            QLabel *subLabel = new QLabel;
            subLabel->setText(subTitle);
            subLabel->setWordWrap(true);
            QFont smallFont = subLabel->font();
            smallFont.setPointSizeF(smallFont.pointSizeF() / 1.2);
            subLabel->setFont(smallFont);

            layout->addWidget(subLabel, insertionRow, 0, 1, 2);
            layout->setRowStretch(insertionRow, 0);
            ++insertionRow;
        }

        if (widget) {
            widget->setContentsMargins(0, 10, 0, 0);
            layout->addWidget(widget, insertionRow, 1, 1, 1);
        }
    } else if (widget)
        layout->addWidget(widget, insertionRow, 0, 1, 2);
}

//
// ProjectsComboBox
//
ProjectsComboBox::ProjectsComboBox(QWidget *parent)
  : QComboBox(parent)
{
    setMaximumHeight(Utils::StyleHelper::navigationWidgetHeight() - 2);
    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(currentProjectChanged()));

    ProjectExplorer::SessionManager *session = ProjectExplorer::ProjectExplorerPlugin::instance()->session();
    foreach(ProjectExplorer::Project *project, session->projects())
        add(project);
    connect(session, SIGNAL(projectAdded(ProjectExplorer::Project*)),
            this, SLOT(add(ProjectExplorer::Project*)));
    connect(session, SIGNAL(aboutToRemoveProject(ProjectExplorer::Project*)),
            this, SLOT(remove(ProjectExplorer::Project*)));
    connect(session, SIGNAL(startupProjectChanged(ProjectExplorer::Project*)),
            this, SLOT(activeChanged(ProjectExplorer::Project*)));
}

bool ProjectsComboBox::isEmpty() const
{
    return !count();
}

ProjectExplorer::Project *ProjectsComboBox::currentProject() const
{
    int index = currentIndex();
    if (index < 0 || index >= m_comboIndexToProject.size())
        return 0;
    return m_comboIndexToProject.at(index);
}

void ProjectsComboBox::add(ProjectExplorer::Project *project)
{
    ProjectExplorer::SessionManager *session = ProjectExplorer::ProjectExplorerPlugin::instance()->session();
    int index = session->projects().indexOf(project);
    m_comboIndexToProject.insert(index, project);
    insertItem(index, project->displayName());
}

void ProjectsComboBox::remove(ProjectExplorer::Project *project)
{
    int index = m_comboIndexToProject.indexOf(project);
    m_comboIndexToProject.removeAt(index);
    removeItem(index);
}

void ProjectsComboBox::activeChanged(ProjectExplorer::Project *project)
{
    int index = m_comboIndexToProject.indexOf(project);
    setCurrentIndex(index);
}

//
// TargetComboBox
//
TargetsComboBox::TargetsComboBox(QWidget *parent)
  : QComboBox(parent)
  , m_project(0)
{
    setMaximumHeight(Utils::StyleHelper::navigationWidgetHeight() - 2);
    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(currentTargetChanged()));
}

void TargetsComboBox::setProject(ProjectExplorer::Project *project)
{
    if (m_project == project)
        return;

    // remove links to previous project
    if (m_project) {
        disconnect(m_project, 0, this, 0);
        while (count())
            remove(itemData(0).value<ProjectExplorer::Target *>());
    }

    m_project = project;

    // link this project
    if (m_project) {
        foreach(ProjectExplorer::Target *target, m_project->targets())
            add(target);
        connect(m_project, SIGNAL(addedTarget(ProjectExplorer::Target*)),
                this, SLOT(add(ProjectExplorer::Target*)));
        connect(m_project, SIGNAL(removedTarget(ProjectExplorer::Target*)),
                this, SLOT(remove(ProjectExplorer::Target*)));
        connect(m_project, SIGNAL(activeTargetChanged(ProjectExplorer::Target*)),
                this, SLOT(activeChanged(ProjectExplorer::Target*)));
    }
}

ProjectExplorer::Target *TargetsComboBox::currentTarget() const
{
    if (currentIndex() < 0)
        return 0;
    return itemData(currentIndex()).value<ProjectExplorer::Target *>();
}

void TargetsComboBox::add(ProjectExplorer::Target *target)
{
    connect(target, SIGNAL(displayNameChanged()),
            this, SLOT(updateDisplayName()));
    addItem(target->displayName(), QVariant::fromValue(target));
    if (m_project->activeTarget() == target)
        setCurrentIndex(count() - 1);
}

void TargetsComboBox::remove(ProjectExplorer::Target *target)
{
    disconnect(target, 0, this, 0);
    removeItem(findData(QVariant::fromValue(target)));
}

void TargetsComboBox::activeChanged(ProjectExplorer::Target *target)
{
    setCurrentIndex(findData(QVariant::fromValue(target)));
}

void TargetsComboBox::updateDisplayName()
{
    ProjectExplorer::Target *target = static_cast<ProjectExplorer::Target *>(sender());
    setItemText(findData(QVariant::fromValue(target)), target->displayName());
}

//
// RunconfComboBox
//
RunconfComboBox::RunconfComboBox(QWidget *parent)
  : QComboBox(parent)
  , m_target(0)
{
    setMaximumHeight(Utils::StyleHelper::navigationWidgetHeight() - 2);
    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(currentRunconfChanged()));
}

void RunconfComboBox::setTarget(ProjectExplorer::Target *target)
{
    if (m_target == target)
        return;

    // remove links to previous target
    if (m_target) {
        disconnect(m_target, 0, this, 0);
        while (count())
            remove(itemData(0).value<ProjectExplorer::RunConfiguration *>());
    }

    m_target = target;

    // link this project
    if (m_target) {
        foreach(ProjectExplorer::RunConfiguration *rc, m_target->runConfigurations())
            add(rc);
        connect(m_target, SIGNAL(addedRunConfiguration(ProjectExplorer::RunConfiguration*)),
                this, SLOT(add(ProjectExplorer::RunConfiguration*)));
        connect(m_target, SIGNAL(removedRunConfiguration(ProjectExplorer::RunConfiguration*)),
                this, SLOT(remove(ProjectExplorer::RunConfiguration*)));
        connect(m_target, SIGNAL(activeRunConfigurationChanged(ProjectExplorer::RunConfiguration*)),
                this, SLOT(activeChanged(ProjectExplorer::RunConfiguration*)));
    }
}

ProjectExplorer::RunConfiguration *RunconfComboBox::currentRunConfiguration() const
{
    if (currentIndex() < 0)
        return 0;
    return itemData(currentIndex()).value<ProjectExplorer::RunConfiguration*>();
}

void RunconfComboBox::add(ProjectExplorer::RunConfiguration *rc)
{
    connect(rc, SIGNAL(displayNameChanged()),
            this, SLOT(updateDisplayName()));
    addItem(rc->displayName(), QVariant::fromValue(rc));
    if (m_target->activeRunConfiguration() == rc)
        setCurrentIndex(count() - 1);
}

void RunconfComboBox::remove(ProjectExplorer::RunConfiguration *rc)
{
    disconnect(rc, 0, this, 0);
    removeItem(findData(QVariant::fromValue(rc)));
}

void RunconfComboBox::activeChanged(ProjectExplorer::RunConfiguration *rc)
{
    setCurrentIndex(findData(QVariant::fromValue(rc)));
}

void RunconfComboBox::updateDisplayName()
{
    ProjectExplorer::RunConfiguration *rc = static_cast<ProjectExplorer::RunConfiguration*>(sender());
    setItemText(findData(QVariant::fromValue(rc)), rc->displayName());
}

//
// FrameworksComboBox
//
FrameworksComboBox::FrameworksComboBox(QWidget *parent)
  : QComboBox(parent)
{
    setMaximumHeight(Utils::StyleHelper::navigationWidgetHeight() - 2);
    foreach (IFrameworkFactory *factory, allFactories())
        addItem(factory->displayName());
    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(currentFrameworkChanged()));
}

IFrameworkFactory *FrameworksComboBox::currentFactory() const
{
    int index = currentIndex();
    QList<IFrameworkFactory *> factories = allFactories();
    if (index < 0 || index >= factories.size())
        return 0;
    return factories.at(index);
}

QList<IFrameworkFactory *> FrameworksComboBox::allFactories()
{
    return ExtensionSystem::PluginManager::instance()->getObjects<IFrameworkFactory>();
}

} // namespace Internal
} // namespace Inspector
