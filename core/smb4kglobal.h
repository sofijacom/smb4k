/***************************************************************************
    smb4kglobal  -  This is the global namespace for Smb4K.
                             -------------------
    begin                : Sa Apr 2 2005
    copyright            : (C) 2005-2009 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KGLOBAL_H
#define SMB4KGLOBAL_H

// Qt includes
#include <QString>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;

/**
 * This is the global namespace for the core classes of Smb4K.
 *
 * @author    Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

namespace Smb4KGlobal
{
  /**
   * This function returns the interval of the timer.
   *
   * @returns           The interval of the timer in msec.
   */
  KDE_EXPORT int timerInterval();

  /**
   * This function returns the global list of workgroups that were discovered by
   * Smb4K. Use this if you want to access and modify the list with your code.
   *
   * @returns the global list of known workgroups.
   */
  KDE_EXPORT QList<Smb4KWorkgroup *> *workgroupsList();

  /**
   * This function returns the workgroup or domain that matches the name @p name or
   * NULL if there is no such workgroup.
   *
   * @returns a pointer to the workgroup with name @p name.
   */
  KDE_EXPORT Smb4KWorkgroup *findWorkgroup( const QString &name );

  /**
   * This function takes a workgroup @p workgroup, checks whether it is already
   * in the list of domains and adds it to the list if necessary. It returns TRUE
   * if the workgroup was added and FALSE otherwise.
   *
   * Please prefer this function over per class solutions.
   *
   * @param workgroup   The workgroup item
   *
   * @returns TRUE if the workgroup was added and FALSE otherwise.
   */
  KDE_EXPORT bool addWorkgroup( Smb4KWorkgroup *workgroup );

  /**
   * This function removes a workgroup @p workgroup from the list of domains. The
   * pointer that is passed to this function will be deleted. You won't be able
   * to use it afterwards. This function returns TRUE if the workgroup was removed
   * and FALSE otherwise.
   *
   * Please prefer this function over per class solutions.
   *
   * @param workgroup   The workgroup item
   *
   * @returns TRUE if the workgroup was removed and FALSE otherwise.
   */
  KDE_EXPORT bool removeWorkgroup( Smb4KWorkgroup *workgroup );

  /**
   * This function clears the global list of workgroups.
   */
  KDE_EXPORT void clearWorkgroupsList();

  /**
   * This function returns the global list of hosts that were discovered by
   * Smb4K. Use this if you want to access and modify the list with your code.
   *
   * @returns the global list of known hosts.
   */
  KDE_EXPORT QList<Smb4KHost *> *hostsList();

  /**
   * This function returns the host matching the name @p name or NULL if there is no
   * such host. The name of the host is mandatory. The workgroup may be empty, but
   * should be given, because this will speed up the search process.
   *
   * @param name          The name of the host
   *
   * @param workgroup     The workgroup where the host is located
   *
   * @returns an Smb4KHost item of NULL if none was found that matches @p name.
   */
  KDE_EXPORT Smb4KHost *findHost( const QString &name,
                                  const QString &workgroup = QString() );

  /**
   * This function takes a host @p host, checks whether it is already
   * in the list of hosts and adds it to the list if necessary. It returns TRUE
   * if the host was added and FALSE otherwise.
   *
   * Please prefer this function over per class solutions.
   *
   * @param host          The host item
   *
   * @returns TRUE if the host was added and FALSE otherwise.
   */
  KDE_EXPORT bool addHost( Smb4KHost *host );

  /**
   * This function removes a host @p host from the list of hosts. The
   * pointer that is passed to this function will be deleted. You won't
   * be able to use it afterwards. This function returns TRUE if the host was removed
   * and FALSE otherwise.
   *
   * Please prefer this function over per class solutions.
   *
   * @param host          The host item
   *
   * @returns TRUE if the host was removed and FALSE otherwise.
   */
  KDE_EXPORT bool removeHost( Smb4KHost *host );

  /**
   * This function clears the global list of hosts.
   */
  KDE_EXPORT void clearHostsList();

  /**
   * This function returns all hosts that belong to the workgroup or domain
   * @p workgroup.
   *
   * Please favor this function over per class solutions.
   *
   * @param workgroup   The workgroup for that the list should be returned.
   *
   * @returns the list of hosts belonging to the workgroup or domain @param workgroup.
   */
  KDE_EXPORT QList<Smb4KHost *> workgroupMembers( Smb4KWorkgroup *workgroup );

  /**
   * This function returns the list of shares that were discovered by Smb4K.
   * Use this if you want to access and modify the list with your code.
   *
   * @returns the global list of known shares.
   */
  KDE_EXPORT QList<Smb4KShare *> *sharesList();

  /**
   * This function returns the share located at host @p host matching the name
   * @p name or NULL if there is no such share. The name of the share and host
   * are mandatory. The workgroup entry may be empty.
   *
   * @param name          The name of the share
   *
   * @param host          The name of the host
   *
   * @param workgroup     The name of the workgroup
   *
   * @returns an Smb4KShare item of NULL if none was found that matches @p name
   * and @p host.
   */
  KDE_EXPORT Smb4KShare *findShare( const QString &name,
                                    const QString &host,
                                    const QString &workgroup = QString() );

  /**
   * This function takes a share @p share, checks whether it is already
   * in the list of shares and adds it to the list if necessary. It returns TRUE
   * if the share was added and FALSE otherwise.
   *
   * Please prefer this function over per class solutions.
   *
   * @param share         The share item
   *
   * @returns TRUE if the share was added and FALSE otherwise.
   */
  KDE_EXPORT bool addShare( Smb4KShare *share );

  /**
   * This function removes a share @p share from the list of shares. The
   * pointer that is passed to this function will be deleted. You won't
   * be able to use it afterwards. This function returns TRUE if the share was removed
   * and FALSE otherwise.
   *
   * Please prefer this function over per class solutions.
   *
   * @param share         The share item
   *
   * @returns TRUE if the share was removed and FALSE otherwise.
   */
  KDE_EXPORT bool removeShare( Smb4KShare *share );

  /**
   * This function clears the global list of shares.
   */
  KDE_EXPORT void clearSharesList();

  /**
   * This function returns the list of shares that is provided by one specific host
   * @p host.
   *
   * Please favor this function over per class solutions.
   *
   * @param host          The host where the shares are located
   *
   * @returns the list of shares that are provided by the host @p host.
   */
  KDE_EXPORT QList<Smb4KShare *> sharedResources( Smb4KHost *host );

  /**
   * This function returns the global list of mounted shares that were discovered by
   * Smb4K. Use this if you want to access and modify the list with your code.
   *
   * @returns the global list of known mounted shares.
   */
  KDE_EXPORT QList<Smb4KShare *> *mountedSharesList();

  /**
   * Find a mounted share by its path (i.e. mount point).
   *
   * @returns the share that is mounted to @p path.
   */
  KDE_EXPORT Smb4KShare *findShareByPath( const QByteArray &path );

  /**
   * Find all mounts of a particular share with UNC @p unc on the system.
   *
   * This function will compare the incoming string with the UNC of each
   * mounted share to find all shares with the same UNC. For the comparison
   * the scheme/protocol, the user info, and the port will be stripped. You
   * can pass the UNC in any valid format. The function will internally
   * convert the string into a QUrl and work with that.
   *
   * @param unc         The UNC of the share
   *
   * @returns the complete list of mounts with the UNC @p unc.
   */
  KDE_EXPORT QList<Smb4KShare *> findShareByUNC( const QString &unc );

  /**
   * This function returns the list of inaccessible shares.
   *
   * @returns the list of inaccessible shares.
   */
  KDE_EXPORT QList<Smb4KShare *> findInaccessibleShares();

  /**
   * This function takes a mounted share @p share, checks whether it is
   * already in the list of mounted shares and adds it to the list if
   * necessary. It returns TRUE if the share was added and FALSE otherwise.
   *
   * Please prefer this function over per class solutions.
   *
   * @param share       The share item
   *
   * @returns TRUE if the share was added and FALSE otherwise.
   */
  KDE_EXPORT bool addMountedShare( Smb4KShare *share );

  /**
   * This function removes a mounted share @p share from the list of mounted
   * shares. The pointer that is passed to this function will be deleted.
   * You won't be able to use it afterwards. This function returns TRUE if the share was removed
   * and FALSE otherwise.
   *
   * Please prefer this function over per class solutions.
   *
   * @param share       The share item
   *
   * @returns TRUE if the share was removed and FALSE otherwise.
   */
  KDE_EXPORT bool removeMountedShare( Smb4KShare *share );
};

#endif
