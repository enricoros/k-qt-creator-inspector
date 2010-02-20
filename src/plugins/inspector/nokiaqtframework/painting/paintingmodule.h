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

#ifndef PAINTINGMODULE_H
#define PAINTINGMODULE_H

#include "iframeworkmodule.h"
#include "iframeworktask.h"
#include <QtCore/QVariantList>
#include <QtGui/QImage>

namespace Inspector {
namespace Internal {

// constants
const int UID_MODULE_PAINTING = 2;

class NokiaQtFramework;
class PaintingModel;

class PaintingModule : public IFrameworkModule
{
    Q_OBJECT

public:
    PaintingModule(NokiaQtFramework *, QObject *parent = 0);
    ~PaintingModule();

    PaintingModel *model() const;

    // ::IFrameworkModule
    int uid() const { return UID_MODULE_PAINTING; }
    QString name() const;
    ModuleMenuEntries menuEntries() const;
    AbstractPanel *createPanel(int panelId);

    void startTemperatureTest(const QString &testTitle, const QVariantList &options);

private:
    NokiaQtFramework *m_framework;
    PaintingModel *m_model;
};


/**
  \brief Handles a Painting Temperature test
*/
class PaintingTemperatureTask : public IFrameworkTask
{
    Q_OBJECT

public:
    PaintingTemperatureTask(NokiaQtFramework *,
                            PaintingModel *,
                            const QVariantList &options,
                            const QString &testTitle,
                            QObject *parent = 0);

    // ::IFrameworkTask
    QString displayName() const;
    void activateTask();
    void deactivateTask();

private slots:
    void slotProcessIncomingData(quint32 channel, quint32 code1, QByteArray *data);

private:
    NokiaQtFramework *m_framework;
    PaintingModel *m_model;
    QVariantList m_options;
    QString m_testTitle;

    QImage m_lastImage;
};

} // namespace Internal
} // namespace Inspector

#endif // PAINTINGMODULE_H
