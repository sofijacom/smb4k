/***************************************************************************
    smb4khost  -  Smb4K's container class for information about a host.
                             -------------------
    begin                : Sa Jan 26 2008
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
#include <QHostAddress>
#include <QAbstractSocket>
#include <QStringList>

// KDE includes
#include <kdebug.h>

// application specific includes
#include <smb4khost.h>
#include <smb4kauthinfo.h>


Smb4KHost::Smb4KHost( const QString &name ) : Smb4KBasicNetworkItem( Host ),
  m_url( QUrl() ), m_workgroup( QString() ), m_ip( QString() ), m_comment( QString() ),
  m_server_string( QString() ), m_os_string( QString() ), m_info_checked( false ),
  m_ip_checked( false ), m_is_master( false ), m_protocol( Automatic )
{
  m_url.setHost( name );
  m_url.setScheme( "smb" );
}


Smb4KHost::Smb4KHost( const Smb4KHost &h ) : Smb4KBasicNetworkItem( Host ),
  m_url( QUrl() ), m_workgroup( h.workgroupName() ), m_ip( h.ip() ), m_comment( h.comment() ),
  m_server_string( h.serverString() ), m_os_string( h.osString() ), m_info_checked( h.infoChecked() ),
  m_ip_checked( h.ipChecked() ), m_is_master( h.isMasterBrowser() ), m_protocol( h.protocol() )
{
  setUNC( h.unc( QUrl::None ) );
}


Smb4KHost::Smb4KHost() : Smb4KBasicNetworkItem( Host ),
  m_url( QUrl() ), m_workgroup( QString() ), m_ip( QString() ), m_comment( QString() ),
  m_server_string( QString() ), m_os_string( QString() ), m_info_checked( false ),
  m_ip_checked( false ), m_is_master( false ), m_protocol( Automatic )
{
}


Smb4KHost::~Smb4KHost()
{
}


void Smb4KHost::setHostName( const QString &name )
{
  m_url.setHost( name );

  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KHost::setUNC( const QString &unc )
{
  // Check that a valid UNC was passed to this function.
  if ( !unc.startsWith( "//" ) && !unc.startsWith( "smb:" ) && unc.count( "/" ) != 2 )
  {
    // The UNC is malformatted.
    return;
  }
  else
  {
    // Do nothing
  }

  m_url.setUrl( unc );

  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KHost::unc( QUrl::FormattingOptions options ) const
{
  return m_url.toString( options|QUrl::RemovePath ).replace( "//"+m_url.host(), "//"+hostName() );
}


void Smb4KHost::setWorkgroupName( const QString &workgroup )
{
  m_workgroup = workgroup;
}


void Smb4KHost::setIP( const QString &ip )
{
  m_ip         = ipIsValid( ip );

  m_ip_checked = true;
}


void Smb4KHost::setComment( const QString &comment )
{
  m_comment = comment;
}


void Smb4KHost::setInfo( const QString &serverString, const QString &osString )
{
  m_server_string = serverString;
  m_os_string     = osString;

  m_info_checked  = true;
}


void Smb4KHost::resetInfo()
{
  m_info_checked = false;

  m_server_string.clear();
  m_os_string.clear();
}


void Smb4KHost::setIsMasterBrowser( bool master )
{
  m_is_master = master;
}


void Smb4KHost::setProtocol( Smb4KHost::Protocol protocol )
{
  m_protocol = protocol;
}


bool Smb4KHost::isEmpty() const
{
  if ( !m_url.isEmpty() )
  {
    return false;
  }

  if ( !m_workgroup.isEmpty() )
  {
    return false;
  }

  if ( !m_ip.isEmpty() )
  {
    return false;
  }

  if ( !m_comment.isEmpty() )
  {
    return false;
  }

  if ( !m_server_string.isEmpty() )
  {
    return false;
  }

  if ( !m_os_string.isEmpty() )
  {
    return false;
  }

  if ( m_protocol != Automatic )
  {
    return false;
  }

  return true;
}


void Smb4KHost::setLogin( const QString &login )
{
  m_url.setUserName( login );
}


void Smb4KHost::setPort( int port )
{
  m_url.setPort( port );
}


bool Smb4KHost::equals( Smb4KHost *host )
{
  Q_ASSERT( host );

  if ( QString::compare( m_url.toString( QUrl::RemovePassword ), host->unc( QUrl::RemovePassword ) ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_workgroup, host->workgroupName() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_ip, host->ip() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_comment, host->comment() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_server_string, host->serverString() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_os_string, host->osString() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( m_protocol != host->protocol() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  return true;
}


void Smb4KHost::setAuthInfo( Smb4KAuthInfo *authInfo )
{
  m_url.setUserName( authInfo->login() );
  m_url.setPassword( authInfo->password() );
}


const QString &Smb4KHost::ipIsValid( const QString &ip )
{
  QHostAddress ip_address( ip );

  if ( ip_address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // The IP address is invalid.
    static_cast<QString>( ip ).clear();
  }

  return ip;
}
