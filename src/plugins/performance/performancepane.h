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
#include <QPixmap>
#include <QPlainTextEdit>

namespace Performance {
namespace Internal {
class PerformanceWindow;

class PerformancePane : public Core::IOutputPane
{
    Q_OBJECT

    public:
        PerformancePane(QObject *parent = 0);
        ~PerformancePane() {}

        PerformanceWindow * defaultWindow() const;

        // ::Core::IOutputPane
        QWidget *outputWidget(QWidget *parent);
        QList<QWidget*> toolBarWidgets() const { return QList<QWidget *>(); }
        QString name() const { return tr("Performance Analyzer"); }

        // -1 don't show in statusBar (100...0 show at front...end)
        int priorityInStatusBar() const {return -1;}

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
        PerformanceWindow * m_widget;
};

class PerformanceMiniWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal signOpacity READ signOpacity WRITE setSignOpacity)

public:
    PerformanceMiniWidget(QWidget * parent = 0);

    void addWarning();
    void clearWarnings();

    // ::QWidget
    void paintEvent(QPaintEvent * event);
    void mousePressEvent(QMouseEvent * event);

signals:
    void clicked();

private:
    qreal signOpacity() const;
    void setSignOpacity(qreal opacity);
    qreal m_signOpacity;
    QPixmap m_pixmap;
    int m_cWarnings;
};

} // namespace Internal
} // namespace Performance

#endif
