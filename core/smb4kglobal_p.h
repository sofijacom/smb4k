/***************************************************************************
    smb4kglobal_p  -  These are the private helper classes of the 
    Smb4KGlobal namespace.
                             -------------------
    begin                : Di Jul 24 2007
    copyright            : (C) 2007-2011 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KGLOBAL_P_H
#define SMB4KGLOBAL_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QList>
#include <QMap>
#include <QObject>

// application specific includes
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4kshare.h>

/**
 * This class is a private helper for the Smb4KGlobal namespace.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KGlobalPrivate : public QObject
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    Smb4KGlobalPrivate();

    /**
     * Destructor
     */
    ~Smb4KGlobalPrivate();

    /**
     * This is the global workgroup list.
     */
    QList<Smb4KWorkgroup *> workgroupsList;

    /**
     * This is the global host list.
     */
    QList<Smb4KHost *> hostsList;

    /**
     * This is global list of mounted shares.
     */
    QList<Smb4KShare *> mountedSharesList;

    /**
     * This is the global list of shares.
     */
    QList<Smb4KShare *> sharesList;
    
    /**
     * The global options defined in smb.conf
     */
    const QMap<QString,QString> &globalSambaOptions( bool read );

    /**
     * Boolean that is TRUE when only foreign shares
     * are in the list of mounted shares
     */
    bool onlyForeignShares;

    /**
     * Set default values for some settings
     */
    void setDefaultSettings();

    /**
     * Make connections
     */
    void makeConnections();

  public slots:
    /**
     * This slot does last things before the application quits
     */
    void slotAboutToQuit();

  private:
    QMap<QString,QString> m_samba_options;
};

#endif
