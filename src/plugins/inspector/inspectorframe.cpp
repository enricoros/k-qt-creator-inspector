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

#include <utils/styledbar.h>
#include "commserver.h"
#include "infodialog.h"
#include "inspectorinstance.h"
#include "taskbarwidget.h"
#include "probecontroller.h"
#include "ui_commview.h"

#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
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

InspectorFrame::InspectorFrame(ProbeController *control, QWidget *parent)
  : QWidget(parent)
  , m_probeController(control)
  , m_viewWidget(0)
  , m_taskbarWidget(0)
{
    // ToolBar
    QWidget *toolBar = new Utils::StyledBar(this);
    QHBoxLayout *tLayout = new QHBoxLayout(toolBar);
    tLayout->setMargin(0);
    tLayout->setSpacing(0);

    m_mainCombo = new QComboBox(toolBar);
    connect(m_mainCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMainComboChanged(int)));
    m_subCombo = new QComboBox(toolBar);
    connect(m_subCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSubComboChanged(int)));
    tLayout->addWidget(m_mainCombo);
    //tLayout->addWidget(new QLabel(tr(" section "), toolBar));
    tLayout->addWidget(m_subCombo);
    tLayout->addStretch(10);

    m_viewWidget = new ViewContainerWidget(this);

    m_taskbarWidget = new TaskbarWidget(this);

    // Main Layout
    QVBoxLayout * vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addWidget(toolBar);
    vLayout->addWidget(m_viewWidget);
    vLayout->addWidget(m_taskbarWidget);

    updateMainCombo();
}

void InspectorFrame::setInstance(InspectorInstance * /*instance*/)
{
    qWarning("InspectorFrame::setInstance: TODO!!");
}

void InspectorFrame::showDefaultView()
{
    Inspector::CommServer *server = Inspector::InspectorInstance::instance()->defaultComm();
    if (!server) {
        m_viewWidget->setWidget(new QWidget);
        return;
    }

    Internal::InfoDialog *info = new Internal::InfoDialog;
    info->setFieldState(info->debLabel, server->m_sDebugging ? 1 : -1);
    info->setFieldState(info->enaButton, server->m_sEnabled ? 1 : -1);
    info->setFieldState(info->hlpLabel, server->m_sHelpers ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->injLabel, server->m_sInjected ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->conLabel, server->m_sConnected ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->workLabel, (server->m_sDebugging && server->m_sEnabled && server->m_sInjected && server->m_sConnected) ? 1 : 0);

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
    ani->setEndValue(QPoint(m_subCombo->x() + (m_subCombo->width() - pix.width()) / 2, 0));
    ani->start(QPropertyAnimation::DeleteWhenStopped);
    m_viewWidget->setWidget(holder);
}

void InspectorFrame::updateMainCombo()
{
    // TODO: keep care of the current state while updating: some items may be
    // unavailable

    // regen the combo
    int prevIndex = m_mainCombo->currentIndex();
    int prevTestId = prevIndex >= 0 ? m_mainCombo->itemData(prevIndex).toInt() : 0;
    m_mainCombo->clear();
    m_mainCombo->addItem(QIcon(":/inspector/images/menu-icon.png"), tr("Status"));

    // add all the top-level items
    m_mergedMenu = m_probeController->mergedMenu();
    foreach (const TestMenuItem & item, m_mergedMenu) {
        if (item.enabled) {
            m_mainCombo->addItem(item.name);
            if (prevTestId == item.testId)
                m_mainCombo->setCurrentIndex(m_mainCombo->count() - 1);
        }
    }

    // select the first item, if no other selected
    if (m_mainCombo->currentIndex() < 0)
        m_mainCombo->setCurrentIndex(0);
}

void InspectorFrame::slotMainComboChanged(int mainIndex)
{
    // reset subcombo
    m_subCombo->hide();
    m_subCombo->clear();

    // handle deault view
    if (!mainIndex) {
        showDefaultView();
        return;
    }
    --mainIndex;

    // handle the selection on the main combo
    TestMenuItem subMenu = m_mergedMenu.at(mainIndex);
    if (!subMenu.children.isEmpty()) {
        foreach (const TestMenuItem & item, subMenu.children) {
            if (item.enabled)
                m_subCombo->addItem(item.name);
        }
        if (m_subCombo->count()) {
            m_subCombo->adjustSize();
            m_subCombo->show();
        }
    } else if (subMenu.testId) {
        activateView(subMenu.testId, subMenu.viewId);
    } else
        qWarning("InspectorFrame::slotMainChanged: can't handle this combo selection");
}

void InspectorFrame::slotSubComboChanged(int subIndex)
{
    // get the item
    int mainIndex = m_mainCombo->currentIndex() - 1;
    if (mainIndex < 0 || mainIndex >= m_mergedMenu.count())
        return;
    const TestMenuItem & menu = m_mergedMenu.at(mainIndex);
    if (subIndex < 0 || subIndex >= menu.children.count())
        return;
    const TestMenuItem & item = menu.children.at(subIndex);

    // activate the related view
    activateView(item.testId, item.viewId);
}

void InspectorFrame::activateView(int testId, int viewId)
{
    qWarning("View activation TODO");
}

} // namespace Internal
} // namespace Inspector
