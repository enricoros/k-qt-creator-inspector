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

#ifndef INSTANCEMODEL_H
#define INSTANCEMODEL_H

#include <QStandardItemModel>
#include <QVariantList>
class QTableView;

namespace Inspector {

namespace Internal {
}

class Q_DECL_EXPORT InstanceModel : public QStandardItemModel
{
    Q_OBJECT

public:
    InstanceModel(QObject *parent = 0);
    ~InstanceModel();

    enum { InstanceStatus_Row = 1, ProbeStatus_Row = 2, CommServer_Row = 3 };

    // low-level value access
    void setValue(int row, int column, const QVariant &value, int role = Qt::DisplayRole);
    QVariant value(int row, int column = 0, int role = Qt::DisplayRole) const;

    // instance status
    bool debugPaint() const;
    bool instanceEnabled() const;

    // probe status (used by the Debugger plugin)
    void setDebugEnabled(bool);
    void setDebugStopped(bool);
    void setProbePresent(bool);
    void setProbeInjected(bool);
    void setProbeActive(bool);
    // startup params (used by the Debugger plugin)
    QString localServerName() const;
    int probeActivationFlags() const;

    // temp model-function? maybe better in CommServer? ###
    bool callProbeFunction(const QString & name, QVariantList args = QVariantList());

public slots:
    void setDebugPaint(bool);
    void setInstanceEnabled(bool);

signals:
    void debuggerCallFunction(const QString & name, QVariantList args);

private:
    void openDebugWidget();
    QTableView *m_debugView;
};

} // namespace Inspector

#endif // INSTANCEMODEL_H
