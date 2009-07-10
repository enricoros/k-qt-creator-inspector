/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *   Started on 10 Jul 2009 by root.                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PERFORMANCEPANE_H
#define PERFORMANCEPANE_H

#include <coreplugin/ioutputpane.h>
#include <QPlainTextEdit>

class PerformancePane : public Core::IOutputPane
{
    Q_OBJECT
    public:
        PerformancePane(QObject *parent = 0);
        ~PerformancePane() {}

        void addString(const QString & string);

        // ::Core::IOutputPane
        QWidget *outputWidget(QWidget *parent);
        QList<QWidget*> toolBarWidgets() const { return QList<QWidget *>(); }
        QString name() const { return tr("Performance Analyzer"); }

        // -1 don't show in statusBar
        // 100...0 show at front...end
        int priorityInStatusBar() const { return 10; }

        void clearContents() {}
        void visibilityChanged(bool /*visible*/) {}

        void setFocus() {}
        bool hasFocus() { return true; }
        // should return true only if there are results here
        bool canFocus() { return true; }

        bool canNavigate() { return true; }
        bool canNext() { return false; }
        bool canPrevious() { return false; }
        void goToNext() {}
        void goToPrev() {}

    private:
        QPlainTextEdit * m_editWidget;
};

#endif
