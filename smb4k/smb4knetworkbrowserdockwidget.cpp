/***************************************************************************
    The network neighborhood browser dock widget 
                             -------------------
    begin                : Sat Apr 28 2018
    copyright            : (C) 2018 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4knetworkbrowserdockwidget.h"
#include "smb4knetworkbrowseritem.h"
#include "core/smb4kscanner.h"
#include "core/smb4kmounter.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4ksettings.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kpreviewer.h"
#include "core/smb4kprint.h"

// Qt includes
#include <QApplication>
#include <QMenu>
#include <QHeaderView>

// KDE includes
#include <KWidgetsAddons/KDualAction>
#include <KWidgetsAddons/KGuiItem>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;


Smb4KNetworkBrowserDockWidget::Smb4KNetworkBrowserDockWidget(const QString& title, QWidget* parent)
: QDockWidget(title, parent)
{
  //
  // Set the network browser widget
  // 
  m_networkBrowser = new Smb4KNetworkBrowser(this);
  setWidget(m_networkBrowser);
  
  //
  // The action collection
  // 
  m_actionCollection = new KActionCollection(this);
  
  //
  // The context menu
  // 
  m_contextMenu = new KActionMenu(this);
  
  //
  // Set up the actions
  //
  setupActions();

  //
  // Load the settings
  // 
  loadSettings();
  
  //
  // Connections
  // 
  connect(m_networkBrowser, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenuRequested(QPoint)));
  connect(m_networkBrowser, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));
  connect(m_networkBrowser, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));
          
  connect(Smb4KScanner::self(), SIGNAL(authError(HostPtr,int)), this, SLOT(slotAuthError(HostPtr,int)));
  connect(Smb4KScanner::self(), SIGNAL(workgroups()), this, SLOT(slotWorkgroups()));
  connect(Smb4KScanner::self(), SIGNAL(hosts(WorkgroupPtr)), this, SLOT(slotWorkgroupMembers(WorkgroupPtr)));
  connect(Smb4KScanner::self(), SIGNAL(shares(HostPtr)), this, SLOT(slotShares(HostPtr)));
  connect(Smb4KScanner::self(), SIGNAL(ipAddress(HostPtr)), this, SLOT(slotAddIPAddress(HostPtr)));
  connect(Smb4KScanner::self(), SIGNAL(aboutToStart(NetworkItemPtr,int)), this, SLOT(slotScannerAboutToStart(NetworkItemPtr,int)));
  connect(Smb4KScanner::self(), SIGNAL(finished(NetworkItemPtr, int)), this, SLOT(slotScannerFinished(NetworkItemPtr,int)));
  
  connect(Smb4KMounter::self(), SIGNAL(mounted(SharePtr)), this, SLOT(slotShareMounted(SharePtr)));
  connect(Smb4KMounter::self(), SIGNAL(unmounted(SharePtr)), this, SLOT(slotShareUnmounted(SharePtr)));
  connect(Smb4KMounter::self(), SIGNAL(aboutToStart(int)), this, SLOT(slotMounterAboutToStart(int)));
  connect(Smb4KMounter::self(), SIGNAL(finished(int)), this, SLOT(slotMounterFinished(int)));
  
  connect(KIconLoader::global(), SIGNAL(iconChanged(int)), this, SLOT(slotIconSizeChanged(int)));
}


Smb4KNetworkBrowserDockWidget::~Smb4KNetworkBrowserDockWidget()
{
}


void Smb4KNetworkBrowserDockWidget::setupActions()
{
  //
  // Rescan and abort dual action
  // 
  KDualAction *rescanAbortAction = new KDualAction(this);
  KGuiItem rescanItem(i18n("Scan Netwo&rk"), KDE::icon("view-refresh"));
  KGuiItem abortItem(i18n("&Abort"), KDE::icon("process-stop"));
  rescanAbortAction->setActiveGuiItem(rescanItem);
  rescanAbortAction->setInactiveGuiItem(abortItem);
  rescanAbortAction->setActive(true);
  rescanAbortAction->setAutoToggle(false);
  connect(rescanAbortAction, SIGNAL(triggered(bool)), this, SLOT(slotRescanAbortActionTriggered(bool)));
  
  //
  // Bookmark action
  // 
  QAction *bookmarkAction = new QAction(KDE::icon("bookmark-new"), i18n("Add &Bookmark"), this);
  connect(bookmarkAction, SIGNAL(triggered(bool)), this, SLOT(slotAddBookmark(bool)));
  
  //
  // Mount dialog action
  // 
  QAction *manualAction = new QAction(KDE::icon("view-form", QStringList("emblem-mounted")), i18n("&Open Mount Dialog"), this);
  connect(manualAction, SIGNAL(triggered(bool)), this, SLOT(slotMountManually(bool)));

  //
  // Authentication action
  // 
  QAction *authAction = new QAction(KDE::icon("dialog-password"), i18n("Au&thentication"), this);
  connect(authAction, SIGNAL(triggered(bool)), this, SLOT(slotAuthentication(bool)));

  //
  // Custom options action
  // 
  QAction *customAction = new QAction(KDE::icon("preferences-system-network"), i18n("&Custom Options"), this);
  connect(customAction, SIGNAL(triggered(bool)), this, SLOT(slotCustomOptions(bool)));

  //
  // Preview action
  // 
  QAction *previewAction  = new QAction(KDE::icon("view-list-icons"), i18n("Pre&view"), this);
  connect(previewAction, SIGNAL(triggered(bool)), this, SLOT(slotPreview(bool)));

  //
  // Print action
  // 
  QAction *printAction = new QAction(KDE::icon("printer"), i18n("&Print File"), this);
  connect(printAction, SIGNAL(triggered(bool)), this, SLOT(slotPrint(bool)));

  //
  // Mount/unmount action
  // 
  KDualAction *mountAction = new KDualAction(this);
  KGuiItem mountItem(i18n("&Mount"), KDE::icon("media-mount"));
  KGuiItem unmountItem(i18n("&Unmount"), KDE::icon("media-eject"));
  mountAction->setActiveGuiItem(mountItem);
  mountAction->setInactiveGuiItem(unmountItem);
  mountAction->setActive(true);
  mountAction->setAutoToggle(false);
  connect(mountAction, SIGNAL(triggered(bool)), this, SLOT(slotMountActionTriggered(bool)));
  connect(mountAction, SIGNAL(activeChanged(bool)), this, SLOT(slotMountActionChanged(bool)));
  
  //
  // Add the actions
  //
  m_actionCollection->addAction("rescan_abort_action", rescanAbortAction);
  m_actionCollection->addAction("bookmark_action", bookmarkAction);
  m_actionCollection->addAction("mount_manually_action", manualAction);
  m_actionCollection->addAction("authentication_action", authAction);
  m_actionCollection->addAction("custom_action", customAction);
  m_actionCollection->addAction("preview_action", previewAction);
  m_actionCollection->addAction("print_action", printAction);
  m_actionCollection->addAction("mount_action", mountAction);
  
  //
  // Set the shortcuts
  //
  m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Refresh);
  m_actionCollection->setDefaultShortcut(bookmarkAction, QKeySequence(Qt::CTRL+Qt::Key_B));
  m_actionCollection->setDefaultShortcut(manualAction, QKeySequence(Qt::CTRL+Qt::Key_O));
  m_actionCollection->setDefaultShortcut(authAction, QKeySequence(Qt::CTRL+Qt::Key_T));
  m_actionCollection->setDefaultShortcut(customAction, QKeySequence(Qt::CTRL+Qt::Key_C));
  m_actionCollection->setDefaultShortcut(previewAction, QKeySequence(Qt::CTRL+Qt::Key_V));
  m_actionCollection->setDefaultShortcut(printAction, QKeySequence(Qt::CTRL+Qt::Key_P));
  m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL+Qt::Key_M));
  
  //
  // Enable/disable the actions
  //
  rescanAbortAction->setEnabled(true);
  bookmarkAction->setEnabled(false);
  manualAction->setEnabled(true);
  authAction->setEnabled(false);
  customAction->setEnabled(false);
  previewAction->setEnabled(false);
  printAction->setEnabled(false);
  mountAction->setEnabled(false);

  // 
  // Plug the actions into the context menu
  //
  m_contextMenu->addAction(rescanAbortAction);
  m_contextMenu->addSeparator();
  m_contextMenu->addAction(bookmarkAction);
  m_contextMenu->addAction(manualAction);
  m_contextMenu->addSeparator();
  m_contextMenu->addAction(authAction);
  m_contextMenu->addAction(customAction);
  m_contextMenu->addAction(previewAction);
  m_contextMenu->addAction(printAction);
  m_contextMenu->addAction(mountAction);
}


void Smb4KNetworkBrowserDockWidget::loadSettings()
{
  //
  // Load icon size
  // 
  int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
  m_networkBrowser->setIconSize(QSize(iconSize, iconSize));
  
  // 
  // Show/hide columns
  // 
  m_networkBrowser->setColumnHidden(Smb4KNetworkBrowser::IP, !Smb4KSettings::showIPAddress());
  m_networkBrowser->setColumnHidden(Smb4KNetworkBrowser::Type, !Smb4KSettings::showType());
  m_networkBrowser->setColumnHidden(Smb4KNetworkBrowser::Comment, !Smb4KSettings::showComment());

  // 
  // Load and apply the positions of the columns
  // 
  KConfigGroup configGroup(Smb4KSettings::self()->config(), "NetworkBrowserPart");

  QMap<int, int> map;
  map.insert(configGroup.readEntry("ColumnPositionNetwork", (int)Smb4KNetworkBrowser::Network), Smb4KNetworkBrowser::Network);
  map.insert(configGroup.readEntry("ColumnPositionType", (int)Smb4KNetworkBrowser::Type), Smb4KNetworkBrowser::Type);
  map.insert(configGroup.readEntry("ColumnPositionIP", (int)Smb4KNetworkBrowser::IP), Smb4KNetworkBrowser::IP);
  map.insert(configGroup.readEntry("ColumnPositionComment", (int)Smb4KNetworkBrowser::Comment), Smb4KNetworkBrowser::Comment);

  QMap<int, int>::const_iterator it = map.constBegin();

  while (it != map.constEnd())
  {
    if (it.key() != m_networkBrowser->header()->visualIndex(it.value()))
    {
      m_networkBrowser->header()->moveSection(m_networkBrowser->header()->visualIndex(it.value()), it.key());
    }
    else
    {
      // Do nothing
    }

    ++it;
  }

  // 
  // Does anything has to be changed with the marked shares?
  // 
  for (const SharePtr &share : mountedSharesList())
  {
    // We do not need to use slotShareUnmounted() here, too,
    // because slotShareMounted() will take care of everything
    // we need here.
    slotShareMounted(share);
  }
}


void Smb4KNetworkBrowserDockWidget::saveSettings()
{
  // 
  // Save the position of the columns
  // 
  KConfigGroup configGroup(Smb4KSettings::self()->config(), "NetworkBrowserPart");
  configGroup.writeEntry("ColumnPositionNetwork", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Network));
  configGroup.writeEntry("ColumnPositionType", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Type));
  configGroup.writeEntry("ColumnPositionIP", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::IP));
  configGroup.writeEntry("ColumnPositionComment", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Comment));

  configGroup.sync();
}


KActionCollection *Smb4KNetworkBrowserDockWidget::actionCollection()
{
  return m_actionCollection;
}


void Smb4KNetworkBrowserDockWidget::slotContextMenuRequested(const QPoint& pos)
{
  m_contextMenu->menu()->popup(m_networkBrowser->viewport()->mapToGlobal(pos));
}


void Smb4KNetworkBrowserDockWidget::slotItemActivated(QTreeWidgetItem* item, int /*column*/)
{
  //
  // Process the activated item
  // 
  if (QApplication::keyboardModifiers() == Qt::NoModifier && m_networkBrowser->selectedItems().size() == 1)
  {
    Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(item);

    if (browserItem)
    {
      switch (browserItem->type())
      {
        case Workgroup:
        {
          if (browserItem->isExpanded())
          {
            Smb4KScanner::self()->lookupDomainMembers(browserItem->workgroupItem(), m_networkBrowser);
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Host:
        {
          if (browserItem->isExpanded())
          {
            Smb4KScanner::self()->lookupShares(browserItem->hostItem(), m_networkBrowser);
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Share:
        {
          if (!browserItem->shareItem()->isPrinter())
          {
            slotMountActionTriggered(false);  // boolean is ignored
          }
          else
          {
            slotPrint(false);  // boolean is ignored
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
      // Do nothing
    }    
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotItemSelectionChanged()
{
  //
  // Get the selected item
  // 
  QList<QTreeWidgetItem *> items = m_networkBrowser->selectedItems();

  //
  // Enable/disable and/or adjust the actions depending of the number 
  // of selected items and their type
  //   
  if (items.size() == 1)
  {
    Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(items.first());

    if (browserItem)
    {
      switch (browserItem->type())
      {
        case Host:
        {
          //
          // Adjust the actions
          // 
          KGuiItem rescanItem(i18n("Scan Compute&r"), KDE::icon("view-refresh"));
          static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setActiveGuiItem(rescanItem);
          m_actionCollection->action("bookmark_action")->setEnabled(false);
          m_actionCollection->action("authentication_action")->setEnabled(true);
          m_actionCollection->action("custom_action")->setEnabled(true);
          m_actionCollection->action("preview_action")->setEnabled(false);
          m_actionCollection->action("print_action")->setEnabled(false);
          static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
          m_actionCollection->action("mount_action")->setEnabled(false);
          break;
        }
        case Share:
        {
          //
          // Adjust the actions
          // 
          KGuiItem rescanItem(i18n("Scan Compute&r"), KDE::icon("view-refresh"));
          static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setActiveGuiItem(rescanItem);
          m_actionCollection->action("bookmark_action")->setEnabled(!browserItem->shareItem()->isPrinter());
          m_actionCollection->action("authentication_action")->setEnabled(true);
          m_actionCollection->action("custom_action")->setEnabled(!browserItem->shareItem()->isPrinter());
          m_actionCollection->action("preview_action")->setEnabled(!browserItem->shareItem()->isPrinter());
          m_actionCollection->action("print_action")->setEnabled(browserItem->shareItem()->isPrinter());

          if (!browserItem->shareItem()->isPrinter())
          {
            if (!browserItem->shareItem()->isMounted() || (browserItem->shareItem()->isMounted() && browserItem->shareItem()->isForeign()))
            {
              static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
              m_actionCollection->action("mount_action")->setEnabled(true);
            }
            else if (browserItem->shareItem()->isMounted() && !browserItem->shareItem()->isForeign())
            {
              static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(false);
              m_actionCollection->action("mount_action")->setEnabled(true);
            }
            else
            {
              static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
              m_actionCollection->action("mount_action")->setEnabled(false);
            }
          }
          else
          {
            static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
            m_actionCollection->action("mount_action")->setEnabled(true);
          }
          break;
        }
        default:
        {
          //
          // Adjust the actions
          // 
          KGuiItem rescanItem(i18n("Scan Wo&rkgroup"), KDE::icon("view-refresh"));
          static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setActiveGuiItem(rescanItem);
          m_actionCollection->action("bookmark_action")->setEnabled(false);
          m_actionCollection->action("authentication_action")->setEnabled(false);
          m_actionCollection->action("custom_action")->setEnabled(false);
          m_actionCollection->action("preview_action")->setEnabled(false);
          m_actionCollection->action("print_action")->setEnabled(false);
          static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
          m_actionCollection->action("mount_action")->setEnabled(false);          
          break;
        }
      }
    }
    else
    {
      // Do nothing. This is managed elsewhere.
    }
  }
  else if (items.size() > 1)
  {
    //
    // In this case there are only shares selected, because all other items
    // are automatically deselected in extended selection mode.
    //
    // For deciding which function the mount action should have, we use
    // the number of unmounted shares. If that is identical with the items.size(),
    // it will mount the items, otherwise it will unmount them.
    //
    int unmountedShares = items.size();

    for (QTreeWidgetItem *item : items)
    {
      Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(item);
      
      if (browserItem && browserItem->shareItem()->isMounted() && !browserItem->shareItem()->isForeign())
      {
        //
        // Substract shares mounted by the user
        // 
        unmountedShares--;
      }
      else
      {
        // Do nothing
      }
    }
    
    //
    // Adjust the actions
    //
    KGuiItem rescanItem(i18n("Scan Netwo&rk"), KDE::icon("view-refresh"));
    static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setActiveGuiItem(rescanItem);
    m_actionCollection->action("bookmark_action")->setEnabled(true);
    m_actionCollection->action("authentication_action")->setEnabled(false);
    m_actionCollection->action("custom_action")->setEnabled(false);
    m_actionCollection->action("preview_action")->setEnabled(true);
    m_actionCollection->action("print_action")->setEnabled(false);
    static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(unmountedShares == items.size());
    m_actionCollection->action("mount_action")->setEnabled(true);      
  }
  else
  {
    //
    // Adjust the actions
    // 
    KGuiItem rescanItem(i18n("Scan Netwo&rk"), KDE::icon("view-refresh"));
    static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setActiveGuiItem(rescanItem);
    m_actionCollection->action("bookmark_action")->setEnabled(false);
    m_actionCollection->action("authentication_action")->setEnabled(false);
    m_actionCollection->action("custom_action")->setEnabled(false);
    m_actionCollection->action("preview_action")->setEnabled(false);
    m_actionCollection->action("print_action")->setEnabled(false);
    static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
    m_actionCollection->action("mount_action")->setEnabled(false);        
  }
}


void Smb4KNetworkBrowserDockWidget::slotAuthError(const HostPtr& host, int process)
{
  switch (process)
  {
    case LookupDomains:
    {
      //
      // We queried a master browser for the list of domains and
      // workgroup. So, we can clear the whole list of domains.
      // 
      while (m_networkBrowser->topLevelItemCount() != 0)
      {
        delete m_networkBrowser->takeTopLevelItem(0);
      }
      break;
    }
    case LookupDomainMembers:
    {
      //
      // Get the workgroup and clear the list of domain members. 
      // Then, reinsert the master browser.
      //
      if (m_networkBrowser->topLevelItemCount() != 0)
      {
        for (int i = 0; i < m_networkBrowser->topLevelItemCount(); ++i)
        {
          Smb4KNetworkBrowserItem *workgroup = static_cast<Smb4KNetworkBrowserItem *>(m_networkBrowser->topLevelItem(i));
          
          if (workgroup && workgroup->type() == Workgroup && 
              QString::compare(host->workgroupName(), workgroup->workgroupItem()->workgroupName(), Qt::CaseInsensitive) == 0)
          {
            QList<QTreeWidgetItem *> hosts = workgroup->takeChildren();
            
            while (!hosts.isEmpty())
            {
              delete hosts.takeFirst();
            }
            break;
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
      break;
    }
    case LookupShares:
    {
      //
      // Get the host that could not be accessed.
      // 
      QTreeWidgetItemIterator it(m_networkBrowser);
      
      while (*it)
      {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);
        
        if (item && item->type() == Host)
        {
          if (QString::compare(host->hostName(), item->hostItem()->hostName(), Qt::CaseInsensitive) == 0 &&
              QString::compare(host->workgroupName(), item->hostItem()->workgroupName(), Qt::CaseInsensitive) == 0)
          {
            QList<QTreeWidgetItem *> shares = item->takeChildren();
            
            while (!shares.isEmpty())
            {
              delete shares.takeFirst();
            }
            break;
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
        ++it;
      }
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkBrowserDockWidget::slotWorkgroups()
{
  //
  // Process the global workgroup list
  //
  if (!workgroupsList().isEmpty())
  {
    //
    // Remove obsolete workgroups and update existing ones
    //    
    QTreeWidgetItemIterator itemIt(m_networkBrowser, QTreeWidgetItemIterator::All);
      
    while (*itemIt)
    {
      Smb4KNetworkBrowserItem *networkItem = static_cast<Smb4KNetworkBrowserItem *>(*itemIt);
        
      if (networkItem->type() == Workgroup)
      {
        WorkgroupPtr workgroup = findWorkgroup(networkItem->workgroupItem()->workgroupName());
          
        if (workgroup)
        {
          networkItem->update();
            
          // Update the master browser
          for (int i = 0; i < networkItem->childCount(); ++i)
          {
            Smb4KNetworkBrowserItem *host = static_cast<Smb4KNetworkBrowserItem *>(networkItem->child(i));
            host->update();
          }
        }
        else
        {
          delete networkItem;
        }
      }
      else
      {
        // Do nothing
      }
          
      ++itemIt;
    }
      
    //
    // Add new workgroups to the tree widget
    //
    for (const WorkgroupPtr &workgroup : workgroupsList())
    {
      QList<QTreeWidgetItem *> items = m_networkBrowser->findItems(workgroup->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network);
        
      if (items.isEmpty())
      {
        (void) new Smb4KNetworkBrowserItem(m_networkBrowser, workgroup);
      }
      else
      {
        // Do nothing
      }
    }
    
    //
    // Sort the items
    //
    m_networkBrowser->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);
  }
  else
  {
    //
    // Clear the tree widget
    //
    m_networkBrowser->clear();
  }
}


void Smb4KNetworkBrowserDockWidget::slotWorkgroupMembers(const WorkgroupPtr& workgroup)
{
  //
  // Process the list of domain members
  //
  if (workgroup)
  {
    //
    // Find the workgroup(s) 
    //
    QList<QTreeWidgetItem *> workgroups = m_networkBrowser->findItems(workgroup->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network);
    QMutableListIterator<QTreeWidgetItem *> it(workgroups);
    
    while (it.hasNext())
    {
      QTreeWidgetItem *item = it.next();
      
      if (item->type() == Workgroup)
      {
        Smb4KNetworkBrowserItem *workgroupItem = static_cast<Smb4KNetworkBrowserItem *>(item);
        QTreeWidgetItemIterator itemIt(workgroupItem);
        
        //
        // Remove obsolete hosts and update existing ones
        //
        while (*itemIt)
        {
          Smb4KNetworkBrowserItem *networkItem = static_cast<Smb4KNetworkBrowserItem *>(*itemIt);
          
          if (networkItem->type() == Host)
          {
            HostPtr host = findHost(networkItem->hostItem()->hostName(), networkItem->hostItem()->workgroupName());
            
            if (host)
            {
              networkItem->update();
            }
            else
            {
              delete networkItem;
            }
          }
          else
          {
            break;
          }
          
          ++itemIt;
        }
        
        //
        // Add new hosts to the workgroup item and remove obsolete workgroups if
        // necessary. Honor the auto-expand feature.
        //
        QList<HostPtr> members = workgroupMembers(workgroupItem->workgroupItem());
        
        if (!members.isEmpty())
        {
          for (const HostPtr &host : members)
          {
            bool foundHost = false;
            
            for (int i = 0; i < workgroupItem->childCount(); ++i)
            {
              Smb4KNetworkBrowserItem *hostItem = static_cast<Smb4KNetworkBrowserItem *>(workgroupItem->child(i));
              
              if (hostItem->hostItem()->hostName() == host->hostName())
              {
                foundHost = true;
                break;
              }
              else
              {
                continue;
              }
            }
            
            if (!foundHost)
            {
              (void) new Smb4KNetworkBrowserItem(workgroupItem, host);
            }
            else
            {
              // Do nothing
            }
          }
          
          // Auto-expand the workgroup item, if applicable
          if (Smb4KSettings::autoExpandNetworkItems() && !workgroupItem->isExpanded())
          {
            m_networkBrowser->expandItem(workgroupItem);
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Delete all hosts of the workgroup (if there should still be some) and
          // remove the workgroup item from the view (no hosts => no workgroup)
          while (workgroupItem->childCount() != 0)
          {
            delete workgroupItem->takeChild(0);
          }
          
          delete workgroupItem;
        }
      }
      else
      {
        // Do nothing
      }
    }
    
    //
    // Sort the items
    //
    m_networkBrowser->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);
  }
  else
  {
    // Do nothing
  }  
}


void Smb4KNetworkBrowserDockWidget::slotShares(const HostPtr& host)
{
  //
  // Process the list of shares
  // 
  if (host)
  {
    //
    // Find the host(s)
    //
    QList<QTreeWidgetItem *> hosts = m_networkBrowser->findItems(host->hostName(), Qt::MatchFixedString|Qt::MatchRecursive, Smb4KNetworkBrowser::Network);
    QMutableListIterator<QTreeWidgetItem *> it(hosts);
    
    while (it.hasNext())
    {
      Smb4KNetworkBrowserItem *hostItem = static_cast<Smb4KNetworkBrowserItem *>(it.next());
      
      if (hostItem->type() == Host && hostItem->hostItem()->workgroupName() == host->workgroupName())
      {
        QTreeWidgetItemIterator itemIt(hostItem);
        
        //
        // Remove obsolete shares and update existing ones
        //
        while (*itemIt)
        {
          Smb4KNetworkBrowserItem *shareItem = static_cast<Smb4KNetworkBrowserItem *>(*itemIt);
          
          if (shareItem->type() == Share)
          {
            SharePtr share = findShare(shareItem->shareItem()->unc(), shareItem->shareItem()->workgroupName());
            
            if (share)
            {
              shareItem->update();
            }
            else
            {
              delete shareItem;
            }
          }
          else
          {
            break;
          }
          
          ++itemIt;
        }
        
        //
        // Add new shares to the host item. The host will not be removed from the
        // view when it has no shares. Honor the auto-expand feature.
        //
        QList<SharePtr> shares = sharedResources(host);
        
        if (!shares.isEmpty())
        {
          for (const SharePtr &share : shares)
          {
            bool foundShare = false;
            
            for (int i = 0; i < hostItem->childCount(); ++i)
            {
              Smb4KNetworkBrowserItem *shareItem = static_cast<Smb4KNetworkBrowserItem *>(hostItem->child(i));
              
              if (shareItem->shareItem()->unc() == share->unc())
              {
                foundShare = true;
                break;
              }
              else
              {
                continue;
              }
            }
            
            if (!foundShare)
            {
              (void) new Smb4KNetworkBrowserItem(hostItem, share);
            }
            else
            {
              // Do nothing
            }
          }
          
          // Auto-expand the host item, if applicable
          if (Smb4KSettings::autoExpandNetworkItems() && !hostItem->isExpanded())
          {
            m_networkBrowser->expandItem(hostItem);
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Delete all shares (if there should still be some), but leave the 
          // host in the view.
          while (hostItem->childCount() != 0)
          {
            delete hostItem->takeChild(0);
          }
        }
      }
      else
      {
        continue;
      }
    }
    
    //
    // Sort the items
    //
    m_networkBrowser->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotAddIPAddress(const HostPtr& host)
{
  //
  // Process the IP address
  //
  if (host)
  {
    //
    // Find the host and update it. Also set the IP address of the master 
    // browser
    //
    QTreeWidgetItemIterator itemIt(m_networkBrowser);
    
    while (*itemIt)
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*itemIt);
      
      if (item->type() == Host && item->hostItem()->hostName() == host->hostName() 
          && item->hostItem()->workgroupName() == host->workgroupName())
      {
        // Update the host
        item->update();
        
        // Update the shares
        for (int i = 0; i < item->childCount(); ++i)
        {
          Smb4KNetworkBrowserItem *shareItem = static_cast<Smb4KNetworkBrowserItem *>(item->child(i));
          shareItem->update();
        }
        
        // Update the workgroup master browser's IP address
        Smb4KNetworkBrowserItem *workgroupItem = static_cast<Smb4KNetworkBrowserItem *>(item->parent());
        
        if (workgroupItem && host->isMasterBrowser())
        {
          workgroupItem->update();
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
      
      ++itemIt;
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotRescanAbortActionTriggered(bool /*checked*/)
{
  //
  // Get the Rescan/Abort action
  // 
  KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"));
  
  
  //
  // Get the selected items
  // 
  QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();
  
  //
  // Perform actions according to the state of the action and the number of 
  // selected items.
  //
  if (rescanAbortAction->isActive())
  {
    if (selectedItems.size() == 1)
    {
      Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(selectedItems.first());
      
      if (browserItem)
      {
        switch (browserItem->type())
        {
          case Workgroup:
          {
            Smb4KScanner::self()->lookupDomainMembers(browserItem->workgroupItem(), m_networkBrowser);
            break;
          }
          case Host:
          {
            Smb4KScanner::self()->lookupShares(browserItem->hostItem(), m_networkBrowser);
            break;
          }
          case Share:
          {
            Smb4KNetworkBrowserItem *parentItem = static_cast<Smb4KNetworkBrowserItem *>(browserItem->parent());
            Smb4KScanner::self()->lookupShares(parentItem->hostItem(), m_networkBrowser);
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
        // Do nothing
      }
    }
    else
    {
      // If several items are selected or no selected items,
      // only the network can be scanned.
      Smb4KScanner::self()->lookupDomains(m_networkBrowser);
    }
  }
  else
  {
    // Stop all actions performed by the scanner
    if (Smb4KScanner::self()->isRunning())
    {
      Smb4KScanner::self()->abortAll();
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KNetworkBrowserDockWidget::slotAddBookmark(bool /*checked*/)
{
  QList<QTreeWidgetItem *> items = m_networkBrowser->selectedItems();
  QList<SharePtr> shares;

  if (!items.isEmpty())
  {
    for (int i = 0; i < items.size(); ++i)
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(items.at(i));

      if (item && item->type() == Share && !item->shareItem()->isPrinter())
      {
        shares << item->shareItem();
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // No selected items. Just return.
    return;
  }

  if (!shares.isEmpty())
  {
    Smb4KBookmarkHandler::self()->addBookmarks(shares, m_networkBrowser);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotMountManually(bool /*checked*/)
{
  Smb4KMounter::self()->openMountDialog(m_networkBrowser);
}


void Smb4KNetworkBrowserDockWidget::slotAuthentication(bool /*checked*/)
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_networkBrowser->currentItem());

  if (item)
  {
    switch (item->type())
    {
      case Host:
      {
        Smb4KWalletManager::self()->showPasswordDialog(item->hostItem(), m_networkBrowser);
        break;
      }
      case Share:
      {
        Smb4KWalletManager::self()->showPasswordDialog(item->shareItem(), m_networkBrowser);
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
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotCustomOptions(bool /*checked*/)
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_networkBrowser->currentItem());
  
  if (item)
  {
    switch (item->type())
    {
      case Host:
      {
        Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(item->hostItem(), m_networkBrowser);
        break;
      }
      case Share:
      {
        Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(item->shareItem(), m_networkBrowser);
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
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotPreview(bool /*checked*/)
{
  QList<QTreeWidgetItem *> items = m_networkBrowser->selectedItems();
  
  if (!items.isEmpty())
  {
    for (int i = 0; i < items.size(); ++i)
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(items.at(i));

      if (item && item->type() == Share && !item->shareItem()->isPrinter())
      {
        Smb4KPreviewer::self()->preview(item->shareItem(), m_networkBrowser);
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
}


void Smb4KNetworkBrowserDockWidget::slotPrint(bool /*checked*/)
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_networkBrowser->currentItem());
  
  if (item && item->shareItem()->isPrinter())
  {
    Smb4KPrint::self()->print(item->shareItem(), m_networkBrowser);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotMountActionTriggered(bool /*checked*/)
{
  //
  // Get the selected items
  // 
  QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();
  
  if (selectedItems.size() > 1)
  {
    // 
    // In the case of multiple selected network items, selectedItems() 
    // only contains shares. Thus, we do not need to test for the type.
    // For deciding what the mount action is supposed to do, i.e. mount
    // the (remaining) selected unmounted shares or unmounting all selected
    // mounted shares, we use the number of unmounted shares. If that is
    // greater than 0, we mount all shares that need to be mounted, otherwise
    // we unmount all selected shares.
    // 
    QList<SharePtr> unmounted, mounted;
    
    for (QTreeWidgetItem *item : selectedItems)
    {
      Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(item);
      
      if (browserItem && browserItem->shareItem()->isMounted())
      {
        mounted << browserItem->shareItem();
      }
      else if (browserItem && !browserItem->shareItem()->isMounted())
      {
        unmounted << browserItem->shareItem();
      }
      else
      {
        // Do nothing
      }
    }
    
    if (!unmounted.empty())
    {
      // Mount the (remaining) unmounted shares.
      Smb4KMounter::self()->mountShares(unmounted, m_networkBrowser);
    }
    else
    {
      // Unmount all shares.
      Smb4KMounter::self()->unmountShares(mounted, m_networkBrowser);
    }
  }
  else
  {
    // 
    // If only one network item is selected, we need to test for the type
    // of the item. Only in case of a share we need to do something.
    // 
    Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(selectedItems.first());

    if (browserItem)
    {
      switch (browserItem->type())
      {
        case Share:
        {
          if (!browserItem->shareItem()->isMounted())
          {
            Smb4KMounter::self()->mountShare(browserItem->shareItem(), m_networkBrowser);
          }
          else
          {
            Smb4KMounter::self()->unmountShare(browserItem->shareItem(), false, m_networkBrowser);
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
      // Do nothing
    }
  }
}


void Smb4KNetworkBrowserDockWidget::slotMountActionChanged(bool active)
{
  //
  // Get the mount action
  // 
  KDualAction *mountAction = static_cast<KDualAction *>(m_actionCollection->action("mount_action"));
  
  //
  // Change the shortcuts depending on the value of the 'active' argument
  // 
  if (mountAction)
  {
    if (active)
    {
      m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL+Qt::Key_M));
    }
    else
    {
      m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL+Qt::Key_U));
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotScannerAboutToStart(const NetworkItemPtr& /*item*/, int /*process*/)
{
  //
  // Get the rescan/abort action
  // 
  KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"));
  
  //
  // Make adjustments
  // 
  if (rescanAbortAction)
  {
    rescanAbortAction->setActive(!rescanAbortAction->isActive());
    
    if (rescanAbortAction->isActive())
    {
      m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Refresh);
    }
    else
    {
      m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Cancel);
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotScannerFinished(const NetworkItemPtr& /*item*/, int /*process*/)
{
  //
  // Get the rescan/abort action
  // 
  KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"));
  
  //
  // Make adjustments
  // 
  if (rescanAbortAction)
  {
    rescanAbortAction->setActive(!rescanAbortAction->isActive());
    
    if (rescanAbortAction->isActive())
    {
      m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Refresh);
    }
    else
    {
      m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Cancel);
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotShareMounted(const SharePtr& share)
{
  QTreeWidgetItemIterator it(m_networkBrowser);
  
  while (*it)
  {
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);
    
    if (item->type() == Share)
    {
      if (QString::compare(item->shareItem()->unc(), share->unc(), Qt::CaseInsensitive) == 0)
      {
        item->update();
        break;
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
    
    ++it;
  }
}


void Smb4KNetworkBrowserDockWidget::slotShareUnmounted(const SharePtr& share)
{
  QTreeWidgetItemIterator it(m_networkBrowser);
  
  while (*it)
  {
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);
    
    if (item->type() == Share)
    {
      if (QString::compare(item->shareItem()->unc(), share->unc(), Qt::CaseInsensitive) == 0)
      {
        item->update();
        break;
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
    
    ++it;
  }
}


void Smb4KNetworkBrowserDockWidget::slotMounterAboutToStart(int /*process*/)
{
  //
  // Unused at the moment
  // 
}


void Smb4KNetworkBrowserDockWidget::slotMounterFinished(int process)
{
  //
  // Get the mount/unmount action
  // 
  KDualAction *mountAction = static_cast<KDualAction *>(m_actionCollection->action("mount_action"));

  //
  // Make adjustments
  // 
  if (mountAction)
  {
    switch (process)
    {
      case MountShare:
      {
        mountAction->setActive(false);
        break;
      }
      case UnmountShare:
      {
        mountAction->setActive(true);
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
    // Do nothing
  }
}


void Smb4KNetworkBrowserDockWidget::slotIconSizeChanged(int group)
{
  switch (group)
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
      m_networkBrowser->setIconSize(QSize(icon_size, icon_size));
      break;
    }
    default:
    {
      break;
    }
  }
}
