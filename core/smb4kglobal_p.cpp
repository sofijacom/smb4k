/*
    These are the private helper classes of the Smb4KGlobal namespace.
    -------------------
    begin                : Di Jul 24 2007
    SPDX-FileCopyrightText: 2007-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kglobal_p.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"

// Samba includes
#include <libsmbclient.h>

// Qt includes
#include <QAbstractSocket>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QHostAddress>
#include <QHostInfo>
#include <QTextCodec>
#include <QTextStream>

Smb4KGlobalPrivate::Smb4KGlobalPrivate()
{
    onlyForeignShares = false;
    coreInitialized = false;

#ifdef Q_OS_LINUX
    allowedMountArguments << "dynperm";
    allowedMountArguments << "rwpidforward";
    allowedMountArguments << "hard";
    allowedMountArguments << "soft";
    allowedMountArguments << "noacl";
    allowedMountArguments << "cifsacl";
    allowedMountArguments << "backupuid";
    allowedMountArguments << "backupgid";
    allowedMountArguments << "ignorecase";
    allowedMountArguments << "nocase";
    allowedMountArguments << "nobrl";
    allowedMountArguments << "sfu";
    allowedMountArguments << "nounix";
    allowedMountArguments << "nouser_xattr";
    allowedMountArguments << "fsc";
    allowedMountArguments << "multiuser";
    allowedMountArguments << "actimeo";
    allowedMountArguments << "noposixpaths";
    allowedMountArguments << "posixpaths";
#endif

    //
    // Create and init the SMB context
    //
    SMBCCTX *smbContext = smbc_new_context();

    if (smbContext) {
        smbContext = smbc_init_context(smbContext);

        if (!smbContext) {
            smbc_free_context(smbContext, 1);
        }
    }

    //
    // Read the computer's NetBIOS name and workgroup
    //
    machineNetbiosName = QString::fromUtf8(smbc_getNetbiosName(smbContext)).toUpper();
    machineWorkgroupName = QString::fromUtf8(smbc_getWorkgroup(smbContext)).toUpper();

    //
    // Free the SMB context
    //
    smbc_free_context(smbContext, 1);

    //
    // Connections
    //
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
}

Smb4KGlobalPrivate::~Smb4KGlobalPrivate()
{
    //
    // Clear the workgroup list
    //
    while (!workgroupsList.isEmpty()) {
        workgroupsList.takeFirst().clear();
    }

    //
    // Clear the host list
    //
    while (!hostsList.isEmpty()) {
        hostsList.takeFirst().clear();
    }

    //
    // Clear the list of mounted shares
    //
    while (!mountedSharesList.isEmpty()) {
        mountedSharesList.takeFirst().clear();
    }

    //
    // Clear the list of shares
    //
    while (!sharesList.isEmpty()) {
        sharesList.takeFirst().clear();
    }
}

void Smb4KGlobalPrivate::slotAboutToQuit()
{
    Smb4KSettings::self()->save();
}
