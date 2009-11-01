/***************************************************************************
    smb4kipaddressscanner  -  This class scans for IP addresses. It
    belongs to the core classes of Smb4K.
                             -------------------
    begin                : Di Apr 22 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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

// Qt includes
#include <QCoreApplication>
#include <QTimer>

// KDE includes
#include <kglobal.h>
#include <kdebug.h>
#include <kshell.h>
#include <kstandarddirs.h>

// application specific includes
#include <smb4kipaddressscanner.h>
#include <smb4kglobal.h>
#include <smb4ksambaoptionshandler.h>
#include <smb4khost.h>
#include <smb4kcoremessage.h>
#include <smb4kipaddressscanner_p.h>
#include <smb4ksettings.h>
#include <smb4kprocess.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KIPAddressScannerPrivate, priv );


Smb4KIPAddressScanner::Smb4KIPAddressScanner() : QObject()
{
  startTimer( 50 );
  
  connect( QCoreApplication::instance(), SIGNAL( aboutToQuit() ),
           this,                         SLOT( slotAboutToQuit() ) );
}


Smb4KIPAddressScanner::~Smb4KIPAddressScanner()
{
}


Smb4KIPAddressScanner *Smb4KIPAddressScanner::self()
{
  return &priv->instance;
}


void Smb4KIPAddressScanner::lookup( Smb4KHost *host, bool wait )
{
  Q_ASSERT( host );

  // Return if the IP address has already been checked.
  if ( host->ipChecked() )
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  // Find the host in the global list and check if it already has an
  // IP address. Copy it if available or initialize a lookup.
  Smb4KHost *known_host = findHost( host->hostName(), host->workgroupName() );

  if ( known_host && known_host->ipChecked() )
  {
    host->setIP( known_host->ip() );
    emit ipAddress( host );
  }
  else
  {
    // Find nmblookup program.
    QString nmblookup = KStandardDirs::findExe( "nmblookup" );

    if ( nmblookup.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "nmblookup" );
      return;
    }
    else
    {
      // Go ahead
    }

    // Find grep program.
    QString grep = KStandardDirs::findExe( "grep" );

    if ( grep.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "grep" );
      return;
    }
    else
    {
      // Go ahead
    }

    // Find the awk program
    QString awk = KStandardDirs::findExe( "awk" );

    if ( awk.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "awk" );
      return;
    }
    else
    {
      // Go ahead
    }

    QString command;
    command += nmblookup;
    command += " ";
    command += Smb4KSambaOptionsHandler::self()->nmblookupOptions();

    if ( !Smb4KSambaOptionsHandler::self()->winsServer().isEmpty() )
    {
      command += " -R -U "+KShell::quoteArg( Smb4KSambaOptionsHandler::self()->winsServer() );
    }
    else
    {
      // Do nothing
    }

    command += " -- ";
    command += KShell::quoteArg( host->hostName() );
    command += " | ";
    command += grep+" '<00>' | ";
    command += awk+" {'print $1'}";

    IPScanThread *thread = new IPScanThread( host, this );
    m_cache.insert( host->hostName(), thread );

    connect( thread, SIGNAL( finished() ), this, SLOT( slotThreadFinished() ) );
    connect( thread, SIGNAL( ipAddress( Smb4KHost * ) ), this, SIGNAL( ipAddress( Smb4KHost * ) ) );
    connect( thread, SIGNAL( ipAddress( Smb4KHost * ) ), this, SLOT( slotProcessIPAddress( Smb4KHost * ) ) );

    thread->start();
    thread->lookup( command );

    if ( wait )
    {
      thread->wait();
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KIPAddressScanner::lookup( const QList<Smb4KHost *> &list, bool wait )
{
  for ( int i = 0; i < list.size(); ++i )
  {
    lookup( list.at( i ), wait );
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KIPAddressScanner::slotAboutToQuit()
{
  // Stop all processes.
  QStringList keys = m_cache.keys();

  foreach ( const QString &key, keys )
  {
    IPScanThread *thread = m_cache.object( key );

    if ( thread->process() && 
         (thread->process()->state() == KProcess::Running || 
         thread->process()->state() == KProcess::Starting) )
    {
      thread->process()->abort();
    }
    else
    {
      // Do not touch the thread
    }
  }
}


void Smb4KIPAddressScanner::slotProcessIPAddress( Smb4KHost *host )
{
  // Find the host in the global list and update it as well,
  // if appropriate.
  Smb4KHost *known_host = findHost( host->hostName(), host->workgroupName() );
  
  if ( known_host )
  {
    if ( host->hasIP() && 
         QString::compare( host->ip(), known_host->ip() ) != 0 )
    {
      known_host->setIP( host->ip() );
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }
  
  // Signal is forwarded from the ip scan thread. We do not need 
  // to emit from it here.
}


void Smb4KIPAddressScanner::slotThreadFinished()
{
  QStringList keys = m_cache.keys();

  foreach ( const QString &key, keys )
  {
    IPScanThread *thread = m_cache.object( key );

    if ( thread->isFinished() )
    {
      m_cache.remove( key );
    }
    else
    {
      // Do not touch the thread
    }
  }
}


#include "smb4kipaddressscanner.moc"
