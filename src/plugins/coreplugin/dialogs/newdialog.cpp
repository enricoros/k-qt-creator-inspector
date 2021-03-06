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

#include "newdialog.h"
#include "ui_newdialog.h"
#include "basefilewizard.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/dialogs/iwizard.h>

#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtCore/QDebug>

Q_DECLARE_METATYPE(Core::IWizard*)

static inline Core::IWizard *wizardOfItem(const QTreeWidgetItem *item = 0)
{
    if (!item)
        return 0;
    return qVariantValue<Core::IWizard*>(item->data(0, Qt::UserRole));
}

using namespace Core;
using namespace Core::Internal;

NewDialog::NewDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Core::Internal::Ui::NewDialog),
    m_okButton(0),
    m_preferredWizardKinds(0)
{
    typedef QMap<QString, QTreeWidgetItem *> CategoryItemMap;
    m_ui->setupUi(this);
    m_okButton = m_ui->buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setDefault(true);

    m_ui->templatesTree->header()->hide();
    connect(m_ui->templatesTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
        this, SLOT(currentItemChanged(QTreeWidgetItem*)));
    connect(m_ui->templatesTree, SIGNAL(itemActivated(QTreeWidgetItem*,int)), m_okButton, SLOT(animateClick()));

    connect(m_okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

// Sort by category. id
bool wizardLessThan(const IWizard *w1, const IWizard *w2)
{
    if (const int cc = w1->category().compare(w2->category()))
        return cc < 0;
    return w1->id().compare(w2->id()) < 0;
}

void NewDialog::setPreferredWizardKinds(IWizard::WizardKinds kinds)
{
    m_preferredWizardKinds = kinds;
}

void NewDialog::setWizards(QList<IWizard*> wizards)
{
    typedef QMap<QString, QTreeWidgetItem *> CategoryItemMap;

    qStableSort(wizards.begin(), wizards.end(), wizardLessThan);

    CategoryItemMap categories;
    QVariant wizardPtr;

    m_ui->templatesTree->clear();
    foreach (IWizard *wizard, wizards) {
        // ensure category root
        const QString categoryName = wizard->category();
        CategoryItemMap::iterator cit = categories.find(categoryName);
        if (cit == categories.end()) {
            QTreeWidgetItem *categoryItem = new QTreeWidgetItem(m_ui->templatesTree);
            categoryItem->setFlags(Qt::ItemIsEnabled);
            categoryItem->setText(0, wizard->displayCategory());
            qVariantSetValue<IWizard*>(wizardPtr, 0);
            categoryItem->setData(0, Qt::UserRole, wizardPtr);
            cit = categories.insert(categoryName, categoryItem);
        }
        // add item
        QTreeWidgetItem *wizardItem = new QTreeWidgetItem(cit.value(), QStringList(wizard->displayName()));
        wizardItem->setIcon(0, wizard->icon());
        qVariantSetValue<IWizard*>(wizardPtr, wizard);
        wizardItem->setData(0, Qt::UserRole, wizardPtr);
        wizardItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
    }
}

Core::IWizard *NewDialog::showDialog()
{
    QTreeWidgetItem *itemToSelect = 0;
    if (m_preferredWizardKinds == 0) {
        m_ui->templatesTree->expandAll();
        if (QTreeWidgetItem *rootItem = m_ui->templatesTree->topLevelItem(0)) {
            if (rootItem->childCount())
                itemToSelect = rootItem->child(0);
        }
    } else {
        for (int i = 0; i < m_ui->templatesTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem *category = m_ui->templatesTree->topLevelItem(i);
            bool hasOnlyPreferred = true;
            for (int j = 0; j < category->childCount(); ++j) {
                QTreeWidgetItem *item = category->child(j);
                if (!(item->data(0, Qt::UserRole).value<IWizard*>()
                        ->kind() & m_preferredWizardKinds)) {
                    hasOnlyPreferred = false;
                    break;
                }
            }
            category->setExpanded(hasOnlyPreferred);
            if (hasOnlyPreferred && itemToSelect == 0 && category->childCount() > 0) {
                itemToSelect = category->child(0);
            }
        }
    }
    if (itemToSelect) {
        m_ui->templatesTree->scrollToItem(itemToSelect);
        m_ui->templatesTree->setCurrentItem(itemToSelect);
    }
    updateOkButton();
    if (exec() != Accepted)
        return 0;
    return currentWizard();
}

NewDialog::~NewDialog()
{
    delete m_ui;
}

IWizard *NewDialog::currentWizard() const
{
    return wizardOfItem(m_ui->templatesTree->currentItem());
}

void NewDialog::currentItemChanged(QTreeWidgetItem *cat)
{

    if (const IWizard *wizard = wizardOfItem(cat))
        m_ui->descLabel->setText(wizard->description());
    else
        m_ui->descLabel->setText(QString());
    updateOkButton();
}

void NewDialog::okButtonClicked()
{
    if (m_ui->templatesTree->currentItem())
        accept();
}

void NewDialog::updateOkButton()
{
    m_okButton->setEnabled(currentWizard() != 0);
}
