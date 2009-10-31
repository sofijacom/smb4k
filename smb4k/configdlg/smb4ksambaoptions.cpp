/***************************************************************************
    smb4ksambaoptions.cpp  -  This is the configuration page for the
    Samba settings of Smb4K
                             -------------------
    begin                : Mo Jan 26 2004
    copyright            : (C) 2004-2008 by Alexander Reinholdt
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
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTreeWidgetItemIterator>

// KDE includes
#include <klocale.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <kuser.h>
#include <kpushbutton.h>
#include <kmenu.h>

// System includes
#include <unistd.h>
#include <sys/types.h>

// application specific includes
#include <smb4ksambaoptions.h>
#include <core/smb4kglobal.h>
#include <core/smb4ksettings.h>
#include <core/smb4ksambaoptionsinfo.h>

using namespace Smb4KGlobal;


Smb4KSambaOptions::Smb4KSambaOptions( QWidget *parent )
: KTabWidget( parent )
{
  m_edit_column = -1;
  m_edit_value = -1;
  m_edit_widget = NULL;
  m_edit_item = NULL;
  m_collection = new KActionCollection( this );

  //
  // General
  //
  QWidget *general_tab          = new QWidget( this );

  QGridLayout *general_layout   = new QGridLayout( general_tab );
  general_layout->setSpacing( 5 );
  general_layout->setMargin( 0 );

  // General options
  QGroupBox *general_box        = new QGroupBox( i18n( "General Options" ), general_tab );

  QGridLayout *gen_opt_layout   = new QGridLayout( general_box );
  gen_opt_layout->setSpacing( 5 );

  QLabel *netbios_name_label    = new QLabel( Smb4KSettings::self()->netBIOSNameItem()->label(),
                                  general_box );

  KLineEdit *netbios_name       = new KLineEdit( general_box );
  netbios_name->setObjectName( "kcfg_NetBIOSName" );

  QLabel *domain_label          = new QLabel( Smb4KSettings::self()->domainNameItem()->label(),
                                  general_box );

  KLineEdit *domain             = new KLineEdit( general_box );
  domain->setObjectName( "kcfg_DomainName" );

  QLabel *socket_options_label  = new QLabel( Smb4KSettings::self()->socketOptionsItem()->label(),
                                  general_box );

  KLineEdit *socket_options     = new KLineEdit( general_box );
  socket_options->setObjectName( "kcfg_SocketOptions" );

  QLabel *netbios_scope_label   = new QLabel( Smb4KSettings::self()->netBIOSScopeItem()->label(),
                                  general_box );

  KLineEdit *netbios_scope      = new KLineEdit( general_box );
  netbios_scope->setObjectName( "kcfg_NetBIOSScope" );

  gen_opt_layout->addWidget( netbios_name_label, 0, 0, 0 );
  gen_opt_layout->addWidget( netbios_name, 0, 1, 0 );
  gen_opt_layout->addWidget( domain_label, 2, 0, 0 );
  gen_opt_layout->addWidget( domain, 2, 1, 0 );
  gen_opt_layout->addWidget( socket_options_label, 3, 0, 0 );
  gen_opt_layout->addWidget( socket_options, 3, 1, 0 );
  gen_opt_layout->addWidget( netbios_scope_label, 4, 0, 0 );
  gen_opt_layout->addWidget( netbios_scope, 4, 1, 0 );

  // General client options
  QGroupBox *ports_box          = new QGroupBox( i18n( "Remote Ports" ), general_tab );

  QGridLayout *ports_layout     = new QGridLayout( ports_box );
  ports_layout->setSpacing( 5 );

  QLabel *remote_smb_port_label = new QLabel( Smb4KSettings::self()->remoteSMBPortItem()->label(),
                                  ports_box );

  KIntNumInput *remote_smb_port = new KIntNumInput( ports_box );
  remote_smb_port->setObjectName( "kcfg_RemoteSMBPort" );

#ifndef __FreeBSD__
  QLabel *remote_fs_port_label  = new QLabel( Smb4KSettings::self()->remoteFileSystemPortItem()->label(),
                                  ports_box );

  KIntNumInput *remote_fs_port  = new KIntNumInput( ports_box );
  remote_fs_port->setObjectName( "kcfg_RemoteFileSystemPort" );
#endif

  ports_layout->addWidget( remote_smb_port_label, 0, 0, 0 );
  ports_layout->addWidget( remote_smb_port, 0, 1, 0 );
#ifndef __FreeBSD__
  ports_layout->addWidget( remote_fs_port_label, 1, 0, 0 );
  ports_layout->addWidget( remote_fs_port, 1, 1, 0 );
#endif

  QGroupBox *auth_box           = new QGroupBox( i18n( "Authentication" ), general_tab );

  QGridLayout *auth_layout      = new QGridLayout( auth_box );
  auth_layout->setSpacing( 5 );

  QCheckBox *auth_kerberos      = new QCheckBox( Smb4KSettings::self()->useKerberosItem()->label(),
                                  auth_box );
  auth_kerberos->setObjectName( "kcfg_UseKerberos" );

  QCheckBox *auth_machine_acc   = new QCheckBox( Smb4KSettings::self()->machineAccountItem()->label(),
                                  auth_box );
  auth_machine_acc->setObjectName( "kcfg_MachineAccount" );

  auth_layout->addWidget( auth_kerberos, 0, 0, 0 );
  auth_layout->addWidget( auth_machine_acc, 0, 1, 0 );

  QGroupBox *signing_box        = new QGroupBox( i18n( "Signing State" ), general_tab );

  QGridLayout *signing_layout   = new QGridLayout( signing_box );
  signing_layout->setSpacing( 5 );

  QLabel *signing_state_label   = new QLabel( Smb4KSettings::self()->signingStateItem()->label(),
                                  signing_box );
  KComboBox *signing_state      = new KComboBox( signing_box );
  signing_state->setObjectName( "kcfg_SigningState" );
  signing_state->insertItem( Smb4KSettings::EnumSigningState::None,
                             Smb4KSettings::self()->signingStateItem()->choices().value( Smb4KSettings::EnumSigningState::None ).label );
  signing_state->insertItem( Smb4KSettings::EnumSigningState::On,
                             Smb4KSettings::self()->signingStateItem()->choices().value( Smb4KSettings::EnumSigningState::On ).label );
  signing_state->insertItem( Smb4KSettings::EnumSigningState::Off,
                             Smb4KSettings::self()->signingStateItem()->choices().value( Smb4KSettings::EnumSigningState::Off ).label );
  signing_state->insertItem( Smb4KSettings::EnumSigningState::Required,
                             Smb4KSettings::self()->signingStateItem()->choices().value( Smb4KSettings::EnumSigningState::Required ).label );

  signing_layout->addWidget( signing_state_label, 0, 0, 0 );
  signing_layout->addWidget( signing_state, 0, 1, 0 );

  QSpacerItem *spacer1 = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  general_layout->addWidget( general_box, 0, 0, 0 );
  general_layout->addWidget( ports_box, 1, 0, 0 );
  general_layout->addWidget( auth_box, 2, 0, 0 );
  general_layout->addWidget( signing_box, 3, 0, 0 );
  general_layout->addItem( spacer1, 4, 0 );

  addTab( general_tab, i18n( "General Settings" ) );

  //
  // Options for the mount commands
  //
  QWidget *mount_tab            = new QWidget( this );

  QGridLayout *mount_layout     = new QGridLayout( mount_tab );
  mount_layout->setSpacing( 5 );
  mount_layout->setMargin( 0 );

  // Common Options
  QGroupBox *common_options    = new QGroupBox( i18n( "Common Options" ), mount_tab );

  QGridLayout *common_layout   = new QGridLayout( common_options );
  common_layout->setSpacing( 5 );

  QLabel *user_id_label        = new QLabel( Smb4KSettings::self()->userIDItem()->label(),
                                 common_options );

  QWidget *user_widget         = new QWidget( common_options );

  QGridLayout *user_layout     = new QGridLayout( user_widget );
  user_layout->setSpacing( 5 );
  user_layout->setMargin( 0 );

  KLineEdit *user_id           = new KLineEdit( user_widget );
  user_id->setObjectName( "kcfg_UserID" );
  user_id->setAlignment( Qt::AlignRight );
  user_id->setReadOnly( true );

  KPushButton *user_chooser    = new KPushButton( KGuiItem( QString(), "edit-find-user",
                                 i18n( "Choose a different user" ) ), user_widget );
  QMenu *user_menu             = new QMenu( user_chooser );
  user_chooser->setMenu( user_menu );

  QList<KUser> user_list = KUser::allUsers();
  QMap<QString,QString> users;

  for ( int i = 0; i < user_list.size(); ++i )
  {
    users.insert( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                      .arg( user_list.at( i ).uid() ),
                  QString( "%1" ).arg( user_list.at( i ).uid() ) );
  }

  QMap<QString,QString>::const_iterator u_it = users.constBegin();

  while ( u_it != users.constEnd() )
  {
    QAction *user_action = user_menu->addAction( u_it.key() );
    user_action->setData( u_it.value() );

    ++u_it;
  }

  user_layout->addWidget( user_id, 0, 0, 0 );
  user_layout->addWidget( user_chooser, 0, 1, Qt::AlignCenter );

  QLabel *group_id_label       = new QLabel( Smb4KSettings::self()->groupIDItem()->label(),
                                 common_options );

  QWidget *group_widget        = new QWidget( common_options );

  QGridLayout *group_layout    = new QGridLayout( group_widget );
  group_layout->setSpacing( 5 );
  group_layout->setMargin( 0 );

  KLineEdit *group_id          = new KLineEdit( group_widget );
  group_id->setObjectName( "kcfg_GroupID" );
  group_id->setAlignment( Qt::AlignRight );
  group_id->setReadOnly( true );

  KPushButton *group_chooser   = new KPushButton( KGuiItem( QString(), "edit-find-user",
                                 i18n( "Choose a different group" ) ), group_widget );
  QMenu *group_menu            = new QMenu( group_chooser );
  group_chooser->setMenu( group_menu );

  QList<KUserGroup> group_list = KUserGroup::allGroups();
  QMap<QString,QString> groups;

  for ( int i = 0; i < group_list.size(); ++i )
  {
    groups.insert( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                       .arg( group_list.at( i ).gid() ),
                   QString( "%1" ).arg( group_list.at( i ).gid() ) );
  }

  QMap<QString,QString>::const_iterator g_it = groups.constBegin();

  while ( g_it != groups.constEnd() )
  {
    QAction *group_action = group_menu->addAction( g_it.key() );
    group_action->setData( g_it.value() );

    ++g_it;
  }

  group_layout->addWidget( group_id, 0, 0, 0 );
  group_layout->addWidget( group_chooser, 0, 1, Qt::AlignCenter );

  QLabel *fmask_label          = new QLabel( Smb4KSettings::self()->fileMaskItem()->label(),
                                 common_options );

  KLineEdit *fmask             = new KLineEdit( common_options );
  fmask->setObjectName( "kcfg_FileMask" );
  fmask->setAlignment( Qt::AlignRight );

  QLabel *dmask_label          = new QLabel( Smb4KSettings::self()->directoryMaskItem()->label(),
                                 common_options );

  KLineEdit *dmask             = new KLineEdit( common_options );
  dmask->setObjectName( "kcfg_DirectoryMask" );
  dmask->setAlignment( Qt::AlignRight );

#ifndef __FreeBSD__
  QLabel *write_access_label   = new QLabel( Smb4KSettings::self()->writeAccessItem()->label(),
                                 common_options );

  KComboBox *write_access      = new KComboBox( common_options );
  write_access->setObjectName( "kcfg_WriteAccess" );
  write_access->insertItem( Smb4KSettings::EnumWriteAccess::ReadWrite,
                            Smb4KSettings::self()->writeAccessItem()->choices().value( Smb4KSettings::EnumWriteAccess::ReadWrite ).label );
  write_access->insertItem( Smb4KSettings::EnumWriteAccess::ReadOnly,
                            Smb4KSettings::self()->writeAccessItem()->choices().value( Smb4KSettings::EnumWriteAccess::ReadOnly ).label );
#endif

  QLabel *charset_label        = new QLabel( Smb4KSettings::self()->clientCharsetItem()->label(),
                                 common_options );

  KComboBox *charset           = new KComboBox( common_options );
  charset->setObjectName( "kcfg_ClientCharset" );
  charset->insertItem( Smb4KSettings::EnumClientCharset::default_charset,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::default_charset ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_1,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_1 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_2,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_2 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_3,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_3 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_4,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_4 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_5,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_5 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_6,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_6 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_7,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_7 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_8,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset:: iso8859_8).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_9,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_9 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_13,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_13 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_14,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_14 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::iso8859_15,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::iso8859_15 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::utf8,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::utf8 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::koi8_r,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::koi8_r ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::koi8_u,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::koi8_u ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::koi8_ru,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::koi8_ru ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::cp1251,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::cp1251 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::gb2312,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::gb2312 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::big5,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::big5 ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::euc_jp,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::euc_jp ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::euc_kr,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::euc_kr ).label );
  charset->insertItem( Smb4KSettings::EnumClientCharset::tis_620,
                       Smb4KSettings::self()->clientCharsetItem()->choices().value( Smb4KSettings::EnumClientCharset::tis_620 ).label );

#ifdef __FreeBSD__
  QLabel *codepage_label       = new QLabel( Smb4KSettings::self()->serverCodepageItem()->label(),
                                 common_options );
  codepage_label->setObjectName( "CodepageLabel" );

  KComboBox *codepage          = new KComboBox( common_options );
  codepage->setObjectName( "kcfg_ServerCodepage" );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::default_codepage,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::default_codepage ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp437,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp437 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp720,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp720 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp737,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp737 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp775,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp775 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp850,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp850 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp852,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp852 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp855,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp855 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp857,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp857 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp858,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp858 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp860,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp860 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp861,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp861 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp862,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp862 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp863,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp863 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp864,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp864 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp865,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp865 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp866,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp866 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp869,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp869 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp874,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp874 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp932,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp932 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp936,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp936 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp949,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp949 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp950,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp950 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1250,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1250 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1251,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1251 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1252,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1252 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1253,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1253 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1254,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1254 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1255,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1255 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1256,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1256 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1257,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1257 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::cp1258,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::cp1258 ).label );
  codepage->insertItem( Smb4KSettings::EnumServerCodepage::unicode,
                        Smb4KSettings::self()->serverCodepageItem()->choices().value( Smb4KSettings::EnumServerCodepage::unicode ).label );
#endif

#ifndef __FreeBSD__
  common_layout->addWidget( user_id_label, 0, 0, 0 );
  common_layout->addWidget( user_widget, 0, 1, 0 );
  common_layout->addWidget( group_id_label, 1, 0, 0 );
  common_layout->addWidget( group_widget, 1, 1, 0 );
  common_layout->addWidget( fmask_label, 2, 0, 0 );
  common_layout->addWidget( fmask, 2, 1, 0 );
  common_layout->addWidget( dmask_label, 3, 0, 0 );
  common_layout->addWidget( dmask, 3, 1, 0 );
  common_layout->addWidget( write_access_label, 4, 0, 0 );
  common_layout->addWidget( write_access, 4, 1, 0 );
  common_layout->addWidget( charset_label, 5, 0, 0 );
  common_layout->addWidget( charset, 5, 1, 0 );
#else
  common_layout->addWidget( user_id_label, 0, 0, 0 );
  common_layout->addWidget( user_widget, 0, 1, 0 );
  common_layout->addWidget( group_id_label, 1, 0, 0 );
  common_layout->addWidget( group_widget, 1, 1, 0 );
  common_layout->addWidget( fmask_label, 2, 0, 0 );
  common_layout->addWidget( fmask, 2, 1, 0 );
  common_layout->addWidget( dmask_label, 3, 0, 0 );
  common_layout->addWidget( dmask, 3, 1, 0 );
  common_layout->addWidget( charset_label, 4, 0, 0 );
  common_layout->addWidget( charset, 4, 1, 0 );
  common_layout->addWidget( codepage_label, 5, 0, 0 );
  common_layout->addWidget( codepage, 5, 1, 0 );
#endif

#ifndef __FreeBSD__
  // Advanced CIFS options
  QGroupBox *advanced_options  = new QGroupBox( i18n( "Advanced Options" ), mount_tab );

  QGridLayout *advanced_layout = new QGridLayout( advanced_options );
  advanced_layout->setSpacing( 5 );

  QCheckBox *permission_checks = new QCheckBox( Smb4KSettings::self()->permissionChecksItem()->label(),
                                 advanced_options );
  permission_checks->setObjectName( "kcfg_PermissionChecks" );

  QCheckBox *client_controls   = new QCheckBox( Smb4KSettings::self()->clientControlsIDsItem()->label(),
                                 advanced_options );
  client_controls->setObjectName( "kcfg_ClientControlsIDs" );

  QCheckBox *server_inodes     = new QCheckBox( Smb4KSettings::self()->serverInodeNumbersItem()->label(),
                                 advanced_options );
  server_inodes->setObjectName( "kcfg_ServerInodeNumbers" );

  QCheckBox *no_inode_caching  = new QCheckBox( Smb4KSettings::self()->noInodeDataCachingItem()->label(),
                                 advanced_options );
  no_inode_caching->setObjectName( "kcfg_NoInodeDataCaching" );

  QCheckBox *reserved_chars    = new QCheckBox( Smb4KSettings::self()->translateReservedCharsItem()->label(),
                                 advanced_options );
  reserved_chars->setObjectName( "kcfg_TranslateReservedChars" );

  QCheckBox *no_locking        = new QCheckBox( Smb4KSettings::self()->noLockingItem()->label(),
                                 advanced_options );
  no_locking->setObjectName( "kcfg_NoLocking" );

  QWidget *c_extra_widget      = new QWidget( advanced_options );

  QGridLayout *c_extra_layout  = new QGridLayout( c_extra_widget );
  c_extra_layout->setSpacing( 5 );
  c_extra_layout->setMargin( 0 );

  QLabel *add_options_label    = new QLabel( Smb4KSettings::self()->customCIFSOptionsItem()->label(),
                                 c_extra_widget );

  KLineEdit *additional_opts   = new KLineEdit( c_extra_widget );
  additional_opts->setObjectName( "kcfg_CustomCIFSOptions" );

  c_extra_layout->addWidget( add_options_label, 0, 0, 0 );
  c_extra_layout->addWidget( additional_opts, 0, 1, 0 );

  advanced_layout->addWidget( permission_checks, 0, 0, 0 );
  advanced_layout->addWidget( client_controls, 0, 1, 0 );
  advanced_layout->addWidget( server_inodes, 1, 0, 0 );
  advanced_layout->addWidget( no_inode_caching, 1, 1, 0 );
  advanced_layout->addWidget( reserved_chars, 2, 0, 0 );
  advanced_layout->addWidget( no_locking, 2, 1, 0 );
  advanced_layout->addWidget( c_extra_widget, 3, 0, 1, 2, 0 );
#endif

  QFrame *note                 = new QFrame( mount_tab );

  QGridLayout *note_layout     = new QGridLayout( note );
  note_layout->setSpacing( 10 );
  note_layout->setMargin( 5 );

  QLabel *important_pix        = new QLabel( note );
  important_pix->setPixmap( KIconLoader::global()->loadIcon( "emblem-important", KIconLoader::Desktop, KIconLoader::SizeMedium ) );
  important_pix->adjustSize();

  QLabel *message              = new QLabel( note );
  message->setText( i18n( "<qt>If you experience problems due to insufficient privileges when mounting shares, you should enable the appropriate settings in the <b>Super User</b> configuration page.</qt>" ) );
  message->setTextFormat( Qt::AutoText );
  message->setWordWrap( true );
  message->setAlignment( Qt::AlignJustify );

  note_layout->addWidget( important_pix, 0, 0, Qt::AlignCenter );
  note_layout->addWidget( message, 0, 1, Qt::AlignVCenter );

  note_layout->setColumnStretch( 1, 1 );

  QSpacerItem *spacer2 = new QSpacerItem( 10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding );

  mount_layout->addWidget( common_options, 0, 0, 0 );
#ifndef __FreeBSD__
  mount_layout->addWidget( advanced_options, 1, 0, 0 );
    mount_layout->addWidget( note, 2, 0, 0 );
  mount_layout->addItem( spacer2, 3, 0 );
#else
  mount_layout->addWidget( note, 1, 0, 0 );
  mount_layout->addItem( spacer2, 2, 0 );
#endif

  addTab( mount_tab, i18n( "Mounting" ) );

  //
  // Options for the client programs
  //
  QWidget *client_programs_tab  = new QWidget( this );

  QGridLayout *client_layout    = new QGridLayout( client_programs_tab );
  client_layout->setSpacing( 5 );
  client_layout->setMargin( 0 );

  // 'net' program
  QGroupBox *net_box            = new QGroupBox( i18n( "net" ), client_programs_tab );

  QGridLayout *net_layout       = new QGridLayout( net_box );
  net_layout->setSpacing( 5 );

  QLabel *proto_hint_label      = new QLabel( Smb4KSettings::self()->protocolHintItem()->label(),
                                  net_box );

  KComboBox *protocol_hint      = new KComboBox( net_box );
  protocol_hint->setObjectName( "kcfg_ProtocolHint" );
  protocol_hint->insertItem( Smb4KSettings::EnumProtocolHint::Automatic,
                             Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::EnumProtocolHint::Automatic ).label );
  protocol_hint->insertItem( Smb4KSettings::EnumProtocolHint::RPC,
                             Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::EnumProtocolHint::RPC ).label );
  protocol_hint->insertItem( Smb4KSettings::EnumProtocolHint::RAP,
                             Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::EnumProtocolHint::RAP ).label );
  protocol_hint->insertItem( Smb4KSettings::EnumProtocolHint::ADS,
                             Smb4KSettings::self()->protocolHintItem()->choices().value( Smb4KSettings::EnumProtocolHint::ADS ).label );

  net_layout->addWidget( proto_hint_label, 0, 0, 0 );
  net_layout->addWidget( protocol_hint, 0, 1, 0 );

  // 'smbclient' program
  QGroupBox *smbclient_box      = new QGroupBox( i18n( "smbclient" ), client_programs_tab );

  QGridLayout *smbclient_layout = new QGridLayout( smbclient_box );
  smbclient_layout->setSpacing( 5 );

  QLabel *name_resolve_label   = new QLabel( Smb4KSettings::self()->nameResolveOrderItem()->label(),
                                 smbclient_box );

  KLineEdit *name_resolve      = new KLineEdit( smbclient_box );
  name_resolve->setObjectName( "kcfg_NameResolveOrder" );

  QLabel *buffer_size_label    = new QLabel( Smb4KSettings::self()->bufferSizeItem()->label(),
                                 smbclient_box );

  KIntNumInput *buffer_size    = new KIntNumInput( smbclient_box );
  buffer_size->setObjectName( "kcfg_BufferSize" );
  buffer_size->setSuffix( i18n( " Bytes" ) );

  smbclient_layout->addWidget( name_resolve_label, 0, 0, 0 );
  smbclient_layout->addWidget( name_resolve, 0, 1, 0 );
  smbclient_layout->addWidget( buffer_size_label, 2, 0, 0 );
  smbclient_layout->addWidget( buffer_size, 2, 1, 0 );

  // 'nmblookup' program
  QGroupBox *nmblookup_box     = new QGroupBox( i18n( "nmblookup" ), client_programs_tab );

  QGridLayout *nmblookup_layout = new QGridLayout( nmblookup_box );
  nmblookup_layout->setSpacing( 5 );

  QLabel *broadcast_add_label  = new QLabel( Smb4KSettings::self()->broadcastAddressItem()->label(),
                                 nmblookup_box );

  KLineEdit *broadcast_address = new KLineEdit( nmblookup_box );
  broadcast_address->setObjectName( "kcfg_BroadcastAddress" );

  QCheckBox *port_137          = new QCheckBox( Smb4KSettings::self()->usePort137Item()->label(),
                                 nmblookup_box );
  port_137->setObjectName( "kcfg_UsePort137" );

  nmblookup_layout->addWidget( broadcast_add_label, 0, 0, 0 );
  nmblookup_layout->addWidget( broadcast_address, 0, 1, 0 );
  nmblookup_layout->addWidget( port_137, 1, 0, 1, 2, 0 );

  // 'smbtree' program
  QGroupBox *smbtree_box       = new QGroupBox( i18n( "smbtree" ), client_programs_tab );

  QGridLayout *smbtree_layout  = new QGridLayout( smbtree_box );
  smbtree_layout->setSpacing( 5 );

  QCheckBox *smbtree_bcasts    = new QCheckBox( Smb4KSettings::self()->smbtreeSendBroadcastsItem()->label(),
                                 smbtree_box );
  smbtree_bcasts->setObjectName( "kcfg_SmbtreeSendBroadcasts" );

  smbtree_layout->addWidget( smbtree_bcasts, 0, 0, 0 );

  QSpacerItem *spacer3 = new QSpacerItem( 10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding );

  client_layout->addWidget( net_box, 0, 0, 0 );
  client_layout->addWidget( nmblookup_box, 1, 0, 0 );
  client_layout->addWidget( smbclient_box, 2, 0, 0 );
  client_layout->addWidget( smbtree_box, 3, 0, 0 );
  client_layout->addItem( spacer3, 4, 0 );

  addTab( client_programs_tab, i18n( "Client Programs" ) );

  //
  // Custom options
  //
  QWidget *custom_tab        = new QWidget( this );

  QGridLayout *custom_layout = new QGridLayout( custom_tab );
  custom_layout->setSpacing( 5 );
  custom_layout->setMargin( 0 );

  m_custom_options           = new QTreeWidget( custom_tab );
  m_custom_options->setObjectName( "CustomOptionsList" );
  m_custom_options->viewport()->installEventFilter( this );
  m_custom_options->setSelectionMode( QTreeWidget::ExtendedSelection );
  m_custom_options->setRootIsDecorated( false );
  m_custom_options->setEditTriggers( QTreeWidget::NoEditTriggers );
  m_custom_options->setContextMenuPolicy( Qt::CustomContextMenu );
#ifndef __FreeBSD__
  m_custom_options->setColumnCount( 7 );
#else
  m_custom_options->setColumnCount( 6 );
#endif

  QStringList header_labels;
  header_labels.append( i18n( "Item" ) );
  header_labels.append( i18n( "Protocol" ) );
#ifndef __FreeBSD__
  header_labels.append( i18n( "Write Access" ) );
#endif
  header_labels.append( i18n( "Kerberos" ) );
  header_labels.append( i18n( "UID" ) );
  header_labels.append( i18n( "GID" ) );
  header_labels.append( i18n( "Port" ) );

  m_custom_options->setHeaderLabels( header_labels );

  m_menu = new KActionMenu( m_custom_options );

  KAction *edit_action      = new KAction( KIcon( "edit-rename" ), i18n( "Edit" ),
                              m_collection );
  edit_action->setEnabled( false );

  KAction *remove_action    = new KAction( KIcon( "edit-delete" ), i18n( "Remove" ),
                               m_collection );
  remove_action->setEnabled( false );

  m_collection->addAction( "edit_action", edit_action );
  m_collection->addAction( "remove_action", remove_action );

  m_menu->addAction( edit_action );
  m_menu->addAction( remove_action );

  custom_layout->addWidget( m_custom_options, 0, 0, 0 );

  addTab( custom_tab, i18n( "Custom Options" ) );

  //
  // Connections
  //
  connect( user_menu,        SIGNAL( triggered( QAction * ) ),
           this,             SLOT( slotNewUserTriggered( QAction * ) ) );

  connect( group_menu,       SIGNAL( triggered( QAction * ) ),
           this,             SLOT( slotNewGroupTriggered( QAction * ) ) );

  connect( m_custom_options, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
           this,             SLOT( slotEditCustomItem( QTreeWidgetItem *, int ) ) );

  connect( m_custom_options, SIGNAL( itemSelectionChanged() ),
           this,             SLOT( slotItemSelectionChanged() ) );

  connect( m_custom_options, SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this,             SLOT( slotCustomContextMenuRequested( const QPoint & ) ) );

  connect( m_menu->menu(),   SIGNAL( triggered( QAction * ) ),
           this,             SLOT( slotMenuActionTriggered( QAction * ) ) );
}


Smb4KSambaOptions::~Smb4KSambaOptions()
{
}


bool Smb4KSambaOptions::eventFilter( QObject *obj, QEvent *e )
{
  bool eat_event = false;

  if ( obj == m_custom_options->viewport() )
  {
    switch ( e->type() )
    {
      case QEvent::MouseButtonPress:
      {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>( e );

        QTreeWidgetItem *item = m_custom_options->itemAt( m_custom_options->viewport()->mapFromGlobal( mouse_event->globalPos() ) );

        if ( (m_edit_item && m_edit_item != item) )
        {
          // When the user clicks somewhere in the viewport,
          // in 99.9% that means he/she wants to get rid of
          // the edit widget without modifying anything. Thus,
          // we will just remove it here.
          m_custom_options->removeItemWidget( m_edit_item, m_edit_column );

          m_edit_column = -1;
          m_edit_value = -1;
          m_edit_item = NULL;

          for ( int i = 0; i < m_custom_options->columnCount(); ++i )
          {
            m_custom_options->resizeColumnToContents( i );
          }

          m_custom_options->sortItems( ItemName, Qt::AscendingOrder );
        }
        else
        {
          // Do nothing
        }

        break;
      }
      case QEvent::KeyPress:
      {
        QKeyEvent *key_event = static_cast<QKeyEvent *>( e );

        switch ( key_event->key() )
        {
          case Qt::Key_Return:
          case Qt::Key_Enter:
          {
            switch ( m_edit_column )
            {
              case Port:
              {
                KIntNumInput *input = static_cast<KIntNumInput *>( m_edit_widget );

                if ( input )
                {
                  disconnect( input, SIGNAL( valueChanged( int ) ),
                              this,  SLOT( slotCustomIntValueChanged( int ) ) );

                  input->clearFocus();
                  input->unsetCursor();

                  m_custom_options->removeItemWidget( m_edit_item, m_edit_column );
                  m_edit_item->setText( m_edit_column, QString( "%1" ).arg( m_edit_value ) );

                  m_edit_column = -1;
                  m_edit_value = -1;
                  m_edit_item = NULL;

                  for ( int i = 0; i < m_custom_options->columnCount(); ++i )
                  {
                    m_custom_options->resizeColumnToContents( i );
                  }

                  m_custom_options->sortItems( ItemName, Qt::AscendingOrder );

                  eat_event = true;
                }
                else
                {
                  // Do nothing
                }

                emit customSettingsModified();

                break;
              }
              default:
              {
                break;
              }
            }
          }
          default:
          {
            break;
          }
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
    // Do nothing
  }

  return (!eat_event ? KTabWidget::eventFilter( obj, e ) : eat_event);
}


void Smb4KSambaOptions::insertCustomOptions( const QList<Smb4KSambaOptionsInfo *> &list )
{
  // Default values
  QString protocol_hint;
#ifndef __FreeBSD__
  QString default_file_system;
  QString default_write_access;
#endif
  QString default_kerberos;

  // Protocol hint
  switch ( Smb4KSettings::protocolHint() )
  {
    case Smb4KSettings::EnumProtocolHint::Automatic:
    {
      // In this case the user leaves it to the net
      // command to determine the right protocol.
      protocol_hint = i18n( "automatic" );

      break;
    }
    case Smb4KSettings::EnumProtocolHint::RPC:
    {
      protocol_hint = "RPC";

      break;
    }
    case Smb4KSettings::EnumProtocolHint::RAP:
    {
      protocol_hint = "RAP";

      break;
    }
    case Smb4KSettings::EnumProtocolHint::ADS:
    {
      protocol_hint = "ADS";

      break;
    }
    default:
    {
      protocol_hint = QString();

      break;
    }
  }

#ifndef __FreeBSD__
  // Default write access
  switch ( Smb4KSettings::writeAccess() )
  {
    case Smb4KSettings::EnumWriteAccess::ReadWrite:
    {
      default_write_access = i18n( "read-write" );

      break;
    }
    case Smb4KSettings::EnumWriteAccess::ReadOnly:
    {
      default_write_access = i18n( "read-only" );

      break;
    }
    default:
    {
      break;
    }
  }
#endif

  // Default Kerberos
  if ( Smb4KSettings::useKerberos() )
  {
    default_kerberos = i18n( "yes" );
  }
  else
  {
    default_kerberos = i18n( "no" );
  }

  // Put the items into the tree widget.
  for ( int i = 0; i < list.size(); ++i )
  {
    switch ( list.at( i )->type() )
    {
      case Smb4KSambaOptionsInfo::Host:
      {
        QTreeWidgetItem *item = new QTreeWidgetItem( m_custom_options, Host );
        item->setIcon( ItemName, KIcon( "network-server" ) );

        // UNC
        item->setText( ItemName, list.at( i )->unc() );

        // Protocol
        switch ( list.at( i )->protocol() )
        {
          case Smb4KSambaOptionsInfo::Automatic:
          {
            item->setText( Protocol, i18n( "automatic" ) );

            break;
          }
          case Smb4KSambaOptionsInfo::RPC:
          {
            item->setText( Protocol, "RPC" );

            break;
          }
          case Smb4KSambaOptionsInfo::RAP:
          {
            item->setText( Protocol, "RAP" );

            break;
          }
          case Smb4KSambaOptionsInfo::ADS:
          {
            item->setText( Protocol, "ADS" );

            break;
          }
          case Smb4KSambaOptionsInfo::UndefinedProtocol:
          {
            if ( !protocol_hint.isEmpty() )
            {
              item->setText( Protocol, protocol_hint );
            }
            else
            {
              item->setText( Protocol, "-" );
            }

            break;
          }
          default:
          {
            break;
          }
        }

#ifndef __FreeBSD__
        // Write access
        item->setText( WriteAccess, "-" );
#endif
        // Kerberos
        switch ( list.at( i )->useKerberos() )
        {
          case Smb4KSambaOptionsInfo::UseKerberos:
          {
            item->setText( Kerberos, i18n( "yes" ) );

            break;
          }
          case Smb4KSambaOptionsInfo::NoKerberos:
          {
            item->setText( Kerberos, i18n( "no" ) );

            break;
          }
          case Smb4KSambaOptionsInfo::UndefinedKerberos:
          {
            item->setText( Kerberos, default_kerberos );

            break;
          }
          default:
          {
            break;
          }
        }

        // UID
        item->setText( UID, "-" );

        // GID
        item->setText( GID, "-" );

        // Port
        if ( list.at( i )->port() != -1 )
        {
          item->setText( Port, QString( "%1" ).arg( list.at( i )->port() ) );
        }
        else
        {
          item->setText( Port, QString( "%1" ).arg( Smb4KSettings::remoteSMBPort() ) );
        }

        break;
      }
      case Smb4KSambaOptionsInfo::Share:
      {
        QTreeWidgetItem *item = new QTreeWidgetItem( m_custom_options, Share );
        item->setIcon( ItemName, KIcon( "folder-remote" ) );

        // UNC
        item->setText( ItemName, list.at( i )->unc() );

        // Protocol
        item->setText( Protocol, "-" );

#ifndef __FreeBSD__
        // Write access
        switch ( list.at( i )->writeAccess() )
        {
          case Smb4KSambaOptionsInfo::ReadWrite:
          {
            item->setText( WriteAccess, i18n( "read-write" ) );

            break;
          }
          case Smb4KSambaOptionsInfo::ReadOnly:
          {
            item->setText( WriteAccess, i18n( "read-only" ) );

            break;
          }
          default:
          {
            item->setText( WriteAccess, default_write_access );

            break;
          }
        }

        // Kerberos
        item->setText( Kerberos, "-" );
#endif
        // UID
        KUser user( list.at( i )->uid() );
        item->setText( UID, QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() ) );

        // GID
        KUserGroup group( list.at( i )->gid() );
        item->setText( GID, QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() ) );

        // Port
        if ( list.at( i )->port() != -1 )
        {
          item->setText( Port, QString( "%1" ).arg( list.at( i )->port() ) );
        }
        else
        {
          item->setText( Port, QString( "%1" ).arg( Smb4KSettings::remoteFileSystemPort() ) );
        }

        break;
      }
      default:
      {
        break;
      }
    }
  }

  for ( int i = 0; i < m_custom_options->columnCount(); ++i )
  {
    m_custom_options->resizeColumnToContents( i );
  }

  m_custom_options->sortItems( ItemName, Qt::AscendingOrder );
}


QList<Smb4KSambaOptionsInfo *> Smb4KSambaOptions::getCustomOptions()
{
  QList<Smb4KSambaOptionsInfo *> list;

  QTreeWidgetItemIterator it( m_custom_options );

  while ( *it )
  {
    Smb4KSambaOptionsInfo *info = new Smb4KSambaOptionsInfo();

    // UNC
    info->setUNC( (*it)->text( ItemName ) );

    // Protocol
    if ( QString::compare( (*it)->text( Protocol ), i18n( "automatic" ), Qt::CaseInsensitive ) == 0 )
    {
      info->setProtocol( Smb4KSambaOptionsInfo::Automatic );
    }
    else if ( QString::compare( (*it)->text( Protocol ), "RPC", Qt::CaseInsensitive ) == 0 )
    {
      info->setProtocol( Smb4KSambaOptionsInfo::RPC );
    }
    else if ( QString::compare( (*it)->text( Protocol ), "RAP", Qt::CaseInsensitive ) == 0 )
    {
      info->setProtocol( Smb4KSambaOptionsInfo::RAP );
    }
    else if ( QString::compare( (*it)->text( Protocol ), "ADS", Qt::CaseInsensitive ) == 0 )
    {
      info->setProtocol( Smb4KSambaOptionsInfo::ADS );
    }
    else
    {
      info->setProtocol( Smb4KSambaOptionsInfo::UndefinedProtocol );
    }

#ifndef __FreeBSD__
    // Write access
    if ( QString::compare( (*it)->text( WriteAccess ), i18n( "read-write" ), Qt::CaseInsensitive ) == 0 )
    {
      info->setWriteAccess( Smb4KSambaOptionsInfo::ReadWrite );
    }
    else if ( QString::compare( (*it)->text( WriteAccess ), i18n( "read-only" ), Qt::CaseInsensitive ) == 0 )
    {
      info->setWriteAccess( Smb4KSambaOptionsInfo::ReadOnly );
    }
    else
    {
      info->setWriteAccess( Smb4KSambaOptionsInfo::UndefinedWriteAccess );
    }
#endif
    // Kerberos
    if ( QString::compare( (*it)->text( Kerberos ), i18n( "yes" ), Qt::CaseInsensitive ) == 0 )
    {
      info->setUseKerberos( Smb4KSambaOptionsInfo::UseKerberos );
    }
    else if ( QString::compare( (*it)->text( Kerberos ), i18n( "no" ), Qt::CaseInsensitive ) == 0 )
    {
      info->setUseKerberos( Smb4KSambaOptionsInfo::NoKerberos );
    }
    else
    {
      info->setUseKerberos( Smb4KSambaOptionsInfo::UndefinedKerberos );
    }

    // UID
    if ( (*it)->type() == Share )
    {
      info->setUID( (uid_t)(*it)->text( UID ).section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
    }
    else
    {
      // Do nothing
    }

    // GID
    if ( (*it)->type() == Share )
    {
      info->setGID( (gid_t)(*it)->text( GID ).section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
    }
    else
    {
      // Do nothing
    }

    // Port
    info->setPort( (*it)->text( Port ).toInt() );

    // Put info object into the list.
    list.append( info );

    ++it;
  }

  return list;
}


/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSambaOptions::slotNewUserTriggered( QAction *action )
{
  KLineEdit *user_id = findChild<KLineEdit *>( "kcfg_UserID" );

  if ( user_id )
  {
    user_id->setText( action->data().toString() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSambaOptions::slotNewGroupTriggered( QAction *action )
{
  KLineEdit *group_id = findChild<KLineEdit *>( "kcfg_GroupID" );

  if ( group_id )
  {
    group_id->setText( action->data().toString() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSambaOptions::slotEditCustomItem( QTreeWidgetItem *item, int column )
{
  if ( item )
  {
    if ( m_edit_item != item || m_edit_column != column )
    {
      // When the user clicks somewhere in the viewport,
      // in 99.9% that means he/she wants to get rid of
      // the edit widget without modifying anything. Thus,
      // we will just remove it here.
      m_custom_options->removeItemWidget( m_edit_item, m_edit_column );

      m_edit_column = -1;
      m_edit_value = -1;
      m_edit_item = NULL;
    }
    else
    {
      // Do nothing
    }

    switch ( item->type() )
    {
      case Host:
      {
        switch ( column )
        {
          case Protocol:
          {
            QStringList choices;
            choices.append( i18n( "automatic" ) );
            choices.append( "RPC" );
            choices.append( "RAP" );
            choices.append( "ADS" );

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );

            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            m_edit_widget = combo;
            m_edit_column = column;
            m_edit_item = item;

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );

            break;
          }
          case Kerberos:
          {
            QStringList choices;
            choices.append( i18n( "yes" ) );
            choices.append( i18n( "no" ) );

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );

            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            m_edit_widget = combo;
            m_edit_column = column;
            m_edit_item = item;

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );

            break;
          }
          case Port:
          {
            KIntNumInput *input = new KIntNumInput( m_custom_options );
            input->setAutoFillBackground( true );
            input->setMinimum( 0 );
            input->setMaximum( 65535 );
            input->setValue( item->text( column ).toInt() );

            m_custom_options->setItemWidget( item, column, input );
            input->adjustSize();

            m_edit_widget = input;
            m_edit_column = column;
            m_edit_item = item;

            connect( input, SIGNAL( valueChanged( int ) ),
                     this,  SLOT( slotCustomIntValueChanged( int ) ) );

            break;
          }
          default:
          {
            break;
          }
        }

        break;
      }
      case Share:
      {
        switch ( column )
        {
#ifndef __FreeBSD__
          case WriteAccess:
          {
            QStringList choices;
            choices.append( i18n( "read-write" ) );
            choices.append( i18n( "read-only" ) );

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );

            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            m_edit_widget = combo;
            m_edit_column = column;
            m_edit_item = item;

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );

            break;
          }
#endif
          case Port:
          {
            KIntNumInput *input = new KIntNumInput( m_custom_options );
            input->setAutoFillBackground( true );
            input->setMinimum( 0 );
            input->setMaximum( 65535 );
            input->setValue( item->text( column ).toInt() );

            m_custom_options->setItemWidget( item, column, input );
            input->adjustSize();

            m_edit_widget = input;
            m_edit_column = column;
            m_edit_item = item;

            connect( input, SIGNAL( valueChanged( int ) ),
                     this,  SLOT( slotCustomIntValueChanged( int ) ) );

            break;
          }
          case UID:
          {
            QList<KUser> user_list = KUser::allUsers();
            QStringList choices;

            for ( int i = 0; i < user_list.size(); ++i )
            {
              choices.append( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                                  .arg( user_list.at( i ).uid() ) );
            }

            choices.sort();

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );

            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            m_edit_widget = combo;
            m_edit_column = column;
            m_edit_item = item;

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );

            break;
          }
          case GID:
          {
            QList<KUserGroup> group_list = KUserGroup::allGroups();
            QStringList choices;

            for ( int i = 0; i < group_list.size(); ++i )
            {
              choices.append( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                                  .arg( group_list.at( i ).gid() ) );
            }

            choices.sort();

            KComboBox *combo = new KComboBox( m_custom_options );
            combo->setAutoFillBackground( true );
            combo->addItems( choices );
            int index = combo->findText( item->text( column ) );
            combo->setCurrentIndex( index );

            m_custom_options->setItemWidget( item, column, combo );
            combo->adjustSize();

            m_edit_widget = combo;
            m_edit_column = column;
            m_edit_item = item;

            connect( combo, SIGNAL( activated( const QString & ) ),
                     this,  SLOT( slotCustomTextChanged( const QString & ) ) );

            break;
          }
          default:
          {
            break;
          }
        }

        break;
      }
      default:
      {
        break;
      }
    }

    for ( int i = 0; i < m_custom_options->columnCount(); ++i )
    {
      m_custom_options->resizeColumnToContents( i );
    }

    m_custom_options->sortItems( ItemName, Qt::AscendingOrder );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSambaOptions::slotCustomTextChanged( const QString &text )
{
  KComboBox *combo = static_cast<KComboBox *>( m_edit_widget );

  disconnect( combo, SIGNAL( activated( const QString & ) ),
              this,  SLOT( slotCustomTextChanged( const QString & ) ) );

  m_custom_options->removeItemWidget( m_edit_item, m_edit_column );
  m_edit_item->setText( m_edit_column, text );

  m_edit_column = -1;
  m_edit_value = -1;
  m_edit_item = NULL;

  for ( int i = 0; i < m_custom_options->columnCount(); ++i )
  {
    m_custom_options->resizeColumnToContents( i );
  }

  m_custom_options->sortItems( ItemName, Qt::AscendingOrder );

  emit customSettingsModified();
}


void Smb4KSambaOptions::slotCustomIntValueChanged( int value )
{
  // The edit widget will be removed by the eventFilter()
  // function when the user presses Return (or Enter).
  m_edit_value = value;
}


void Smb4KSambaOptions::slotItemSelectionChanged()
{
  m_collection->action( "remove_action" )->setEnabled( !m_custom_options->selectedItems().isEmpty() );
}


void Smb4KSambaOptions::slotCustomContextMenuRequested( const QPoint &pos )
{
  QTreeWidgetItem *item = m_custom_options->itemAt( pos );
  int column            = m_custom_options->columnAt( pos.x() );

  if ( m_edit_item &&
       (m_edit_item != item || m_edit_column != column) )
  {
    // When the user clicks somewhere in the viewport,
    // in 99.9% that means he/she wants to get rid of
    // the edit widget without modifying anything. Thus,
    // we will just remove it here.
    m_custom_options->removeItemWidget( m_edit_item, m_edit_column );

    m_edit_column = -1;
    m_edit_value = -1;
    m_edit_item = NULL;

    for ( int i = 0; i < m_custom_options->columnCount(); ++i )
    {
      m_custom_options->resizeColumnToContents( i );
    }

    m_custom_options->sortItems( ItemName, Qt::AscendingOrder );
  }
  else
  {
    // Do nothing
  }

  if ( item )
  {
    m_edit_item = item;
    m_edit_column = column;

    // Enable/disable the edit action.
    switch ( item->type() )
    {
      case Host:
      {
        switch ( column )
        {
          case Protocol:
          case Kerberos:
          case Port:
          {
            m_collection->action( "edit_action" )->setEnabled( true );

            break;
          }
          default:
          {
            m_collection->action( "edit_action" )->setEnabled( false );

            break;
          }
        }

        break;
      }
      case Share:
      {
        switch ( column )
        {
#ifndef __FreeBSD__
          case WriteAccess:
#endif
          case UID:
          case GID:
          case Port:
          {
            m_collection->action( "edit_action" )->setEnabled( true );

            break;
          }
          default:
          {
            m_collection->action( "edit_action" )->setEnabled( false );

            break;
          }
        }

        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    m_collection->action( "edit_action" )->setEnabled( false );
  }

  m_menu->menu()->popup( m_custom_options->viewport()->mapToGlobal( pos ) );
}


void Smb4KSambaOptions::slotMenuActionTriggered( QAction *action )
{
  if ( QString::compare( action->objectName(), "edit_action" ) == 0 )
  {
    slotEditCustomItem( m_edit_item, m_edit_column );
  }
  else if ( QString::compare( action->objectName(), "remove_action" ) == 0 )
  {
    // Clear the edit item.
    m_edit_item = NULL;

    // Remove the selected items.
    if ( !m_custom_options->selectedItems().isEmpty() )
    {
      while ( !m_custom_options->selectedItems().isEmpty() )
      {
        delete m_custom_options->selectedItems().takeFirst();
      }

      emit customSettingsModified();
    }
    else
    {
      // Do nothing
    }

    // Adjust the rows and columns.
    for ( int i = 0; i < m_custom_options->columnCount(); ++i )
    {
      m_custom_options->resizeColumnToContents( i );
    }

    m_custom_options->sortItems( ItemName, Qt::AscendingOrder );
  }
  else
  {
    // Do nothing
  }
}


#include "smb4ksambaoptions.moc"
