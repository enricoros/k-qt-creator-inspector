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

#include "performancewindow.h"

#include <utils/styledbar.h>
#include "infoview.h"
#include "performancemanager.h"
#include "performanceserver.h"
#include "ptview.h"
#include "taskbarwidget.h"
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

namespace Performance {
namespace Internal {

class ViewContainerWidget : public QWidget
{
    public:
        ViewContainerWidget(QWidget * parent = 0)
          : QWidget(parent)
          , m_widget(0)
        {
            // precache watermark pixmap
            QSvgRenderer wmRender(QString(":/performance/images/probe-watermark.svg"));
            if (wmRender.isValid()) {
                m_watermarkPixmap = QPixmap(wmRender.defaultSize());
                m_watermarkPixmap.fill(Qt::transparent);
                QPainter wmPainter(&m_watermarkPixmap);
                wmRender.render(&wmPainter);
            }

            // set a vertical layout
            QVBoxLayout * lay = new QVBoxLayout(this);
            setLayout(lay);
        }

        void setWidget(QWidget * widget)
        {
            delete m_widget;
            m_widget = widget;
            if (widget) {
                widget->setParent(this);
                layout()->addWidget(widget);
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
};

PerformanceWindow::PerformanceWindow(QWidget *parent)
  : QWidget(parent)
  , m_viewWidget(0)
{
    // ToolBar
    QWidget *toolBar = new Utils::StyledBar(this);
    QHBoxLayout *tLayout = new QHBoxLayout(toolBar);
    tLayout->setMargin(0);
    tLayout->setSpacing(0);

    m_mainCombo = new QComboBox(toolBar);
    connect(m_mainCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMainChanged(int)));
    m_subCombo = new QComboBox(toolBar);
    connect(m_subCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSubChanged(int)));
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

    updateMainCombo(true);
}

PerformanceWindow::~PerformanceWindow()
{
}

void PerformanceWindow::slotMainChanged(int choice)
{
    // reset subcombo
    m_subCombo->hide();
    m_subCombo->clear();

    // main category selected
    int id = m_mainCombo->itemData(choice, Qt::UserRole).toInt();
    switch (id) {
        case 1:
            m_subCombo->addItem(tr("Information"), 101);
            m_subCombo->addItem(tr("Debugging"), 102);
            m_subCombo->adjustSize();
            m_subCombo->show();
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            m_subCombo->addItem(tr("..."));
            m_subCombo->addItem(tr("Temperature"), 401);
            m_subCombo->addItem(tr("Pixel Energy"), 402);
            m_subCombo->adjustSize();
            m_subCombo->show();
            activateSubSelector();
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
    }
}

void PerformanceWindow::slotSubChanged(int choice)
{
    int id = m_subCombo->itemData(choice, Qt::UserRole).toInt();
    switch (id) {
        case 101:
            activateRInformation();
            break;
        case 102:
            activateRDebugging();
            break;
        case 401:
            activatePaintingTemperature();
            break;
        default:
            activateSubSelector();
            break;
    }
}

void PerformanceWindow::activateRInformation()
{
    Performance::PerformanceServer *server = Performance::PerformanceManager::instance()->defaultServer();
    if (!server) {
        m_viewWidget->setWidget(new QWidget);
        return;
    }

    Internal::InfoView *info = new Internal::InfoView;
    info->setFieldState(info->debLabel, server->m_sDebugging ? 1 : -1);
    info->setFieldState(info->enaButton, server->m_sEnabled ? 1 : -1);
    info->setFieldState(info->hlpLabel, server->m_sHelpers ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->injLabel, server->m_sInjected ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->conLabel, server->m_sConnected ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->workLabel, (server->m_sDebugging && server->m_sEnabled && server->m_sInjected && server->m_sConnected) ? 1 : 0);

    m_viewWidget->setWidget(info);
}

void PerformanceWindow::activatePaintingTemperature()
{
    PaintTemperatureView * ptView = new PaintTemperatureView;
    m_viewWidget->setWidget(ptView);
}


void PerformanceWindow::activateRDebugging()
{
    QWidget *w = new QWidget;
    Ui::CommView * ui = new Ui::CommView;
    ui->setupUi(w);
    m_viewWidget->setWidget(w);
}

void PerformanceWindow::activateSubSelector()
{
    QWidget *holder = new QWidget;
    QLabel *arrowLabel = new QLabel(holder);
    QPixmap pix(":/performance/images/submenu-up.png");
    arrowLabel->setPixmap(pix);
    arrowLabel->setFixedSize(pix.size());
    QPropertyAnimation * ani = new QPropertyAnimation(arrowLabel, "pos");
    ani->setEasingCurve(QEasingCurve::OutElastic);
    ani->setDuration(800);
    ani->setEndValue(QPoint(m_subCombo->x() + (m_subCombo->width() - pix.width()) / 2, 0));
    ani->start(QPropertyAnimation::DeleteWhenStopped);
    m_viewWidget->setWidget(holder);
}

void PerformanceWindow::updateMainCombo(bool enabled)
{
    int prevIdx = m_mainCombo->currentIndex();
    m_mainCombo->clear();
    m_mainCombo->addItem(tr("Probe"), 1);
    if (enabled) {
        m_mainCombo->addItem(tr("Event Loop"), 2);
        m_mainCombo->addItem(tr("Edit"), 3);
        m_mainCombo->addItem(tr("Painting"), 4);
        m_mainCombo->addItem(tr("Timers"), 5);
        m_mainCombo->addItem(tr("Network"), 6);
        m_mainCombo->addItem(tr("Input"), 7);
        m_mainCombo->addItem(tr("Parallel"), 8);
    }
    if (prevIdx > 0 && prevIdx < m_mainCombo->count())
        m_mainCombo->setCurrentIndex(prevIdx);
    else
        m_mainCombo->setCurrentIndex(0);
}

} // namespace Internal
} // namespace Performance
