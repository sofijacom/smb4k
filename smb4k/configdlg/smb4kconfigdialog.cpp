/***************************************************************************
    smb4kconfigdialog  -  The configuration dialog of Smb4K
                             -------------------
    begin                : Sa Apr 14 2007
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

// Qt includes
#include <QRadioButton>
#include <QCheckBox>
#include <QTreeWidget>
#include <QShowEvent>
#include <QSize>
#include <QScrollArea>

// KDE includes
#include <klineedit.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kcombobox.h>
#include <kgenericfactory.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kpassworddialog.h>

// system specific includes
#include <unistd.h>
#include <sys/types.h>

// application specific includes
#include <smb4kconfigdialog.h>
#include <smb4kuserinterfaceoptions.h>
#include <smb4knetworkoptions.h>
#include <smb4kshareoptions.h>
#include <smb4kauthoptions.h>
#include <smb4ksuperuseroptions.h>
#include <smb4ksambaoptions.h>
#include <smb4krsyncoptions.h>
#include <smb4klaptopsupportoptions.h>
#include <core/smb4ksettings.h>
#include <core/smb4kglobal.h>
#include <core/smb4ksambaoptionsinfo.h>
#include <core/smb4ksambaoptionshandler.h>
#include <core/smb4kcore.h>
#include <core/smb4kauthinfo.h>
#include <core/smb4kwalletmanager.h>

using namespace Smb4KGlobal;

K_EXPORT_COMPONENT_FACTORY( libsmb4kconfigdialog, KGenericFactory<Smb4KConfigDialog> )


Smb4KConfigDialog::Smb4KConfigDialog( QWidget *parent, const char *name, Smb4KSettings *settings )
: KConfigDialog( parent, name, settings )
{
  setupDialog();
}


Smb4KConfigDialog::Smb4KConfigDialog( QWidget *parent, const QStringList &/*args*/ )
: KConfigDialog( parent, "ConfigDialog", Smb4KSettings::self() )
{
  setupDialog();
}


Smb4KConfigDialog::~Smb4KConfigDialog()
{
}


