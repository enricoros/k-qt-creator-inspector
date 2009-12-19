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

#include "combotreewidget.h"

#include <QHBoxLayout>

using namespace Inspector::Internal;

ComboTreeWidget::ComboTreeWidget(QWidget *parent)
  : Utils::StyledBar(parent)
{
    // set the horizontal layout
    QHBoxLayout *hLay = new QHBoxLayout(this);
    hLay->setMargin(0);
    hLay->setSpacing(0);
    setLayout(hLay);

    /*
    m_mainCombo = new QComboBox(toolBar);
    connect(m_mainCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMainComboChanged(int)));
    m_subCombo = new QComboBox(toolBar);
    connect(m_subCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSubComboChanged(int)));
    hLay->addWidget(m_mainCombo);
    //tLayout->addWidget(new QLabel(tr(" section "), toolBar));
    hLay->addWidget(m_subCombo);
    hLay->addStretch(10);
    */
}

void ComboTreeWidget::addPath(const QStringList &path, const QVariant &userData, const QIcon &icon)
{
    qWarning("ComboTreeWidget::addPath: TODO");
    Q_UNUSED(path);
    Q_UNUSED(userData);
    Q_UNUSED(icon);
}

void ComboTreeWidget::removePath(const QStringList &path)
{
    qWarning("ComboTreeWidget::removePath: TODO");
    Q_UNUSED(path);
}

void ComboTreeWidget::clear()
{
    // TODO: remove all the combos...

    // TODO: add the default menu item wiht data (quint32)0x00
    /*int prevIndex = m_mainCombo->currentIndex();
    int prevProbeId = prevIndex >= 0 ? m_mainCombo->itemData(prevIndex).toInt() : 0;
    m_mainCombo->clear();
    m_mainCombo->addItem(QIcon(":/inspector/images/menu-icon.png"), tr("Status"));
    */
}

QStringList ComboTreeWidget::currentPath() const
{
    qWarning("ComboTreeWidget::currentPath: TODO");
    return QStringList();
}

QVariant ComboTreeWidget::currentData() const
{
    qWarning("ComboTreeWidget::currentData: TODO");
    return 42;
}

void ComboTreeWidget::setPath(const QStringList &path)
{
    qWarning("ComboTreeWidget::setPath: TODO");
    Q_UNUSED(path);
}

void ComboTreeWidget::slotComboIndexChanged(int index)
{
    qWarning("ComboTreeWidget::slotComboIndexChanged: TODO %d", index);
}

/*
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
    ProbeMenuItem subMenu = m_mergedMenu.at(mainIndex);
    if (!subMenu.children.isEmpty()) {
        foreach (const ProbeMenuItem & item, subMenu.children) {
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
    const ProbeMenuItem & menu = m_mergedMenu.at(mainIndex);
    if (subIndex < 0 || subIndex >= menu.children.count())
        return;
    const ProbeMenuItem & item = menu.children.at(subIndex);

    // activate the related view
    activateView(item.testId, item.viewId);
}
*/
