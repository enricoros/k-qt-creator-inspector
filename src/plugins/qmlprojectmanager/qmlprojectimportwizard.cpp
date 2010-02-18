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

#include "qmlprojectimportwizard.h"

#include "qmlprojectconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <utils/filenamevalidatinglineedit.h>
#include <utils/filewizardpage.h>

#include <QtCore/QDir>
#include <QtCore/QtDebug>
#include <QtCore/QCoreApplication>


using namespace Utils;

namespace QmlProjectManager {
namespace Internal {

//////////////////////////////////////////////////////////////////////////////
// QmlProjectImportWizardDialog
//////////////////////////////////////////////////////////////////////////////

QmlProjectImportWizardDialog::QmlProjectImportWizardDialog(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Import of QML Project"));

    // first page
    m_firstPage = new FileWizardPage;
    m_firstPage->setTitle(tr("QML Project"));
    m_firstPage->setFileNameLabel(tr("Project name:"));
    m_firstPage->setPathLabel(tr("Location:"));

    addPage(m_firstPage);
}

QmlProjectImportWizardDialog::~QmlProjectImportWizardDialog()
{ }

QString QmlProjectImportWizardDialog::path() const
{
    return m_firstPage->path();
}

void QmlProjectImportWizardDialog::setPath(const QString &path)
{
    m_firstPage->setPath(path);
}

QString QmlProjectImportWizardDialog::projectName() const
{
    return m_firstPage->fileName();
}

//////////////////////////////////////////////////////////////////////////////
// QmlProjectImportWizard
//////////////////////////////////////////////////////////////////////////////

QmlProjectImportWizard::QmlProjectImportWizard()
    : Core::BaseFileWizard(parameters())
{ }

QmlProjectImportWizard::~QmlProjectImportWizard()
{ }

Core::BaseFileWizardParameters QmlProjectImportWizard::parameters()
{
    Core::BaseFileWizardParameters parameters(ProjectWizard);
    parameters.setIcon(QIcon(QLatin1String(":/wizards/images/console.png")));
    parameters.setDisplayName(tr("Import of existing QML directory"));
    parameters.setId(QLatin1String("QI.QML Import"));
    parameters.setDescription(tr("Creates a QML project from an existing directory of QML files."));
    parameters.setCategory(QLatin1String(Constants::QML_WIZARD_CATEGORY));
    parameters.setDisplayCategory(QCoreApplication::translate(Constants::QML_WIZARD_TR_SCOPE,
                                                              Constants::QML_WIZARD_TR_CATEGORY));
    return parameters;
}

QWizard *QmlProjectImportWizard::createWizardDialog(QWidget *parent,
                                                  const QString &defaultPath,
                                                  const WizardPageList &extensionPages) const
{
    QmlProjectImportWizardDialog *wizard = new QmlProjectImportWizardDialog(parent);
    setupWizard(wizard);

    wizard->setPath(defaultPath);

    foreach (QWizardPage *p, extensionPages)
        wizard->addPage(p);

    return wizard;
}

Core::GeneratedFiles QmlProjectImportWizard::generateFiles(const QWizard *w,
                                                     QString *errorMessage) const
{
    Q_UNUSED(errorMessage)

    const QmlProjectImportWizardDialog *wizard = qobject_cast<const QmlProjectImportWizardDialog *>(w);
    const QString projectPath = wizard->path();
    const QDir dir(projectPath);
    const QString projectName = wizard->projectName();
    const QString creatorFileName = QFileInfo(dir, projectName + QLatin1String(".qmlproject")).absoluteFilePath();

    QString projectContents;
    {
        QTextStream out(&projectContents);

        out
            //: Comment added to generated .qmlproject file
            << "// " << tr("File generated by QtCreator", "qmlproject Template") << endl
            << endl
            << "import QmlProject 1.0" << endl
            << endl
            << "Project {" << endl
            << "    // " << tr("Scan current directory and subdirectories for .qml, .js, and image files", "qmlproject Template") << endl
            << "    QmlFiles {" << endl
            << "        directory: \".\"" << endl
            << "        recursive: true" << endl
            << "    }" << endl
            << "    JavaScriptFiles {" << endl
            << "        directory: \".\"" << endl
            << "        recursive: true" << endl
            << "    }" << endl
            << "    ImageFiles {" << endl
            << "        directory: \".\"" << endl
            << "        recursive: true" << endl
            << "    }" << endl
            << "}" << endl;
    }
    Core::GeneratedFile generatedCreatorFile(creatorFileName);
    generatedCreatorFile.setContents(projectContents);

    Core::GeneratedFiles files;
    files.append(generatedCreatorFile);

    return files;
}

bool QmlProjectImportWizard::postGenerateFiles(const Core::GeneratedFiles &l, QString *errorMessage)
{
    // Post-Generate: Open the project
    const QString proFileName = l.back().path();
    if (!ProjectExplorer::ProjectExplorerPlugin::instance()->openProject(proFileName)) {
        *errorMessage = tr("The project %1 could not be opened.").arg(proFileName);
        return false;
    }
    return true;
}

} // namespace Internal
} // namespace QmlProjectManager