void Smb4KConfigDialog::setupDialog()
{
  // FIXME: I guess, normally we would not need to close destructively,
  // but at the moment there are issues with the KURLRequester in file
  // mode. To work around those, we are closing the dialog destructively.
  // Maybe we can remove this if we moved to KDE4.
  setAttribute( Qt::WA_DeleteOnClose, true );

  // Add the pages:
  Smb4KUserInterfaceOptions *interface_options = new Smb4KUserInterfaceOptions( this );
  QScrollArea *interface_area = new QScrollArea( this );
  interface_area->setWidget( interface_options );
  interface_area->setWidgetResizable( true );
  interface_area->setFrameStyle( QFrame::NoFrame );

  Smb4KNetworkOptions *network_options = new Smb4KNetworkOptions( this );
  QScrollArea *network_area = new QScrollArea( this );
  network_area->setWidget( network_options );
  network_area->setWidgetResizable( true );
  network_area->setFrameStyle( QFrame::NoFrame );

  Smb4KShareOptions *share_options = new Smb4KShareOptions( this );
  QScrollArea *share_area = new QScrollArea( this );
  share_area->setWidget( share_options );
  share_area->setWidgetResizable( true );
  share_area->setFrameStyle( QFrame::NoFrame );

  Smb4KAuthOptions *auth_options = new Smb4KAuthOptions( this );
  QScrollArea *auth_area = new QScrollArea( this );
  auth_area->setWidget( auth_options );
  auth_area->setWidgetResizable( true );
  auth_area->setFrameStyle( QFrame::NoFrame );

  Smb4KSambaOptions *samba_options = new Smb4KSambaOptions( this );
  QScrollArea *samba_area = new QScrollArea( this );
  samba_area->setWidget( samba_options );
  samba_area->setWidgetResizable( true );
  samba_area->setFrameStyle( QFrame::NoFrame );

  Smb4KRsyncOptions *rsync_options = new Smb4KRsyncOptions( this );
  QScrollArea *rsync_area = new QScrollArea( this );
  rsync_area->setWidget( rsync_options );
  rsync_area->setWidgetResizable( true );
  rsync_area->setFrameStyle( QFrame::NoFrame );

  Smb4KSuperUserOptions *super_user_options = new Smb4KSuperUserOptions( this );
  QScrollArea *super_user_area = new QScrollArea( this );
  super_user_area->setWidget( super_user_options );
  super_user_area->setWidgetResizable( true );
  super_user_area->setFrameStyle( QFrame::NoFrame );

  Smb4KLaptopSupportOptions *laptop_options = new Smb4KLaptopSupportOptions( this );
  QScrollArea *laptop_area = new QScrollArea( this );
  laptop_area->setWidget( laptop_options );
  laptop_area->setWidgetResizable( true );
  laptop_area->setFrameStyle( QFrame::NoFrame );

  // Disable widgets if the respective programs are not installed.
  if ( KStandardDirs::findExe( "rsync" ).isEmpty() )
  {
    rsync_options->setEnabled( false );
  }
  else
  {
    // Do nothing
  }

  if ( KStandardDirs::findExe( "sudo" ).isEmpty() )
  {
    super_user_options->setEnabled( false );
  }
  else
  {
    // Do nothing
  }

  // Now add the pages to the configuration dialog
  m_user_interface  = addPage( interface_area, i18n( "User Interface" ), "view-choose" );
  m_network         = addPage( network_area, i18n( "Network" ), "network-workgroup" );
  m_shares          = addPage( share_area, i18n( "Shares" ), "folder-remote" );
  m_authentication  = addPage( auth_area, i18n( "Authentication" ), "dialog-password" );
  m_samba           = addPage( samba_area, i18n( "Samba" ), "preferences-system-network" );
  m_synchronization = addPage( rsync_area, i18n( "Synchronization" ), "go-bottom" );
  m_super_user      = addPage( super_user_area, i18n( "Super User" ), "user-identity" );
  m_laptop_support  = addPage( laptop_area, i18n( "Laptop Support" ), "computer-laptop" );

  // Stuff that's not managed by KConfig XT is loaded by
  // Smb4KConfigDialog::showEvent()!

  // Connections
  connect( samba_options,      SIGNAL( customSettingsModified() ),
           this,               SLOT( slotEnableApplyButton() ) );

  connect( super_user_options, SIGNAL( removeEntries() ),
           this,               SLOT( slotRemoveSuperUserEntries() ) );
          
  connect( auth_options,       SIGNAL( loadWalletEntries() ),
           this,               SLOT( slotLoadAuthenticationInformation() ) );
           
  connect( auth_options,       SIGNAL( saveWalletEntries() ),
           this,               SLOT( slotSaveAuthenticationInformation() ) );
           
  connect( auth_options,       SIGNAL( setDefaultLogin() ),
           this,               SLOT( slotSetDefaultLogin() ) );
           
  connect( auth_options,       SIGNAL( walletEntriesModified() ),
           this,               SLOT( slotEnableApplyButton() ) );

  setInitialSize( QSize( 800, 600 ) );

  KConfigGroup group( Smb4KSettings::self()->config(), "ConfigDialog" );
  restoreDialogSize( group );
}


