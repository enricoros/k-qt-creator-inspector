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

#include "infopanel.h"
#include "ibackend.h"
#include "ibackendmodule.h"
#include "../nokiaqtinspectionmodel.h"
#include <QtGui/QCheckBox>
#include <QtGui/QFont>

using namespace Inspector;
using namespace Inspector::Internal;

InfoPanel::InfoPanel(IBackendModule *parentModule)
  : AbstractPanel(parentModule)
  , m_okPixmap(":/inspector/images/status-ok.png")
  , m_errorPixmap(":/inspector/images/status-err.png")
{
    setupUi(this);
    QFont smallFont = connName->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    connName->setFont(smallFont);

    // update Plugin data
    modLabel->setText(parentBackend()->loadedModules().join(", "));

    // update Inspection data
    connect(parentBackend()->inspectionModel(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(slotRefreshInspectionData()));
    connect(parentBackend()->inspectionModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(slotRowsInserted(QModelIndex,int,int)));
    // NOTE: THIS IS PAINFULLY SLOW ###
#if 0
    if (QStandardItem *item = parentBackend()->inspectionModel()->item(InspectionModel::CommServer_Row, 8)) {
        int rowCount = item->rowCount();
        for (int row = 0; row < rowCount; ++row)
            commText->appendPlainText(item->child(row)->text());
    }
#endif
    slotRefreshInspectionData();
}

QString InfoPanel::helpHtml() const
{
    return tr("<b>Welcome</b>. You just started a new Inspection.<br/><br/>You can now operate with the connected Target by using the <i>Panels</i> in the above list.");
}

void InfoPanel::slotRefreshInspectionData()
{
    NokiaQtInspectionModel *model = static_cast<NokiaQtInspectionModel *>(parentBackend()->inspectionModel());

    int _debugEnabled = model->itemValue(NokiaQtInspectionModel::ProbeStatus_Row, 1).toInt();
    setFieldState(debugEnabled,     _debugEnabled);
    setFieldState(debugStopped,     model->itemValue(NokiaQtInspectionModel::ProbeStatus_Row, 2).toInt());
    setFieldState(probePresent,     model->itemValue(NokiaQtInspectionModel::ProbeStatus_Row, 4).toInt());
    probeCaps->setText(             model->itemValue(NokiaQtInspectionModel::ProbeStatus_Row, 6).toString());
    setFieldState(probeInjected,    model->itemValue(NokiaQtInspectionModel::ProbeStatus_Row, 5).toInt());
    setFieldState(probeActive,      model->itemValue(NokiaQtInspectionModel::ProbeStatus_Row, 7).toInt());

    bool _connEnabled = model->itemValue(NokiaQtInspectionModel::CommServer_Row, 0).toBool();
    setFieldState(connEnabled,      _connEnabled);
    connName->setText(              model->itemValue(NokiaQtInspectionModel::CommServer_Row, 1).toString());
    setFieldState(serverListening,  model->itemValue(NokiaQtInspectionModel::CommServer_Row, 2).toBool());
    bool _probeConnected = model->itemValue(NokiaQtInspectionModel::CommServer_Row, 3).toBool();
    setFieldState(probeConnected,   _probeConnected);
    clientInfo->setText(            model->itemValue(NokiaQtInspectionModel::CommServer_Row, 4).toString());

    //bool works = _debugEnabled == 1 && _connEnabled && _probeConnected;
    //setFieldState(workLabel,        works);
}

void InfoPanel::slotRowsInserted(const QModelIndex &parent, int start, int end)
{
    IInspectionModel *model = parentBackend()->inspectionModel();
    QStandardItem *parentItem = model->itemFromIndex(parent);

    // log all incoming packets
    if (parentItem && parentItem->row() == NokiaQtInspectionModel::CommServer_Row) {
        for (int row = start; row <= end; ++row) {
            if (parentItem->column() == 6 || parentItem->column() == 7)
                messagesText->appendHtml("<font color='#800'>" + parentItem->child(row)->text() + "</font>");
            else if (parentItem->column() == 8)
                commText->appendPlainText(parentItem->child(row)->text());
        }
    }
}

void InfoPanel::setFieldState(QWidget *field, int state)
{
    if (QCheckBox *c = dynamic_cast<QCheckBox *>(field)) {
        c->setChecked(state == 1);
        return;
    }

    if (QAbstractButton *b = dynamic_cast<QAbstractButton *>(field)) {
        switch (state) {
        case -1:
            b->setText(tr("?"));
            break;
        case 0:
            b->setIcon(m_errorPixmap);
            if (b->isCheckable())
                b->setChecked(false);
            break;
        case 1:
            b->setIcon(m_okPixmap);
            if (b->isCheckable())
                b->setChecked(true);
            break;
        }
    }

    if (QLabel *l = dynamic_cast<QLabel *>(field)) {
        switch (state) {
        case -1:
            l->setText(tr("unknown"));
            break;
        case 0:
            l->setPixmap(m_errorPixmap);
            break;
        case 1:
            l->setPixmap(m_okPixmap);
            break;
        }
    }
}
