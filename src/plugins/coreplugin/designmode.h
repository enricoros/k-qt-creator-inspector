/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef DESIGNMODE_H
#define DESIGNMODE_H

#include <coreplugin/imode.h>
#include <coreplugin/icorelistener.h>
#include <coreplugin/editormanager/ieditor.h>

#include <QWeakPointer>
#include <QPair>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QAction;
class QStackedWidget;
QT_END_NAMESPACE

namespace Core {
class EditorManager;
class DesignMode;

namespace Internal {

class DesignModeCoreListener : public Core::ICoreListener
{
    Q_OBJECT
public:
    DesignModeCoreListener(DesignMode* mode);
    bool coreAboutToClose();
private:
    DesignMode *m_mode;
};

} // namespace Internal

/**
  * A global mode for Design pane - used by Bauhaus (QML Designer) and
  * Qt Designer. Other plugins can register themselves by registerDesignWidget()
  * and giving a list of mimetypes that the editor understands, as well as an instance
  * to the main editor widget itself.
  */
class CORE_EXPORT DesignMode : public Core::IMode
{
    Q_OBJECT

public:
    DesignMode(EditorManager *editorManager);
    ~DesignMode();

    void registerDesignWidget(QWidget *widget, const QStringList &mimeTypes,
                              bool preferDesignMode = false);
    void unregisterDesignWidget(QWidget *widget);
    // IContext
    QList<int> context() const;
    QWidget *widget();

    // IMode
    QString displayName() const;
    QIcon icon() const;
    int priority() const;
    QString id() const;

signals:
    void actionsUpdated(Core::IEditor *editor);

private slots:
    void currentEditorChanged(Core::IEditor *editor);
    void updateActions();

private:
    Internal::DesignModeCoreListener *m_coreListener;
    QWeakPointer<Core::IEditor> m_currentEditor;
    bool m_isActive;

    struct DesignEditorInfo {
        int widgetIndex;
        QStringList mimeTypes;
        bool preferredMode;
        QWidget *widget;
    };

    QList<DesignEditorInfo*> m_editors;

    EditorManager *m_editorManager;
    QStackedWidget *m_stackWidget;

    friend class Internal::DesignModeCoreListener;
};


} // namespace Core

#endif // DESIGNMODE_H