void Smb4KConfigDialog::loadCustomSambaOptions()
{
  if ( m_samba )
  {
    QList<Smb4KSambaOptionsInfo *> list = Smb4KSambaOptionsHandler::self()->customOptionsList();
    m_samba->widget()->findChild<Smb4KSambaOptions *>()->insertCustomOptions( list );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::saveCustomSambaOptions()
{
  if ( m_samba )
  {
    QList<Smb4KSambaOptionsInfo> list;
    list = m_samba->widget()->findChild<Smb4KSambaOptions *>()->getCustomOptions();
    
    for ( int i = 0; i < list.size(); ++i )
    {
      Smb4KSambaOptionsHandler::self()->addItem( &list[i], false );
    }
    
    Smb4KSambaOptionsHandler::self()->sync();
  }
  else
  {
    return;
  }
}


void Smb4KConfigDialog::writeSuperUserEntries()
{
  if ( m_super_user )
  {
    if ( m_super_user->widget()->findChild<Smb4KSuperUserOptions *>()->writeSuperUserEntries() )
    {
      setEnabled( false );
      
      // Write to the /etc/sudoers file.      
      if ( !Smb4KSudoWriterInterface::self()->addUser() )
      {
        // The writing failed. Reset the settings in the "Super User"
        // page.
        m_super_user->widget()->findChild<Smb4KSuperUserOptions *>()->resetSuperUserSettings();
        
        // Set super user settings to FALSE
        Smb4KSettings::setUseForceUnmount( false );
        Smb4KSettings::setAlwaysUseSuperUser( false );
      }
      else
      {
        // Set super user setting according to the state of the check boxes
        Smb4KSettings::setUseForceUnmount( 
          m_super_user->widget()->findChild<QCheckBox *>( "kcfg_UseForceUnmount" )->isChecked() 
        );
        
        Smb4KSettings::setAlwaysUseSuperUser( 
          m_super_user->widget()->findChild<QCheckBox *>( "kcfg_AlwaysUseSuperUser" )->isChecked() 
        );
      }
      
      setEnabled( true );
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


bool Smb4KConfigDialog::checkSettings()
{
  bool ok = true;
  QString issues;
  int index = 0;

  // If the user chose "Query custom master browser" in the
  // "Network" tab, there must be a master browser present:
  QRadioButton *query_custom_master = findChild<QRadioButton *>( "kcfg_QueryCustomMaster" );
  KLineEdit *custom_master_input    = findChild<KLineEdit *>( "kcfg_CustomMasterBrowser" );

  if ( query_custom_master && custom_master_input &&
       query_custom_master->isChecked() &&
       custom_master_input->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Network] The custom master browser has not been entered." )+"</li>" );
  }

  // If the user chose "Scan broadcast areas" in the
  // "Network" tab, there must broadcast areas present:
  QRadioButton *scan_bcast_areas    = findChild<QRadioButton *>( "kcfg_ScanBroadcastAreas" );
  KLineEdit *bcast_areas_input      = findChild<KLineEdit *>( "kcfg_BroadcastAreas" );

  if ( scan_bcast_areas && bcast_areas_input &&
       scan_bcast_areas->isChecked() &&
       bcast_areas_input->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Network] The broadcast areas have not been entered." )+"</li>" );
  }

  // The mount prefix must not be empty:
  KUrlRequester *mount_prefix       = findChild<KUrlRequester *>( "kcfg_MountPrefix" );

  if ( mount_prefix && mount_prefix->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Shares] The mount prefix is empty." )+"</li>" );
  }

  // The file mask must not be empty.
  KLineEdit *file_mask              = findChild<KLineEdit *>( "kcfg_FileMask" );

  if ( file_mask && file_mask->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Samba] The file mask is empty." )+"</li>" );
  }

  // The directory mask must not be empty.
  KLineEdit *directory_mask         = findChild<KLineEdit *>( "kcfg_DirectoryMask" );

  if ( directory_mask && directory_mask->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Samba] The directory mask is empty." )+"</li>" );
  }

  // The rsync prefix must not be empty.
  KUrlRequester *rsync_prefix       = findChild<KUrlRequester *>( "kcfg_RsyncPrefix" );

  if ( rsync_prefix && rsync_prefix->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The rsync prefix is empty." )+"</li>" );
  }

  // The path where to store partial files must not be empty.
  QCheckBox *use_partical_directory = findChild<QCheckBox *>( "kcfg_UsePartialDirectory" );
  KUrlRequester *partial_directory  = findChild<KUrlRequester *>( "kcfg_PartialDirectory" );

  if ( use_partical_directory && use_partical_directory->isChecked() &&
       partial_directory && partial_directory->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The directory where partially transferred files should be stored is empty." )+"</li>" );
  }

  // The the exclude patterns must not be empty.
  QCheckBox *use_exclude_pattern    = findChild<QCheckBox *>( "kcfg_UseExcludePattern" );
  KLineEdit *exclude_pattern        = findChild<KLineEdit *>( "kcfg_ExcludePattern" );

  if ( use_exclude_pattern && use_exclude_pattern->isChecked() &&
       exclude_pattern && exclude_pattern->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The exclude patterns have not been entered." )+"</li>" );
  }

  // The the path of the exclude file must not be empty.
  QCheckBox *use_exclude_file       = findChild<QCheckBox *>( "kcfg_UseExcludeFrom" );
  KUrlRequester *exclude_file       = findChild<KUrlRequester *>( "kcfg_ExcludeFrom" );

  if ( use_exclude_file && use_exclude_file->isChecked() &&
       exclude_file && exclude_file->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The path of the exclude file is empty." )+"</li>" );
  }

  // The the include patterns must not be empty.
  QCheckBox *use_include_pattern    = findChild<QCheckBox *>( "kcfg_UseIncludePattern" );
  KLineEdit *include_pattern        = findChild<KLineEdit *>( "kcfg_IncludePattern" );

  if ( use_include_pattern && use_include_pattern->isChecked() &&
       include_pattern && include_pattern->text().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The include patterns have not been entered." )+"</li>" );
  }

  // The the path of the exclude file must not be empty.
  QCheckBox *use_include_file       = findChild<QCheckBox *>( "kcfg_UseIncludeFrom" );
  KUrlRequester *include_file       = findChild<KUrlRequester *>( "kcfg_IncludeFrom" );

  if ( use_include_file && use_include_file->isChecked() &&
       include_file && include_file->url().path().trimmed().isEmpty() )
  {
    ok = false;
    index++;

    issues.append( "<li>"+i18n( "[Synchronization] The path of the include file is empty." )+"</li>" );
  }

  // If you make backups, check that the suffix and that the
  // backup directory is not empty.
  QCheckBox *make_backups           = findChild<QCheckBox *>( "kcfg_MakeBackups" );

  if ( make_backups && make_backups->isChecked() )
  {
    // The backup suffix must not be empty.
    QCheckBox *use_backup_suffix    = findChild<QCheckBox *>( "kcfg_UseBackupSuffix" );
    KLineEdit *backup_suffix        = findChild<KLineEdit *>( "kcfg_BackupSuffix" );

    if ( use_backup_suffix && use_backup_suffix->isChecked() &&
         backup_suffix && backup_suffix->text().trimmed().isEmpty() )
    {
      ok = false;
      index++;

      issues.append( "<li>"+i18n( "[Synchronization] The backup suffix has not been defined." )+"</li>" );
    }

    // The the path for backups must not be empty.
    QCheckBox *use_backup_dir       = findChild<QCheckBox *>( "kcfg_UseBackupDirectory" );
    KUrlRequester *backup_dir       = findChild<KUrlRequester *>( "kcfg_BackupDirectory" );

    if ( use_backup_dir && use_backup_dir->isChecked() &&
         backup_dir && backup_dir->url().path().trimmed().isEmpty() )
    {
      ok = false;
      index++;

      issues.append( "<li>"+i18n( "[Synchronization] The backup directory is empty." )+"</li>" );
    }
  }

  if ( !ok )
  {
    KMessageBox::error( this, i18np( "<qt>The configuration could not be written, because one setting is incomplete:<ul>%2</ul>Please correct this issue.</qt>",
                                     "<qt>The configuration could not be written, because %1 settings are incomplete:<ul>%2</ul>Please correct these issues.</qt>",
                                     index, issues ) );
  }

  return ok;
}


