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

#ifndef INSPECTORFRAME_H
#define INSPECTORFRAME_H

#include <QWidget>
#include "abstractprobe.h"
#include "inspectorinstance.h"
class QComboBox;
class QVBoxLayout;

namespace Inspector {
namespace Internal {
class TaskbarWidget;
class ProbeController;
class ViewContainerWidget;

class InspectorFrame : public QWidget
{
    Q_OBJECT

public:
    InspectorFrame(ProbeController * control, QWidget *parent = 0);

    // the instance of which we'll show the Probes
    void setInstance(InspectorInstance * instance);

    /// displays the only hardcoded view. all the others are plugged in
    void showDefaultView();
    void showSubSelectorView();


private slots:
    void updateMainCombo();
    void slotMainComboChanged(int);
    void slotSubComboChanged(int);

private:
    void activateView(int testId, int viewId);

    ProbeController *m_probeController;
    QComboBox *m_mainCombo;
    QComboBox *m_subCombo;
    ViewContainerWidget *m_viewWidget;
    TaskbarWidget *m_taskbarWidget;
    TestMenu m_mergedMenu;
};

} // namespace Internal
} // namespace Inspector

#endif // INSPECTORFRAME_H
