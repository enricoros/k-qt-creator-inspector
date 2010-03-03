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

#ifndef THERMALPANEL_H
#define THERMALPANEL_H

#include "abstractpanel.h"
#include "ui_thermalpanel.h"

namespace Inspector {
namespace Internal {

class PaintingModule;
class ThermalItemDelegate;
class ThermalModel;

class ThermalPanel : public AbstractPanel, public Ui::ThermalPanel
{
    Q_OBJECT

public:
    ThermalPanel(PaintingModule *);

    QString helpHtml() const;

private slots:
    void slotActivatePreset(int comboIndex);
    void slotValidateTestLabel();
    void slotRegenLabel();

    void slotCheckIterations();
    void slotCheckWeight();

    void slotRunThermalClicked();

    void slotDisplayClicked();
    void slotRemoveClicked();
    void slotClearClicked();
    void slotExportClicked();
    void slotImportClicked();

    // model slots
    void slotModelItemChanged();

    // view slots
    void slotViewSelectionChanged();
    void slotDisplayResultImage(const QModelIndex &index);

private:
    PaintingModule *m_paintingModule;
    ThermalModel *m_thermalModel;
};


class Thermal3DOnDemand : public QWidget
{
public:
    Thermal3DOnDemand(PaintingModule *, bool useDepthPeeling, QWidget *parent = 0);

protected:
    void showEvent(QShowEvent *);

private:
    PaintingModule *m_paintingModule;
    bool m_useDepthPeeling;
    bool m_created;
};


} // namespace Internal
} // namespace Inspector

#endif // THERMALPANEL_H