void Smb4KConfigDialog::showEvent( QShowEvent *e )
{
  // Spontaneous show events come from outside the application.
  // We do not want to react on them.
  if ( !e->spontaneous() )
  {
    loadCustomSambaOptions();
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KConfigDialog::slotButtonClicked( int button )
{
  switch( button )
  {
    case Apply:
    {
      // If some settings are not complete, stop here and give
      // the user the opportunity to fill in the needed string(s).
      if ( !checkSettings() )
      {
        return;
      }

      saveCustomSambaOptions();
      writeSuperUserEntries();
      slotSaveAuthenticationInformation();

      break;
    }
    case Ok:
    {
      // If some settings are not complete, stop here and give
      // the user the opportunity to fill in the needed string(s).
      if ( !checkSettings() )
      {
        return;
      }

      saveCustomSambaOptions();
      writeSuperUserEntries();
      slotSaveAuthenticationInformation();

      KConfigGroup group( Smb4KSettings::self()->config(), "ConfigDialog" );
      saveDialogSize( group, KConfigGroup::Normal );

      break;
    }
    default:
    {
      break;
    }
  }

  KConfigDialog::slotButtonClicked( button );
}


// void Smb4KConfigDialog::slotCustomSambaSettingsModified()
// {
//   qDebug() << "Modified";
//   // Get the list view and all other input widgets:
//   QTreeWidget *view = findChild<QTreeWidget *>( "CustomOptionsList" );
//   bool enable_apply = false;
// 
//   if ( view )
//   {
//     // Get the old list of custom options from the options handler.
//     QList<Smb4KSambaOptionsInfo *> old_list = Smb4KSambaOptionsHandler::self()->customOptionsList();
// 
//     // Get the new list of custom options from the Samba options tab.
//     QList<Smb4KSambaOptionsInfo *> new_list = static_cast<Smb4KSambaOptions *>( findChild<Smb4KSambaOptions *>() )->getCustomOptions();
// 
//     if ( old_list.size() == new_list.size() )
//     {
//       for ( int i = 0; i < old_list.size(); ++i )
//       {
//         for ( int j = 0; j < new_list.size(); ++j )
//         {
//           if ( QString::compare( old_list.at( i )->unc(), new_list.at( j )->unc(),
//                Qt::CaseInsensitive ) == 0 )
//           {
//             if ( old_list.at( i )->protocol() != new_list.at( j )->protocol() )
//             {
//               enable_apply = true;
// 
//               break;
//             }
//             else
//             {
//               // Do nothing
//             }
// 
// #ifndef Q_OS_FREEBSD
//             if ( old_list.at( i )->writeAccess() != new_list.at( j )->writeAccess() )
//             {
//               enable_apply = true;
// 
//               break;
//             }
//             else
//             {
//               // Do nothing
//             }
// #endif
// 
//             if ( old_list.at( i )->useKerberos() != new_list.at( j )->useKerberos() )
//             {
//               enable_apply = true;
// 
//               break;
//             }
//             else
//             {
//               // Do nothing
//             }
// 
//             if ( old_list.at( i )->uid() != new_list.at( j )->uid() )
//             {
//               enable_apply = true;
// 
//               break;
//             }
//             else
//             {
//               // Do nothing
//             }
// 
//             if ( old_list.at( i )->gid() != new_list.at( j )->gid() )
//             {
//               enable_apply = true;
// 
//               break;
//             }
//             else
//             {
//               // Do nothing
//             }
// 
//             if ( old_list.at( i )->port() != new_list.at( j )->port() )
//             {
//               enable_apply = true;
// 
//               break;
//             }
//             else
//             {
//               // Do nothing
//             }
// 
//             break;
//           }
//           else
//           {
//             continue;
//           }
//         }
// 
//         if ( enable_apply )
//         {
//           break;
//         }
//         else
//         {
//           continue;
//         }
//       }
//     }
//     else
//     {
//       enable_apply = true;
//     }
//   }
//   else
//   {
//     // Do nothing
//   }
// 
//   enableButtonApply( enable_apply );
// }


void Smb4KConfigDialog::slotRemoveSuperUserEntries()
{
  setEnabled( false );

  // Remove the entries.
  if ( !Smb4KSudoWriterInterface::self()->removeUser() )
  {
    // Actually, we do not need to care about failed removals.
  }
  else
  {
    // Do nothing
  }

  setEnabled( true );
}


void Smb4KConfigDialog::slotLoadAuthenticationInformation()
{
  Smb4KAuthOptions *auth_options = m_authentication->widget()->findChild<Smb4KAuthOptions *>();
  Smb4KWalletManager::self()->init( this );
  QList<Smb4KAuthInfo *> entries = Smb4KWalletManager::self()->walletEntries();
  auth_options->insertWalletEntries( entries );
  auth_options->displayWalletEntries();
}


void Smb4KConfigDialog::slotSaveAuthenticationInformation()
{
  Smb4KAuthOptions *auth_options = m_authentication->widget()->findChild<Smb4KAuthOptions *>();
  
  if ( auth_options->walletEntriesDisplayed() )
  {
    Smb4KWalletManager::self()->init( this );
    QList<Smb4KAuthInfo *> entries = auth_options->getWalletEntries();
    Smb4KWalletManager::self()->writeWalletEntries( entries );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::slotSetDefaultLogin()
{
  Smb4KAuthOptions *auth_options = m_authentication->widget()->findChild<Smb4KAuthOptions *>();
  
  if ( !auth_options->undoRemoval() )
  {
    Smb4KWalletManager::self()->init( this );
  
    Smb4KAuthInfo authInfo;
    authInfo.setDefaultAuthInfo();
    
    Smb4KWalletManager::self()->readAuthInfo( &authInfo );
    
    KPasswordDialog dlg( this, KPasswordDialog::ShowUsernameLine );
    dlg.setPrompt( i18n( "Enter the default login information." ) );
    dlg.setUsername( authInfo.login() );
    dlg.setPassword( authInfo.password() );
    
    if ( dlg.exec() == KPasswordDialog::Accepted )
    {
      authInfo.setLogin( dlg.username() );
      authInfo.setPassword( dlg.password() );
      
      Smb4KWalletManager::self()->writeAuthInfo( &authInfo );
      
      if ( auth_options->walletEntriesDisplayed() )
      {
        slotLoadAuthenticationInformation();
      }
      else
      {
        // Do nothing
      }    
    }
    else
    {
      // Do nothing. The user canceled.
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KConfigDialog::slotEnableApplyButton()
{
  // Check if we need to enable the Apply button.
  bool enable = false;
  
  // Check the wallet entries.
  Smb4KAuthOptions *auth_options = m_authentication->widget()->findChild<Smb4KAuthOptions *>();

  if ( auth_options->walletEntriesMaybeChanged() )
  {
    QList<Smb4KAuthInfo *> old_wallet_entries = Smb4KWalletManager::self()->walletEntries();
    QList<Smb4KAuthInfo *> new_wallet_entries = auth_options->getWalletEntries();
    
    for ( int i = 0; i < old_wallet_entries.size(); ++i )
    {
      for ( int j = 0; j < new_wallet_entries.size(); ++j )
      {
        if ( QString::compare( old_wallet_entries.at( i )->unc(),
                               new_wallet_entries.at( j )->unc(),
                               Qt::CaseInsensitive ) == 0 &&
             (QString::compare( old_wallet_entries.at( i )->workgroupName(),
                                new_wallet_entries.at( j )->workgroupName(),
                                Qt::CaseInsensitive ) != 0 ||
              QString::compare( old_wallet_entries.at( i )->login(),
                                new_wallet_entries.at( j )->login(),
                                Qt::CaseInsensitive ) != 0 ||
              QString::compare( old_wallet_entries.at( i )->password(),
                                new_wallet_entries.at( j )->password(),
                                Qt::CaseInsensitive ) != 0) )
        {
          enable = true;
          break;
        }
        else
        {
          continue;
        }
      }
      
      if ( enable )
      {
        break;
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  // Check the custom settings.
  Smb4KSambaOptions *samba_options = m_samba->widget()->findChild<Smb4KSambaOptions *>();
  
  if ( !enable && samba_options->customSettingsMaybeChanged() )
  {
    QList<Smb4KSambaOptionsInfo> new_list = samba_options->getCustomOptions();
    QList<Smb4KSambaOptionsInfo *> old_list = Smb4KSambaOptionsHandler::self()->customOptionsList();
    
    if ( new_list.size() == old_list.size() )
    {
      for ( int i = 0; i < new_list.size(); ++i )
      {
        for ( int j = 0; j < old_list.size(); ++j )
        {
          if ( !new_list[i].equals( old_list.at( j ) ) )
          {
            enable = true;
            break;
          }
          else
          {
            continue;
          }
        }
        
        if ( enable )
        {
          break;
        }
        else
        {
          continue;
        }
      }
    }
    else
    {
      enable = true;
    }
  }
  else
  {
    // Do nothing
  }
  
  enableButtonApply( enable );
}

#include "smb4kconfigdialog.moc"
