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

#ifndef INSPECTORCONTAINER_H
#define INSPECTORCONTAINER_H

#include <QtCore/QList>
#include <QtGui/QWidget>
class QStackedWidget;

namespace Inspector {
namespace Internal {

class InspectorWindow;
class Instance;
class SingleTabWidget;
class TargetWindow;

class InspectorContainer : public QWidget
{
    Q_OBJECT

public:
    InspectorContainer(QWidget *parent = 0);

signals:
    void requestWindowDisplay();

private slots:
    void slotInstanceAdded(Instance *);
    void slotInstanceRemoved(Instance *);

    void slotDisplayInspectorWindow();
    void slotDisplayTargetWindow();

private:
    SingleTabWidget *       m_topbarWidget;
    InspectorWindow *       m_inspectorWindow;
    QStackedWidget *        m_centralWidget;
    QList<TargetWindow *>   m_targets;
};

} // namespace Internal
} // namespace Inspector

#endif // INSPECTORCONTAINER_H
