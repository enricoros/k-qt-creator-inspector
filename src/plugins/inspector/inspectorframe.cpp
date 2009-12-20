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

#include "combotreewidget.h"
#include "commserver.h"
#include "infodialog.h"
#include "inspectorinstance.h"
#include "taskbarwidget.h"
#include "probecontroller.h"
#include "ui_commview.h"

#include <QtGui/QLabel>
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
    // ToolBar
    m_menuWidget = new ComboTreeWidget(this);
    connect(m_menuWidget, SIGNAL(pathSelected(QStringList,QVariant)), this, SLOT(slotMenuChanged(QStringList,QVariant)));
    m_viewWidget = new ViewContainerWidget(this);
    m_taskbarWidget = new TaskbarWidget(this);

    // Main Layout
    QVBoxLayout * vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_menuWidget);
    vLayout->addWidget(m_viewWidget);
    vLayout->addWidget(m_taskbarWidget);
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
        // update menu
        m_menuWidget->addItem(QStringList() << tr("Status"), (quint32)0, QIcon(":/inspector/images/menu-icon.png"));
        ProbeMenuEntries entries = m_extInstance->probeController()->allMenuEntries();
        foreach (const ProbeMenuEntry &entry, entries) {
            if ((entry.probeId & 0xFF000000) || (entry.viewId & 0xFFFFFF00)) {
                qWarning("InspectorFrame::setInstance: probeId (%d) or viewId (%d) not valid", entry.probeId, entry.viewId);
                continue;
            }
            quint32 compoId = (entry.probeId << 8) + entry.viewId;
            m_menuWidget->addItem(entry.path, compoId, entry.icon);
        }

        // show the default view about this probe
        showDefaultView();

        // TODO link the taskbar
        //m_taskbarWidget-> ...
    }
}

void InspectorFrame::showDefaultView()
{
    if (!m_extInstance)
        return;
    Inspector::CommServer *server = m_extInstance->commServer();
    if (!server) {
        m_viewWidget->setWidget(new QWidget);
        return;
    }

    Internal::InfoDialog *info = new Internal::InfoDialog;
    bool debugging = m_extInstance->debugging();
    info->setFieldState(info->debLabel, debugging ? 1 : -1);
    info->setFieldState(info->enaButton, server->m_sEnabled ? 1 : -1);
    info->setFieldState(info->hlpLabel, server->m_sHelpers ? 1 : debugging ? -1 : 0);
    info->setFieldState(info->injLabel, server->m_sInjected ? 1 : debugging ? -1 : 0);
    info->setFieldState(info->conLabel, server->m_sConnected ? 1 : debugging ? -1 : 0);
    info->setFieldState(info->workLabel, (debugging && server->m_sEnabled && server->m_sInjected && server->m_sConnected) ? 1 : 0);

    m_viewWidget->setWidget(info);

/*
    QWidget *w = new QWidget;
    Ui::CommView * ui = new Ui::CommView;
    ui->setupUi(w);
    m_viewWidget->setWidget(w);
*/
}

void InspectorFrame::showSubSelectorView()
{
    QWidget *holder = new QWidget;
    QLabel *arrowLabel = new QLabel(holder);
    QPixmap pix(":/inspector/images/submenu-up.png");
    arrowLabel->setPixmap(pix);
    arrowLabel->setFixedSize(pix.size());
    QPropertyAnimation * ani = new QPropertyAnimation(arrowLabel, "pos");
    ani->setEasingCurve(QEasingCurve::OutElastic);
    ani->setDuration(800);
    ani->setEndValue(200 /*QPoint(m_subCombo->x() + (m_subCombo->width() - pix.width()) / 2, 0)*/);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
    m_viewWidget->setWidget(holder);
}

void InspectorFrame::slotMenuChanged(const QStringList &path, const QVariant &data)
{
    Q_UNUSED(path)
    quint32 compoId = data.toInt();
    int probeId = compoId >> 8;
    int viewId = compoId & 0xFF;
    //m_extInstance->probeController()->

}

} // namespace Internal
} // namespace Inspector
