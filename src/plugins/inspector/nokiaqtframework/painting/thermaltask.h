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

#ifndef THERMALTASK_H
#define THERMALTASK_H

#include "iframeworktask.h"
#include <QtCore/QDateTime>
#include <QtCore/QVariantList>
#include <QtGui/QImage>

namespace Inspector {
namespace Internal {

class NokiaQtFramework;
class ThermalModel;

/**
  \brief Handles a Thermal test
*/
class ThermalTask : public IFrameworkTask
{
    Q_OBJECT

public:
    ThermalTask(NokiaQtFramework *, ThermalModel *, const QVariantList &options,
                const QString &testTitle, QObject *parent = 0);

    // ::IFrameworkTask
    QString displayName() const;
    void activateTask();
    void deactivateTask();

private slots:
    void slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data);

private:
    NokiaQtFramework *m_framework;
    ThermalModel *m_model;
    QVariantList m_options;
    QString m_testTitle;

    QString m_optionsString;
    QDateTime m_startDate;
    QImage m_lastImage;
};

} // namespace Internal
} // namespace Inspector

#endif // THERMALTASK_H
