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

#include "panelcontainerwidget.h"
#include <QtGui/QVBoxLayout>

using namespace Inspector::Internal;

PanelContainerWidget::PanelContainerWidget(QWidget * parent)
  : QWidget(parent)
  , m_widget(0)
  , m_disabled(false)
  //, m_disabledLabel(0)
{
    // create the disabled warning lable
    //m_disabledLabel = new QLabel(tr("This panel is disabled. Probably the panel is not available in the current state."), this);
    //m_disabledLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    //m_disabledLabel->hide();

    // set a vertical layout
    QVBoxLayout * lay = new QVBoxLayout(this);
    lay->setMargin(0);
    lay->setSpacing(0);
    //lay->addWidget(m_disabledLabel);
    setLayout(lay);
}

/*
void PanelContainerWidget::setDisableWidget(bool disabled)
{
    if (disabled != m_disabled) {
        m_disabled = disabled;
        m_disabledLabel->setVisible(disabled);
        if (m_widget)
            m_widget->setEnabled(!m_disabled);
    }
}
*/

void PanelContainerWidget::setPanel(QWidget * widget)
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
