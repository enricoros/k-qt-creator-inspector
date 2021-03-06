/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qmlpropertychangesnodeinstance.h"
#include "qmlstatenodeinstance.h"
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeExpression>
#include <private/qdeclarativebinding_p.h>
#include <metainfo.h>

namespace QmlDesigner {
namespace Internal {

QmlPropertyChangesObject::QmlPropertyChangesObject() :
        QDeclarativeStateOperation(),
        m_restoreEntryValues(true),
        m_isExplicit(false)
{
}

QDeclarativeStateOperation::ActionList QmlPropertyChangesObject::actions()
{
    ActionList list;

    foreach (const QString &property, m_properties.keys()) {
        QDeclarativeAction a(m_targetObject.data(), property, m_properties.value(property));

        if (a.property.isValid()) {
            a.restore = restoreEntryValues();

            if (a.property.propertyType() == QVariant::Url &&
                (a.toValue.type() == QVariant::String || a.toValue.type() == QVariant::ByteArray) && !a.toValue.isNull())
                a.toValue.setValue(qmlContext(this)->resolvedUrl(QUrl(a.toValue.toString())));

            list << a;
        }
    }

//    for (int ii = 0; ii < d->signalReplacements.count(); ++ii) {
//
//        QmlReplaceSignalHandler *handler = d->signalReplacements.at(ii);
//
//        if (handler->property.isValid()) {
//            Action a;
//            a.event = handler;
//            list << a;
//        }
//    }

    foreach (const QString &property, m_expressions.keys()) {
        QDeclarativeProperty mProperty = metaProperty(property);

        if (mProperty.isValid()) {
            QDeclarativeAction a;
            a.restore = restoreEntryValues();
            a.property = mProperty;
            a.fromValue = a.property.read();
            a.specifiedObject = m_targetObject.data();
            a.specifiedProperty = property;

            if (m_isExplicit) {
                a.toValue = QDeclarativeExpression(qmlContext(object()), m_expressions.value(property), object()).value();
            } else {
                QDeclarativeBinding *newBinding = new QDeclarativeBinding(m_expressions.value(property), object(), qmlContext(object()));
                newBinding->setTarget(mProperty);
                a.toBinding = newBinding;
                a.deletableToBinding = true;
            }

            list << a;
        }
    }

    return list;
}

QDeclarativeProperty QmlPropertyChangesObject::metaProperty(const QString &property)
{
    QDeclarativeProperty prop(m_targetObject.data(), property);
    if (!prop.isValid()) {
        qWarning() << "Cannot assign to non-existent property" << property;
        return QDeclarativeProperty();
    } else if (!prop.isWritable()) {
        qWarning() << "Cannot assign to read-only property" << property;
        return QDeclarativeProperty();
    }
    return prop;
}

QmlPropertyChangesNodeInstance::QmlPropertyChangesNodeInstance(QmlPropertyChangesObject *propertyChangesObject) :
        ObjectNodeInstance(propertyChangesObject)
{
}

QmlPropertyChangesNodeInstance::Pointer
        QmlPropertyChangesNodeInstance::create(const NodeMetaInfo & /*metaInfo*/,
                                               QDeclarativeContext *context,
                                               QObject *objectToBeWrapped)
{
    Q_ASSERT(!objectToBeWrapped);

    QmlPropertyChangesObject *object = new QmlPropertyChangesObject;
    QDeclarativeEngine::setContextForObject(object, context);
    Pointer instance(new QmlPropertyChangesNodeInstance(object));
    return instance;
}

void QmlPropertyChangesNodeInstance::setPropertyVariant(const QString &name, const QVariant &value)
{
    QMetaObject metaObject = QmlPropertyChangesObject::staticMetaObject;

    if (metaObject.indexOfProperty(name.toLatin1()) > 0) { // 'restoreEntryValues', 'explicit'
        ObjectNodeInstance::setPropertyVariant(name, value);
        return;
    } else {
        changesObject()->m_properties.insert(name, value);       updateStateInstance();
    }
}

void QmlPropertyChangesNodeInstance::setPropertyBinding(const QString &name, const QString &expression)
{
    QMetaObject metaObject = QmlPropertyChangesObject::staticMetaObject;

    if (metaObject.indexOfProperty(name.toLatin1()) > 0) { // 'restoreEntryValues', 'explicit'
        ObjectNodeInstance::setPropertyBinding(name, expression);
        return;
    } else {
        changesObject()->m_expressions.insert(name, expression);
        updateStateInstance();
    }
}

QVariant QmlPropertyChangesNodeInstance::property(const QString &name) const
{
    if (changesObject()->m_properties.contains(name))
        return changesObject()->m_properties.value(name);
    if (changesObject()->m_expressions.contains(name))
        return changesObject()->m_expressions.value(name);

    return QVariant();
}

void QmlPropertyChangesNodeInstance::resetProperty(const QString &name)
{
    if (changesObject()->m_properties.contains(name))
        changesObject()->m_properties.remove(name);
    else if (changesObject()->m_expressions.contains(name))
        changesObject()->m_expressions.remove(name);
    // TODO: How to force states object to update?

    updateStateInstance();
}

QmlPropertyChangesObject *QmlPropertyChangesNodeInstance::changesObject() const
{
    Q_ASSERT(qobject_cast<QmlPropertyChangesObject*>(object()));
    return static_cast<QmlPropertyChangesObject*>(object());
}

void QmlPropertyChangesNodeInstance::updateStateInstance() const
{
    if (!nodeInstanceView()->hasInstanceForNode(modelNode()))
        return;

    NodeInstance myInstance = nodeInstanceView()->instanceForNode(modelNode());
    Q_ASSERT(myInstance.isValid());

    NodeInstance qmlStateInstance = myInstance.parent();
    if (!qmlStateInstance.isValid()
        || !qmlStateInstance.modelNode().isValid())
        return;

    qmlStateInstance.refreshState();
}

} // namespace Internal
} // namespace QmlDesigner
