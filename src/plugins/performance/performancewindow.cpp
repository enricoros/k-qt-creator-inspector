/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 14 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "performancewindow.h"

#include <utils/styledbar.h>

#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QGradient>
#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QSvgRenderer>

using namespace Performance::Internal;

PerformanceWindow::PerformanceWindow(QWidget *parent)
  : QWidget(parent)
  , m_centralWidget(0)
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

    // Main Layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(toolBar);

    updateMainCombo(true);

    // precache watermark pixmap
    QSvgRenderer wmRender(QString(":/performance/images/probe-watermark.svg"));
    if (wmRender.isValid()) {
        m_watermarkPixmap = QPixmap(wmRender.defaultSize());
        m_watermarkPixmap.fill(Qt::transparent);
        QPainter wmPainter(&m_watermarkPixmap);
        wmRender.render(&wmPainter);
    }
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
            activatePainting(1);
            break;
        case 402:
            activatePainting(2);
            break;
        default:
            activateSubSelector();
            break;
    }
}

void PerformanceWindow::paintEvent(QPaintEvent * event)
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

#include "performanceinformation.h"
#include "performancemanager.h"
#include "performanceserver.h"
void PerformanceWindow::activateRInformation()
{
    Performance::PerformanceServer *server = Performance::PerformanceManager::instance()->defaultServer();
    if (!server) {
        setCentralWidget(new QWidget);
        return;
    }

    Internal::PerformanceInformation *info = new Internal::PerformanceInformation;
    info->setFieldState(info->debLabel, server->m_sDebugging ? 1 : -1);
    info->setFieldState(info->enaButton, server->m_sEnabled ? 1 : -1);
    info->setFieldState(info->hlpLabel, server->m_sHelpers ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->injLabel, server->m_sInjected ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->conLabel, server->m_sConnected ? 1 : server->m_sDebugging ? -1 : 0);
    info->setFieldState(info->workLabel, (server->m_sDebugging && server->m_sEnabled && server->m_sInjected && server->m_sConnected) ? 1 : 0);

    setCentralWidget(info);
}

#include "ui_servicewindow.h"
void PerformanceWindow::activateRDebugging()
{
    QWidget *w = new QWidget;
    Ui::ServiceWindow * ui = new Ui::ServiceWindow;
    ui->setupUi(w);
    setCentralWidget(w);
}

void PerformanceWindow::activatePainting(int /*subChoice*/)
{
    setCentralWidget(new QWidget);
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
    setCentralWidget(holder);
}


void PerformanceWindow::setCentralWidget(QWidget * widget)
{
    delete m_centralWidget;
    m_centralWidget = widget;
    if (m_centralWidget)
        m_mainLayout->addWidget(m_centralWidget, 10);
}

void PerformanceWindow::updateMainCombo(bool enabled)
{
    int prevIdx = m_mainCombo->currentIndex();
    m_mainCombo->clear();
    m_mainCombo->addItem(tr("Runtime"), 1);
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
