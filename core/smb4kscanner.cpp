/***************************************************************************
    smb4kscanner.cpp  -  The network scan core class of Smb4K.
                             -------------------
    begin                : Sam Mai 31 2003
    copyright            : (C) 2003-2009 by Alexander Reinholdt
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
#include <QApplication>
#include <QMutableListIterator>

// KDE includes
#include <kapplication.h>
#include <kdebug.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

// Application specific includes.
#include <smb4kscanner.h>
#include <smb4kscanner_p.h>
#include <smb4kauthinfo.h>
#include <smb4kcoremessage.h>
#include <smb4kglobal.h>
#include <smb4ksettings.h>
#include <smb4kworkgroup.h>
#include <smb4khost.h>
#include <smb4kshare.h>
#include <smb4kipaddressscanner.h>
#include <smb4khomesshareshandler.h>
#include <smb4kdefs.h>
#include <smb4kwalletmanager.h>
#include <smb4kprocess.h>
#include <smb4ksambaoptionshandler.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KScannerPrivate, priv );



Smb4KScanner::Smb4KScanner() : QObject()
{
  m_working = false;
  m_state = SCANNER_STOP;
}


Smb4KScanner::~Smb4KScanner()
{
}


Smb4KScanner *Smb4KScanner::self()
{
  return &priv->instance;
}


void Smb4KScanner::init()
{
  // Scan the network for domains.
  lookupDomains();
}


void Smb4KScanner::abort( Smb4KBasicNetworkItem *item, int process )
{
  if ( item )
  {
    BasicScanThread *thread = m_cache.object( item->key() );

    if ( thread && thread->process() &&
         (thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting) )
    {
      thread->process()->abort();
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    QStringList keys = m_cache.keys();

    foreach ( QString key, keys )
    {
      BasicScanThread *thread = m_cache.object( key );

      if ( thread->process() )
      {
        switch ( thread->process()->type() )
        {
          case Smb4KProcess::LookupDomains:
          {
            if ( process == LookupDomains )
            {
              thread->process()->abort();
            }
            else
            {
              // Do nothing
            }

            break;
          }
          case Smb4KProcess::LookupDomainMembers:
          {
            if ( process == LookupDomainMembers )
            {
              thread->process()->abort();
            }
            else
            {
              // Do nothing
            }

            break;
          }
          case Smb4KProcess::LookupShares:
          {
            if ( process == LookupShares )
            {
              thread->process()->abort();
            }
            else
            {
              // Do nothing
            }
          }
          case Smb4KProcess::LookupInfo:
          {
            if ( process == LookupInfo )
            {
              thread->process()->abort();
            }
            else
            {
              // Do nothing
            }
          }
          default:
          {
            break;
          }
        }
      }
      else
      {
        continue;
      }
    }
  }
}


bool Smb4KScanner::isAborted( Smb4KBasicNetworkItem *item, int process )
{
  bool aborted = false;

  if ( item )
  {
    BasicScanThread *thread = m_cache.object( item->key() );
    aborted = (thread && thread->process() && thread->process()->isAborted());
  }
  else
  {
    QStringList keys = m_cache.keys();

    foreach ( QString key, keys )
    {
      if ( !aborted )
      {
        BasicScanThread *thread = m_cache.object( key );

        if ( thread->process() )
        {
          switch ( thread->process()->type() )
          {
            case Smb4KProcess::LookupDomains:
            {
              if ( process == LookupDomains )
              {
                aborted = thread->process()->isAborted();
              }
              else
              {
                // Do nothing
              }

              break;
            }
            case Smb4KProcess::LookupDomainMembers:
            {
              if ( process == LookupDomainMembers )
              {
                aborted = thread->process()->isAborted();
              }
              else
              {
                // Do nothing
              }

              break;
            }
            case Smb4KProcess::LookupShares:
            {
              if ( process == LookupShares )
              {
                aborted = thread->process()->isAborted();
              }
              else
              {
                // Do nothing
              }
            }
            case Smb4KProcess::LookupInfo:
            {
              if ( process == LookupInfo )
              {
                aborted = thread->process()->isAborted();
              }
              else
              {
                // Do nothing
              }
            }
            default:
            {
              break;
            }
          }
        }
        else
        {
          continue;
        }
      }
      else
      {
        break;
      }
    }
  }

  return aborted;
}


void Smb4KScanner::abortAll()
{
  if ( !kapp->closingDown() )
  {
    QStringList keys = m_cache.keys();

    foreach ( QString key, keys )
    {
      BasicScanThread *thread = m_cache.object( key );

      if ( thread->process() )
      {
        if ( thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting )
        {
          thread->process()->abort();
          continue;
        }
        else
        {
          continue;
        }
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    // priv has already been deleted
  }
}


bool Smb4KScanner::isRunning( Smb4KBasicNetworkItem *item, int process )
{
  bool running = false;

  if ( item )
  {
    BasicScanThread *thread = m_cache.object( item->key() );
    running = (thread && thread->process() && thread->process()->state() == KProcess::Running);
  }
  else
  {
    QStringList keys = m_cache.keys();

    foreach ( QString key, keys )
    {
      if ( !running )
      {
        BasicScanThread *thread = m_cache.object( key );

        if ( thread->process() )
        {
          switch ( thread->process()->type() )
          {
            case Smb4KProcess::LookupDomains:
            {
              if ( process == LookupDomains )
              {
                running = (thread->process()->state() == KProcess::Running);
              }
              else
              {
                // Do nothing
              }

              break;
            }
            case Smb4KProcess::LookupDomainMembers:
            {
              if ( process == LookupDomainMembers )
              {
                running = (thread->process()->state() == KProcess::Running);
              }
              else
              {
                // Do nothing
              }

              break;
            }
            case Smb4KProcess::LookupShares:
            {
              if ( process == LookupShares )
              {
                running = (thread->process()->state() == KProcess::Running);
              }
              else
              {
                // Do nothing
              }
            }
            case Smb4KProcess::LookupInfo:
            {
              if ( process == LookupInfo )
              {
                running = (thread->process()->state() == KProcess::Running);
              }
              else
              {
                // Do nothing
              }
            }
            default:
            {
              break;
            }
          }
        }
        else
        {
          continue;
        }
      }
      else
      {
        break;
      }
    }
  }

  return running;
}


void Smb4KScanner::lookupDomains()
{
  // The mode
  LookupDomainsThread::Mode mode = LookupDomainsThread::Unknown;

  // Compile the command.
  QString command;

  if ( Smb4KSettings::lookupDomains() )
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

    // Find awk program
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

    // Find xargs program
    QString xargs = KStandardDirs::findExe( "xargs" );

    if ( xargs.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "xargs" );
      return;
    }
    else
    {
      // Go ahead
    }

    command += nmblookup;
    command += " -M";
    command += " "+Smb4KSambaOptionsHandler::self()->nmblookupOptions();
    command += " --";
    command += " - | ";
    command += grep+" '<01>' | ";
    command += awk+" '{print $1}' | ";

    if ( !Smb4KSambaOptionsHandler::self()->winsServer().isEmpty() )
    {
      command += xargs+" -Iips ";
      command += nmblookup;
      command += " -R";
      command += " -U "+Smb4KSambaOptionsHandler::self()->winsServer();
      command += " -A ips";
    }
    else
    {
      command += xargs+" -Iips ";
      command += nmblookup;
      command += " -A ips";
    }

    command += " "+Smb4KSambaOptionsHandler::self()->nmblookupOptions();

    m_state = SCANNER_LOOKUP_DOMAINS;
    mode = LookupDomainsThread::LookupDomains;
  }
  else if ( Smb4KSettings::queryCurrentMaster() )
  {
    // Find net program
    QString net = KStandardDirs::findExe( "net" );

    if ( net.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "net" );
      return;
    }
    else
    {
      // Go ahead
    }

    // Find xargs program
    QString xargs = KStandardDirs::findExe( "xargs" );

    if ( xargs.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "xargs" );
      return;
    }
    else
    {
      // Go ahead
    }

    Smb4KWorkgroup workgroup;

    if ( !Smb4KSettings::domainName().isEmpty() )
    {
      workgroup.setWorkgroupName( Smb4KSettings::domainName() );
    }
    else
    {
      workgroup.setWorkgroupName( Smb4KSambaOptionsHandler::self()->globalSambaOptions()["workgroup"] );
    }

    command += net;
    command += " "+Smb4KSambaOptionsHandler::self()->netOptions( Smb4KSambaOptionsHandler::LookupMaster, &workgroup );
    command += " -U % | ";
    command += xargs+" -Imaster ";
    command += net;
    command += " "+Smb4KSambaOptionsHandler::self()->netOptions( Smb4KSambaOptionsHandler::Domain );
    command += " -U %";
    command += " -S master";

    m_state = SCANNER_QUERY_MASTER_BROWSER;
    mode = LookupDomainsThread::QueryMaster;
  }
  else if ( Smb4KSettings::queryCustomMaster() )
  {
    // Find net program
    QString net = KStandardDirs::findExe( "net" );

    if ( net.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "net" );
      return;
    }
    else
    {
      // Go ahead
    }

    // Find xargs program
    QString xargs = KStandardDirs::findExe( "xargs" );

    if ( xargs.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "xargs" );
      return;
    }
    else
    {
      // Go ahead
    }

    Smb4KHost host( Smb4KSettings::customMasterBrowser() );

    command += net;
    command += " "+Smb4KSambaOptionsHandler::self()->netOptions( Smb4KSambaOptionsHandler::LookupHost, &host );
    command += " -U %";
    command += " -S "+KShell::quoteArg( host.hostName() );
    command += " | ";
    command += xargs+" -Iip ";
    command += net;
    command += " "+Smb4KSambaOptionsHandler::self()->netOptions( Smb4KSambaOptionsHandler::Domain );
    command += " -U %";
    command += " -S "+KShell::quoteArg( host.hostName() );
    command += " -I ip";

    m_state = SCANNER_QUERY_MASTER_BROWSER;
    mode = LookupDomainsThread::QueryMaster;
  }
  else if ( Smb4KSettings::scanBroadcastAreas() )
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

    // Find awk program
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

    // Find sed program
    QString sed = KStandardDirs::findExe( "sed" );

    if ( sed.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sed" );
      return;
    }
    else
    {
      // Go ahead
    }

    // Find xargs program
    QString xargs = KStandardDirs::findExe( "xargs" );

    if ( xargs.isEmpty() )
    {
      Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "xargs" );
      return;
    }
    else
    {
      // Go ahead
    }

    QStringList addresses = Smb4KSettings::broadcastAreas().split( ",", QString::SkipEmptyParts );

    for ( int i = 0; i < addresses.size(); ++i )
    {
      command += nmblookup;
      // We want all globally defined options for nmblookup, except
      // the broadcast address, because that is needed for the IP
      // scan:
      command += " "+Smb4KSambaOptionsHandler::self()->nmblookupOptions( false );
      command += " -B "+addresses.at( i );
      command += " --";
      command += " '*' | ";
      command += sed+" -e /querying/d | ";
      command += awk+" '{print $1}' | ";
      command += xargs+" -Iip ";
      command += nmblookup;
      // This time we want to have the globally defined broadcast
      // address:
      command += " "+Smb4KSambaOptionsHandler::self()->nmblookupOptions();
      // Include the WINS server:
      if ( !Smb4KSambaOptionsHandler::self()->winsServer().isEmpty() )
      {
        command += " -R -U "+Smb4KSambaOptionsHandler::self()->winsServer();
      }
      else
      {
        // Do nothing
      }
      command += " -A ip";
      command += " ; ";
    }

    // Get rid of the last 3 characters (" ; "):
    command.truncate( command.length() - 3 );

    m_state = SCANNER_SCAN_BROADCAST_AREAS;
    mode = LookupDomainsThread::ScanBroadcastAreas;
  }
  else
  {
    // This should never happen. Return.
    return;
  }

  // Start looking for the workgroups.
  if ( m_cache.size() == 0 )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    m_working = true;
    // State was set above.
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  emit aboutToStart( NULL, LookupDomains );

  LookupDomainsThread *thread = new LookupDomainsThread( mode, this );
  m_cache.insert( QString( "%1" ).arg( rand() ), thread );

  connect( thread, SIGNAL( finished() ),
           this,   SLOT( slotThreadFinished() ) );
  connect( thread, SIGNAL( workgroups( QList<Smb4KWorkgroup> & ) ),
           this,   SLOT( slotWorkgroups( QList<Smb4KWorkgroup> & ) ) );
  connect( thread, SIGNAL( hosts( Smb4KWorkgroup *, QList<Smb4KHost> & ) ),
           this,   SLOT( slotHosts( Smb4KWorkgroup *, QList<Smb4KHost> & ) ) );

  thread->start();
  thread->lookup( command );
}


void Smb4KScanner::lookupDomainMembers( Smb4KWorkgroup *workgroup )
{
  Q_ASSERT( workgroup );

  // Find net program
  QString net = KStandardDirs::findExe( "net" );

  if ( net.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "net" );
    return;
  }
  else
  {
    // Go ahead
  }

  // Get the authentication information.
  Smb4KHost *master = findHost( workgroup->masterBrowserName(), workgroup->workgroupName() );

  if ( master )
  {
    Smb4KAuthInfo authInfo( master );

    if ( Smb4KSettings::masterBrowsersRequireAuth() )
    {
      Smb4KWalletManager::self()->readAuthInfo( &authInfo );
    }
    else
    {
      // Do nothing
    }

    // Compile the command.
    QString command;
    command += net;
    command += " "+Smb4KSambaOptionsHandler::self()->netOptions( Smb4KSambaOptionsHandler::ServerDomain );

    if ( workgroup->hasMasterBrowserIP() )
    {
      command += " -I "+workgroup->masterBrowserIP();
    }
    else
    {
      // Do nothing
    }

    command += " -w "+KShell::quoteArg( workgroup->workgroupName() );
    command += " -S "+KShell::quoteArg( workgroup->masterBrowserName() );

    if ( Smb4KSettings::masterBrowsersRequireAuth() )
    {
      if ( !authInfo.login().isEmpty() )
      {
        command += " -U "+KShell::quoteArg( authInfo.login() );
        // Password will be set below.
      }
      else
      {
        command += " -U %";
      }
    }
    else
    {
      command += " -U %";
    }

    m_state = SCANNER_OPEN_WORKGROUP;

    // Start looking for the workgroup members.
    if ( m_cache.size() == 0 )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      m_working = true;
      emit stateChanged();
    }
    else
    {
      // Already running
    }

    emit aboutToStart( workgroup, LookupDomainMembers );

    LookupMembersThread *thread = new LookupMembersThread( workgroup, this );
    m_cache.insert( workgroup->key(), thread );

    connect( thread, SIGNAL( finished() ),
            this,   SLOT( slotThreadFinished() ) );
    connect( thread, SIGNAL( hosts( Smb4KWorkgroup *, QList<Smb4KHost> & ) ),
            this,   SLOT( slotHosts( Smb4KWorkgroup *, QList<Smb4KHost> & ) ) );
    connect( thread, SIGNAL( authError( Smb4KBasicNetworkItem * ) ),
            this,   SLOT( slotAuthError( Smb4KBasicNetworkItem * ) ) );

    thread->start();
    thread->lookup( Smb4KSettings::masterBrowsersRequireAuth(), &authInfo, command );
  }
  else
  {
    // The master browser could not be determined. Thus, we
    // will just emit the already known list of domain members.
    QList<Smb4KHost *> workgroup_members = workgroupMembers( workgroup );

    emit hosts( workgroup, workgroup_members );
    emit hostListChanged();
  }
}


void Smb4KScanner::lookupShares( Smb4KHost *host )
{
  Q_ASSERT( host );

  // Find net program
  QString net = KStandardDirs::findExe( "net" );

  if ( net.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "net" );
    return;
  }
  else
  {
    // Go ahead
  }

  // Authentication information.
  Smb4KAuthInfo authInfo( host );
  Smb4KWalletManager::self()->readAuthInfo( &authInfo );

  // Compile the command.
  QString command;
  command += net;
  command += " "+Smb4KSambaOptionsHandler::self()->netOptions( Smb4KSambaOptionsHandler::Share, host );
  command += " -w "+KShell::quoteArg( host->workgroupName() );
  command += " -S "+KShell::quoteArg( host->hostName() );

  if ( host->hasIP() )
  {
    command += " -I " +KShell::quoteArg( host->ip() );
  }
  else
  {
    // Do nothing
  }

  if ( !authInfo.login().isEmpty() )
  {
    command += " -U " +KShell::quoteArg( authInfo.login() );
  }
  else
  {
    command += " -U %";
  }

  m_state = SCANNER_OPEN_HOST;

  // Start looking for the shares.
  if ( m_cache.size() == 0 )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    m_working = true;
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  emit aboutToStart( host, LookupShares );

  LookupSharesThread *thread = new LookupSharesThread( host, this );
  m_cache.insert( host->key(), thread );

  connect( thread, SIGNAL( finished() ),
           this,   SLOT( slotThreadFinished() ) );
  connect( thread, SIGNAL( shares( Smb4KHost *, QList<Smb4KShare> & ) ),
           this,   SLOT( slotShares( Smb4KHost *, QList<Smb4KShare> & ) ) );
  connect( thread, SIGNAL( authError( Smb4KBasicNetworkItem * ) ),
           this,   SLOT( slotAuthError( Smb4KBasicNetworkItem * ) ) );

  thread->start();
  thread->lookup( &authInfo, command );
}


void Smb4KScanner::lookupInfo( Smb4KHost *host )
{
  Q_ASSERT( host );

  // Find the host and check if information has already been aquired.
  Smb4KHost *known_host = findHost( host->hostName(), host->workgroupName() );

  if ( known_host && known_host->infoChecked() )
  {
    emit info( known_host );
    return;
  }
  else
  {
    // Do nothing
  }

  // Find the smbclient program
  QString smbclient = KStandardDirs::findExe( "smbclient" );

  if ( smbclient.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smbclient" );
    return;
  }
  else
  {
    // Go ahead
  }

  // Compile the command.
  QString command;
  command += smbclient;
  command += " -d1";
  command += " -N";
  command += " -W "+KShell::quoteArg( host->workgroupName() );
  command += " -L "+KShell::quoteArg( host->hostName() );

  if ( host->hasIP() )
  {
    command += " -I "+KShell::quoteArg( host->ip() );
  }
  else
  {
    // Do nothing
  }

  // Common options
  command += Smb4KSambaOptionsHandler::self()->smbclientOptions();

  m_state = SCANNER_QUERY_INFO;

  // Start looking for the workgroups.
  if ( m_cache.size() == 0 )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    m_working = true;
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  emit aboutToStart( host, LookupInfo );

  LookupInfoThread *thread = new LookupInfoThread( host, this );
  m_cache.insert( host->key(), thread );

  connect( thread, SIGNAL( finished() ),
           this,   SLOT( slotThreadFinished() ) );
  connect( thread, SIGNAL( info( Smb4KHost * ) ),
           this,   SLOT( slotInformation( Smb4KHost * ) ) );

  thread->start();
  thread->lookup( command );
}


void Smb4KScanner::insertHost( Smb4KHost *host )
{
  Q_ASSERT( host );

  // Add the host to the global list. Use the copy constructor here,
  // so that we do not run into trouble when/if the host is deleted.
  Smb4KHost *new_host = new Smb4KHost( *host );

  if ( addHost( new_host ) )
  {
    // Check if the workgroup is already known. If not, create a new
    // Smb4KWorkgroup object, declare the host a pseudo master and add
    // the workgroup to the list.
    if ( !findWorkgroup( new_host->workgroupName() ) )
    {
      Smb4KWorkgroup *workgroup = new Smb4KWorkgroup( new_host->workgroupName() );
      workgroup->setMasterBrowser( new_host->hostName(), new_host->ip(), true /*pseudo*/ );

      addWorkgroup( workgroup );

      new_host->setIsMasterBrowser( true );  // pseudo master

      emit workgroups( *workgroupsList() );
    }
    else
    {
      // Do nothing
    }

    // Lookup at least the IP address of this host, if necessary:
    if ( !new_host->ipChecked() )
    {
      Smb4KIPAddressScanner::self()->lookup( new_host );
    }
    else
    {
      // Do nothing
    }

    emit hostInserted( new_host );
    emit hostListChanged();
  }
  else
  {
    delete new_host;
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KScanner::slotAboutToQuit()
{
  // Abort all running processes.
  abortAll();
}


void Smb4KScanner::slotThreadFinished()
{
  QStringList keys = m_cache.keys();

  foreach ( QString key, keys )
  {
    BasicScanThread *thread = m_cache.object( key );

    if ( thread->isFinished() )
    {
      switch ( thread->process()->type() )
      {
        case Smb4KProcess::LookupDomains:
        {
          emit finished( thread->networkItem(), LookupDomains );
          break;
        }
        case Smb4KProcess::LookupDomainMembers:
        {
          emit finished( thread->networkItem(), LookupDomainMembers );
          break;
        }
        case Smb4KProcess::LookupShares:
        {
          emit finished( thread->networkItem(), LookupShares );
          break;
        }
        case Smb4KProcess::LookupInfo:
        {
          emit finished( thread->networkItem(), LookupInfo );
          break;
        }
        default:
        {
          break;
        }
      }

      m_cache.remove( key );
    }
    else
    {
      // Do not touch the thread
    }
  }

  if ( m_cache.size() == 0 )
  {
    m_working = false;
    m_state = SCANNER_STOP;
    emit stateChanged();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Still running
  }
}


void Smb4KScanner::slotAuthError( Smb4KBasicNetworkItem *item )
{
  switch ( item->itemType() )
  {
    case Smb4KBasicNetworkItem::Workgroup:
    {
      Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( item );

      if ( workgroup )
      {
        Smb4KHost *master_browser = findHost( workgroup->masterBrowserName(), workgroup->workgroupName() );
        Smb4KAuthInfo authInfo( master_browser );

        if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
        {
          lookupDomainMembers( workgroup );
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
      break;
    }
    case Smb4KBasicNetworkItem::Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( item );

      if ( host )
      {
        Smb4KAuthInfo authInfo( host );

        if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
        {
          lookupShares( host );
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
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KScanner::slotWorkgroups( QList<Smb4KWorkgroup> &workgroups_list )
{
  // Copy the information present in the global workgroup list to
  // the entries in the temporary workgroup list and remove the
  // workgroups that were processed.
  if ( !workgroups_list.isEmpty() )
  {
    for ( int i = 0; i < workgroups_list.size(); ++i )
    {
      Smb4KWorkgroup *workgroup = findWorkgroup( workgroups_list.at( i ).workgroupName() );

      if ( workgroup )
      {
        // Check if the master browser changed.
        if ( QString::compare( workgroup->masterBrowserName(), workgroups_list.at( i ).masterBrowserName(), Qt::CaseInsensitive ) != 0 )
        {
          // Get the old master browser and reset the master browser flag.
          Smb4KHost *old_master_browser = findHost( workgroup->masterBrowserName(), workgroup->workgroupName() );

          if ( old_master_browser )
          {
            old_master_browser->setIsMasterBrowser( false );
          }
          else
          {
            // Do nothing.
          }

          // Lookup new master browser and either set the master browser flag
          // or insert it if it does not exits.
          Smb4KHost *new_master_browser = findHost( workgroups_list.at( i ).masterBrowserName(), workgroups_list.at( i ).workgroupName() );

          if ( new_master_browser )
          {
            if ( workgroups_list.at( i ).hasMasterBrowserIP() )
            {
              new_master_browser->setIP( workgroups_list.at( i ).masterBrowserIP() );
            }
            else
            {
              // Do nothing
            }

            new_master_browser->setIsMasterBrowser( true );
          }
          else
          {
            new_master_browser = new Smb4KHost();
            new_master_browser->setHostName( workgroups_list.at( i ).masterBrowserName() );

            if ( workgroups_list.at( i ).hasMasterBrowserIP() )
            {
              new_master_browser->setIP( workgroups_list.at( i ).masterBrowserIP() );
            }
            else
            {
              // Do nothing
            }

            new_master_browser->setWorkgroupName( workgroups_list.at( i ).workgroupName() );
            new_master_browser->setIsMasterBrowser( true );

            addHost( new_master_browser );
          }
        }
        else
        {
          // Do nothing.
        }

        // Remove the workgroup from the global list.
        removeWorkgroup( workgroup );
      }
      else
      {
        // Lookup new master browser of 'workgroup' and either set the master browser
        // flag or insert it if it does not exits.
        Smb4KHost *new_master_browser = findHost( workgroups_list.at( i ).masterBrowserName(), workgroups_list.at( i ).workgroupName() );

        if ( new_master_browser )
        {
          if ( workgroups_list.at( i ).hasMasterBrowserIP() )
          {
            new_master_browser->setIP( workgroups_list.at( i ).masterBrowserIP() );
          }
          else
          {
            // Do nothing
          }

          new_master_browser->setIsMasterBrowser( true );
        }
        else
        {
          new_master_browser = new Smb4KHost();
          new_master_browser->setHostName( workgroups_list.at( i ).masterBrowserName() );

          if ( workgroups_list.at( i ).hasMasterBrowserIP() )
          {
            new_master_browser->setIP( workgroups_list.at( i ).masterBrowserIP() );
          }
          else
          {
            // Do nothing
          }

          new_master_browser->setWorkgroupName( workgroups_list.at( i ).workgroupName() );
          new_master_browser->setIsMasterBrowser( true );

          addHost( new_master_browser );
        }
      }
    }
  }
  else
  {
    // Do nothing.
  }

  // The global workgroup list only contains obsolete workgroups now.
  // Remove all hosts belonging to those obsolete workgroups from the
  // host list.
  while ( !workgroupsList()->isEmpty() )
  {
    Smb4KWorkgroup *workgroup = workgroupsList()->first();

    QList<Smb4KHost *> obsolete_hosts = workgroupMembers( workgroup );
    QMutableListIterator<Smb4KHost *> it( obsolete_hosts );

    while ( it.hasNext() )
    {
      Smb4KHost *host = it.next();
      removeHost( host );
    }

    removeWorkgroup( workgroup );
  }

  // Add a copy of all workgroups to the global list.
  for ( int i = 0; i < workgroups_list.size(); ++i )
  {
    addWorkgroup( new Smb4KWorkgroup( workgroups_list.at( i ) ) );
  }

  // Check that the workgroup master browsers have an IP address and
  // acquire it if not present.
  for ( int i = 0; i < workgroupsList()->size(); ++i )
  {
    if ( !workgroupsList()->at( i )->hasMasterBrowserIP() )
    {
      // The master browser is in the global host list. Find it.
      Smb4KHost *master_browser = findHost( workgroupsList()->at( i )->masterBrowserName(), workgroupsList()->at( i )->workgroupName() );

      if ( master_browser )
      {
        if ( !master_browser->hasIP() )
        {
          Smb4KIPAddressScanner::self()->lookup( master_browser, true );
        }
        else
        {
          // Do nothing
        }

        workgroupsList()->at( i )->setMasterBrowserIP( master_browser->ip() );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      continue;
    }
  }

  emit hostListChanged();
  emit workgroups( *workgroupsList() );
}


void Smb4KScanner::slotHosts( Smb4KWorkgroup *workgroup, QList<Smb4KHost> &hosts_list )
{
  // Copy some information from the global list to this one before
  // proceeding.
  if ( !hosts_list.isEmpty() )
  {
    for ( int i = 0; i < hosts_list.size(); ++i )
    {
      Smb4KHost *host = findHost( hosts_list.at( i ).hostName(), hosts_list.at( i ).workgroupName() );

      if ( host )
      {
        // Set comment
        if ( hosts_list.at( i ).comment().isEmpty() && !host->comment().isEmpty() )
        {
          hosts_list[i].setComment( host->comment() );
        }
        else
        {
          // Do nothing
        }

        // Set protocol
        if ( hosts_list.at( i ).protocol() != host->protocol() )
        {
          hosts_list[i].setProtocol( host->protocol() );
        }
        else
        {
          // Do nothing
        }

        // Set the additional information
        if ( !hosts_list.at( i ).infoChecked() && host->infoChecked() )
        {
          hosts_list[i].setInfo( host->serverString(), host->osString() );
        }
        else
        {
          // Do nothing
        }

        // Set the IP addresses
        if ( !hosts_list.at( i ).ipChecked() && host->ipChecked() )
        {
          hosts_list[i].setIP( host->ip() );
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
    }
  }
  else
  {
    // Do nothing
  }

  if ( workgroup )
  {
    if ( !hosts_list.isEmpty() )
    {
      // Find the host in the global list and update the entry in the
      // temporary list with its data.
      for ( int i = 0; i < hosts_list.size(); ++i )
      {
        Smb4KHost *host = findHost( hosts_list.at( i ).hostName(), hosts_list.at( i ).workgroupName() );

        if ( host )
        {
          // Remove the host from the global list. It will be inserted
          // again in an instant.
          removeHost( host );
        }
        else
        {
          // Do nothing. The host is new.
        }

        // Add the host to the global list.
        addHost( new Smb4KHost( hosts_list.at( i ) ) );
      }
    }
    else
    {
      // Clear the global hosts list from all hosts belonging to
      // this workgroup/domain.
      QList<Smb4KHost *> obsolete_hosts = workgroupMembers( workgroup );
      QMutableListIterator<Smb4KHost *> it( obsolete_hosts );

      while ( it.hasNext() )
      {
        Smb4KHost *host = it.next();
        removeHost( host );
      }
    }

    // Lookup the IP addresses of the hosts.
    Smb4KIPAddressScanner::self()->lookup( *hostsList() );

    // Get the list of workgroup members.
    QList<Smb4KHost *> workgroup_members = workgroupMembers( workgroup );

    emit hosts( workgroup, workgroup_members );
    emit hostListChanged();
  }
  else
  {
    // The hosts list carries all hosts that are available on the
    // network. We need to clear the global list first.
    clearHostsList();

    // And now we can add the hosts to the global list.
    for ( int i = 0; i < hosts_list.size(); ++i )
    {
      addHost( new Smb4KHost( hosts_list.at( i ) ) );
    }

    // Emit the hosts list.
    emit hosts( NULL, *hostsList() );
    emit hostListChanged();
  }
}


void Smb4KScanner::slotShares( Smb4KHost *host, QList<Smb4KShare> &shares_list )
{
  // Copy some information to the list of shares, before processing it further.
  // The IP address and other information stemming from the host were already
  // inserted by the sending thread.
  if ( !shares_list.isEmpty() )
  {
    for ( int i = 0; i < shares_list.size(); ++i )
    {
      // Set homes users.
      if ( shares_list.at( i ).isHomesShare() )
      {
        Smb4KHomesSharesHandler::self()->setHomesUsers( &shares_list[i] );
      }
      else
      {
        // Do nothing
      }

      // Check if the share is mounted.
      QList<Smb4KShare *> mounted_shares = findShareByUNC( shares_list.at( i ).unc() );

      if ( !mounted_shares.isEmpty() )
      {
        // We prefer the mounted share that is owned by the user.
        // If he/she has not mounted that share, then we take the
        // first entry.
        Smb4KShare *share = mounted_shares.first();

        for ( int j = 0; j < mounted_shares.size(); ++j )
        {
          if ( !mounted_shares.at( j )->isForeign() )
          {
            share = mounted_shares.at( j );
            break;
          }
          else
          {
            continue;
          }
        }

        shares_list[i].setMountData( share );
      }
      else
      {
        // Do nothing
      }

      // Now that we updated the share in the temporary list, we can
      // delete the equivalent one in the global list and append the
      // new entry.
      Smb4KShare *share = findShare( shares_list.at( i ).shareName(), shares_list.at( i ).hostName(), shares_list.at( i ).workgroupName() );

      if ( share )
      {
        removeShare( share );
      }
      else
      {
        // Do nothing. The share is new.
      }

      // Add the share to the list.
      addShare( new Smb4KShare( shares_list.at( i ) ) );
    }
  }
  else
  {
    // Find all shares that belong to this host in the global
    // list and delete them.
    QList<Smb4KShare *> obsolete_shares = sharedResources( host );
    QMutableListIterator<Smb4KShare *> it( obsolete_shares );

    while ( it.hasNext() )
    {
      Smb4KShare *share = it.next();
      removeShare( share );
    }
  }

  QList<Smb4KShare *> shared_resources = sharedResources( host );
  emit shares( host, shared_resources );
}


void Smb4KScanner::slotInformation( Smb4KHost *host )
{
  if ( host->infoChecked() )
  {
    // Copy the information also to host in the global list, if present,
    // or copy 'host' to the global list.
    Smb4KHost *known_host = findHost( host->hostName(), host->workgroupName() );

    if ( known_host )
    {
      known_host->setInfo( host->serverString(), host->osString() );
    }
    else
    {
      known_host = new Smb4KHost( *host );
      addHost( known_host );
    }
  }
  else
  {
    // Do nothing
  }

  // Emit the host here.
  emit info( host );
}


#include <smb4kscanner.moc>
