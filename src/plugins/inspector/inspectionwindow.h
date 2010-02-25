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

#ifndef INSPECTIONWINDOW_H
#define INSPECTIONWINDOW_H

#include <QtGui/QTextBrowser>
#include <QtGui/QWidget>
class QComboBox;
class QLabel;
class QPushButton;
class QStackedWidget;

namespace Core {
class MiniSplitter;
}

namespace Inspector {
namespace Internal {

class Inspection;
class InspectionWindowSidebar;
class ModuleMenuWidget;
class PanelContainerWidget;
class PanelInfoLabel;
class SideHelpWidget;
class StatusBarWidget;


class InspectionWindow : public QWidget
{
    Q_OBJECT

public:
    InspectionWindow(Inspection *inspection, QWidget *parent = 0);

    Inspection *inspection() const;

signals:
    void requestInspectionDisplay();

private slots:
    void slotCloseInspection();
    void slotShowPanel(quint32 compoId);
    void slotSetCurrentPanel(int moduleUid, int panelId);
    void slotFrameworkConnected(bool connected);

private:
    void setInspection(Inspection *inspection);
    void showPanel(int moduleUid, int panelId);
    Inspection *                m_inspection;
    PanelInfoLabel *            m_panelInfoLabel;
    PanelContainerWidget *      m_panelContainer;
    StatusBarWidget *           m_statusbarWidget;
    // side panels
    ModuleMenuWidget *          m_modulesMenu;
    SideHelpWidget *            m_sideHelp;
};


class PanelInfoLabel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int fixedHeight READ maximumHeight WRITE setFixedHeight)

public:
    PanelInfoLabel(QWidget *parent = 0);

    void setContents(const QString &, bool showClose);
    void hideContents();

signals:
    void buttonClicked();

protected:
    void paintEvent(QPaintEvent *);

private:
    void animateHeight(int from, int to, bool hideAtEnd);
    QLabel *m_label;
    QPushButton *m_closeButton;
};


class InspectionWindowSidebar : public QWidget
{
    Q_OBJECT

public:
    InspectionWindowSidebar(QWidget *parent = 0);

    void addWidget(const QString &label, QWidget *widget);

private:
    QComboBox *m_navigationComboBox;
    QStackedWidget *m_stack;
};


class SideHelpWidget : public QTextBrowser
{
public:
    SideHelpWidget(QWidget *parent = 0);

    void setHelpHtml(const QString &data);
    void clearHelp();
};

} // namespace Internal
} // namespace Inspector

#endif // INSPECTIONWINDOW_H
