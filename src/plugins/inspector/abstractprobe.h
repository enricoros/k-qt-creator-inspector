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

#ifndef ABSTRACTPROBE_H
#define ABSTRACTPROBE_H

#include <QObject>
class QWidget;

namespace Inspector {
namespace Internal {
class ProbeController;

struct TestMenuItem {
    QString name;
    bool enabled;
    int testId;
    int viewId;
    QList<TestMenuItem> children;

    TestMenuItem(const QString & name, bool enabled, int testId, int viewId)
        : name(name), enabled(enabled), testId(testId), viewId(viewId) {}
};

typedef QList<TestMenuItem> TestMenu;

class AbstractTest : public QObject
{
    Q_OBJECT

public:
    AbstractTest(QObject *parent = 0);
    virtual ~AbstractTest();

    // describe the Test
    enum { Uid = 0x00 };
    virtual QString name() const = 0;
    virtual TestMenu menu() const = 0;
    //virtual QList<int> cmdClasses() const = 0;
    virtual QWidget * createView(int viewId) = 0;
    //virtual * createCommSession(int cmdClass) = 0;

signals:
    // tells TestContol to activate this test
    void requestActivation();
    // tells ProbeController that this test is idle again
    void deactivated();

protected slots:
    // reimplement to be notified when the state changes
    virtual void slotActivate();
    virtual void slotDeactivate();
    virtual void slotLock();
    virtual void slotUnlock();

private:
    // used by ProbeController for state transitions
    friend class ProbeController;
    void controlActivate();
    void controlDeactivate();
    void controlRefuse();
    void controlWait();
    struct AbstractTestPrivate * d;
};

} // namespace Internal
} // namespace Inspector

#endif // ABSTRACTPROBE_H
