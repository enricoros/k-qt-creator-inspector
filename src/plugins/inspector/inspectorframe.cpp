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

#include "inspectorframe.h"
#include "abstractview.h"
#include "combotreewidget.h"
#include "commserver.h"
#include "inspectorinstance.h"
#include "instanceview.h"
#include "probecontroller.h"
#include "taskbarwidget.h"
#include "ui_commview.h"
#include <QGradient>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QSvgRenderer>

namespace Inspector {
namespace Internal {

class ViewContainerWidget : public QWidget
{
    public:
        ViewContainerWidget(QWidget * parent = 0)
          : QWidget(parent)
          , m_widget(0)
          , m_disabled(false)
          //, m_disabledLabel(0)
        {
            // precache watermark pixmap
            QSvgRenderer wmRender(QString(":/inspector/images/probe-watermark.svg"));
            if (wmRender.isValid()) {
                m_watermarkPixmap = QPixmap(wmRender.defaultSize());
                m_watermarkPixmap.fill(Qt::transparent);
                QPainter wmPainter(&m_watermarkPixmap);
                wmRender.render(&wmPainter);
            }

            // create the disabled warning lable
            //m_disabledLabel = new QLabel(tr("This View is disabled. Probably the View is not available in the current state."), this);
            //m_disabledLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            //m_disabledLabel->hide();

            // set a vertical layout
            QVBoxLayout * lay = new QVBoxLayout(this);
            //lay->addWidget(m_disabledLabel);
            setLayout(lay);
        }

        /*void setDisableWidget(bool disabled)
        {
            if (disabled != m_disabled) {
                m_disabled = disabled;
                m_disabledLabel->setVisible(disabled);
                if (m_widget)
                    m_widget->setEnabled(!m_disabled);
            }
        }*/

        void setWidget(QWidget * widget)
        {
            delete m_widget;
            m_widget = widget;
            if (widget) {
                widget->setParent(this);
                layout()->addWidget(widget);
                //if (m_disabled)
                //    widget->setEnabled(false);
            }
        }

    protected:
        void paintEvent(QPaintEvent * event)
        {
            // draw a light gradient as the background
            QPainter p(this);
            QLinearGradient bg(0, 0, 0, 1);
            bg.setCoordinateMode(QGradient::StretchToDeviceMode);
            bg.setColorAt(0.0, QColor(247, 247, 247));
            bg.setColorAt(1.0, QColor(215, 215, 215));
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(event->rect(), bg);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);

            // draw the watermark
            if (!m_watermarkPixmap.isNull())
                p.drawPixmap(isLeftToRight() ? (width() - m_watermarkPixmap.width()) : 0, 0, m_watermarkPixmap);
        }

    private:
        QPixmap m_watermarkPixmap;
        QWidget * m_widget;
        bool m_disabled;
        //QLabel * m_disabledLabel;
};

InspectorFrame::InspectorFrame(QWidget *parent)
  : QWidget(parent)
  , m_extInstance(0)
  , m_menuWidget(0)
  , m_viewWidget(0)
  , m_taskbarWidget(0)
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_menuWidget = new ComboTreeWidget(this);
    connect(m_menuWidget, SIGNAL(pathSelected(QStringList,QVariant)), this, SLOT(slotMenuChanged(QStringList,QVariant)));
    layout->addWidget(m_menuWidget);

    m_viewWidget = new ViewContainerWidget(this);
    layout->addWidget(m_viewWidget);

    m_taskbarWidget = new TaskbarWidget(this);
    layout->addWidget(m_taskbarWidget);
}

void InspectorFrame::setInstance(Inspector::InspectorInstance *instance)
{
    // remove references to any previous instance
    if (m_extInstance) {
        m_menuWidget->clear();
        m_viewWidget->setWidget(new QWidget);
        m_taskbarWidget->clear();
    }

    m_extInstance = instance;

    if (m_extInstance) {
        // menu: add default entry
        m_menuWidget->addItem(QStringList() << tr("Status"), (quint32)0, QIcon(":/inspector/images/menu-icon.png"));

        // menu: add all entries by the plugged probes
        ProbeMenuEntries entries = m_extInstance->probeController()->menuEntries();
        foreach (const ProbeMenuEntry &entry, entries) {
            if ((entry.probeId & 0xFF000000) || (entry.viewId & 0xFFFFFF00)) {
                qWarning("InspectorFrame::setInstance: probeId (%d) or viewId (%d) not valid", entry.probeId, entry.viewId);
                continue;
            }
            quint32 compoId = (entry.probeId << 8) + entry.viewId;
            m_menuWidget->addItem(entry.path, compoId, entry.icon);
        }

        // show information about the current instance
        showInstanceView();

        // TODO link the taskbar
        //m_taskbarWidget-> ...
    }
}

void InspectorFrame::slotMenuChanged(const QStringList &/*path*/, const QVariant &data)
{
    // show the default view, if requested
    quint32 compoId = data.toInt();
    if (!compoId) {
        showInstanceView();
        return;
    }

    // create a probe view
    int probeId = compoId >> 8;
    int viewId = compoId & 0xFF;
    AbstractView * view = m_extInstance ? m_extInstance->probeController()->createView(probeId, viewId) : 0;
    if (!view) {
        qWarning("InspectorFrame::slotMenuChanged: can't create view %d for probe %d", viewId, probeId);
        view = new AbstractView;
    }
    m_viewWidget->setWidget(view);
}

void InspectorFrame::showInstanceView()
{
    if (!m_extInstance)
        return;
    Inspector::CommServer *server = m_extInstance->commServer();
    if (!server) {
        m_viewWidget->setWidget(new QWidget);
        return;
    }

    bool debugging = m_extInstance->debugging();
    InstanceView *view = new InstanceView;
    view->modLabel->setText(m_extInstance->probeController()->probeNames().join(","));
    view->setFieldState(view->debLabel, debugging ? 1 : -1);
    view->setFieldState(view->enaButton, server->m_sEnabled ? 1 : -1);
    view->setFieldState(view->hlpLabel, server->m_sHelpers ? 1 : debugging ? -1 : 0);
    view->setFieldState(view->injLabel, server->m_sInjected ? 1 : debugging ? -1 : 0);
    view->setFieldState(view->conLabel, server->m_sConnected ? 1 : debugging ? -1 : 0);
    view->setFieldState(view->workLabel, (debugging && server->m_sEnabled && server->m_sInjected && server->m_sConnected) ? 1 : 0);

    m_viewWidget->setWidget(view);
}

} // namespace Internal
} // namespace Inspector
