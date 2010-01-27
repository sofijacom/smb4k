/***************************************************************************
    smb4kmounter_p  -  This file contains private helper classes for the
    Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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

#ifndef SMB4KMOUNTER_P_H
#define SMB4KMOUNTER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QThread>
#include <QString>

// KDE includes
#include <kdebug.h>

// system includes
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __FreeBSD__
#include <sys/statfs.h>
#include <unistd.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
#endif
#include <errno.h>

// application specific includes
#include <smb4kmounter.h>
#include <smb4kprocess.h>
#include <smb4kshare.h>

class Smb4KAuthInfo;
class Smb4KShare;

class BasicMountThread : public QThread
{
  Q_OBJECT

  friend class MountThread;
  friend class UnmountThread;

  public:
    enum Type { MountThread,
                UnmountThread };

    BasicMountThread( Type type,
                      Smb4KShare *share,
                      QObject *parent = 0 );
    ~BasicMountThread();
    Type type() { return m_type; }
    Smb4KProcess *process() { return m_proc; }
    Smb4KShare *share() { return &m_share; }

  private:
    Type m_type;
    Smb4KShare m_share;
    Smb4KProcess *m_proc;
};


class MountThread : public BasicMountThread
{
  Q_OBJECT

  public:
    MountThread( Smb4KShare *share,
                 QObject *parent = 0 );
    ~MountThread();
    void mount( Smb4KAuthInfo *authInfo,
                const QString &command );
                
  signals:
    void mounted( Smb4KShare *share );
    void authError( Smb4KShare *share );
    void badShareName( Smb4KShare *share );

  protected slots:
    void slotProcessError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
};


class UnmountThread : public BasicMountThread
{
  Q_OBJECT
  
  public:
    UnmountThread( Smb4KShare *share,
                   QObject *parent = 0 );
    ~UnmountThread();
    void unmount( const QString &command );
    
  signals:
    void unmounted( Smb4KShare *share );
    
  protected slots:
    void slotProcessError();
    void slotProcessFinished( int exitCode,
                              QProcess::ExitStatus exitStatus );
};


class CheckThread : public QThread
{
  Q_OBJECT

  public:
    CheckThread( Smb4KShare *share,
                 QObject *parent = 0 );
    ~CheckThread();
    void check();

  private:
    Smb4KShare *m_share;
};


class Smb4KMounterPrivate
{
  public:
    Smb4KMounterPrivate();
    ~Smb4KMounterPrivate();
    Smb4KMounter instance;
    void setAboutToQuit();
    bool aboutToQuit() { return m_quit; }
    void setHardwareReason( bool hardware );
    bool hardwareReason() { return m_hardware; }

  private:
    bool m_quit;
    bool m_hardware;
};

#endif
