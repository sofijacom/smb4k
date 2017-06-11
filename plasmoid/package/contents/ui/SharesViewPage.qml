/***************************************************************************
 *   Copyright (C) 2017 by A. Reinholdt <alexander.reinholdt@kdemail.net>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQml.Models 2.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.smb4k.smb4kqmlplugin 2.0

PlasmaComponents.Page {
  id: sharesViewPage
  
  //
  // Tool bar
  //
  PlasmaComponents.ToolBar {
    id: sharesViewToolBar
    
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
    
    tools: PlasmaComponents.ToolBarLayout {
      PlasmaComponents.ToolButton {
        id: unmountAllButton
        tooltip: i18n("Unmount all shares")
        iconSource: "system-run"
        width: minimumWidth
        onClicked: {
          iface.unmountAll()
        }
      }
    }
  }
  
  //
  // Delegate Model (used for sorting)
  //
  DelegateModel {
    id: sharesViewItemDelegateModel
    
    function lessThan(left, right) {
      var less = false
      
      if (left.hostName == right.hostName) {
        less = (left.shareName < right.shareName)
      }
      else {
        less = (left.shareName < right.shareName && left.hostName < right.hostName)
      }

      return less
    }
    
    function insertPosition(item) {
      var lower = 0
      var upper = items.count
      
      while (lower < upper) {
        var middle = Math.floor(lower + (upper - lower) / 2)
        var result = lessThan(item.model.object, items.get(middle).model.object)
        if (result) {
          upper = middle
        }
        else {
          lower = middle + 1
        }
      }
      return lower
    }
    
    function sort() {
      while (unsortedItems.count > 0) {
        var item = unsortedItems.get(0)
        var index = insertPosition(item)
        
        item.groups = "items"
        items.move(item.itemsIndex, index)
      }
    }
    
    items.includeByDefault: false
    
    groups: [ 
      DelegateModelGroup {
        id: unsortedItems
        name: "unsorted"
      
        includeByDefault: true
      
        onChanged: {
          sharesViewItemDelegateModel.sort()
        }
      }
    ]

    filterOnGroup: "items"
    
    model: ListModel {}
    
    delegate: SharesViewItemDelegate {
      id: sharesViewItemDelegate
        
      onItemClicked: {
        sharesViewListView.currentIndex = DelegateModel.itemsIndex
        var object = sharesViewItemDelegateModel.items.get(DelegateModel.itemsIndex).model.object
        if (object !== null) {
          Qt.openUrlExternally(object.mountpoint)
        }
        else {
          // Do nothing
        }
      }
        
      onUnmountClicked: {
        sharesViewListView.currentIndex = DelegateModel.itemsIndex
        var object = sharesViewItemDelegateModel.items.get(DelegateModel.itemsIndex).model.object
        if (object !== null) {
          iface.unmount(object)
        }
        else {
          // Do nothing
        }
      }
        
      onBookmarkClicked: {
        sharesViewListView.currentIndex = DelegateModel.itemsIndex
        var object = sharesViewItemDelegateModel.items.get(DelegateModel.itemsIndex).model.object
        if (object !== null) {
          iface.addBookmark(object)
        }
        else {
          // Do nothing
        }
      }
        
      onSyncClicked: {
        sharesViewListView.currentIndex = DelegateModel.itemsIndex
        var object = sharesViewItemDelegateModel.items.get(DelegateModel.itemsIndex).model.object
        if (object !== null) {
          iface.synchronize(object)
        }
        else {
          // Do nothing
        }
      }
    }
  }
  
  //
  // List view
  //
  PlasmaExtras.ScrollArea {
    id: sharesViewScrollArea
    
    anchors {
      top: sharesViewToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    
    ListView {
      id: sharesViewListView
      anchors.fill: parent
      model: sharesViewItemDelegateModel
      clip: true
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    onMountedSharesChanged: shareMountedOrUnmounted()
  }
  
  //
  // Functions
  //
  function shareMountedOrUnmounted() {
    while (sharesViewItemDelegateModel.model.count != 0) {
      sharesViewItemDelegateModel.model.remove(0)
    }
    
    for (var i = 0; i < iface.mountedShares.length; i++) {
      // The unmounted() signal is emitted before the share is
      // actually removed from the list. So, we need to check 
      // here, if the share is still mounted.
      var mountedShare = iface.mountedShares[i]
      if (mountedShare !== null && mountedShare.isMounted) {
        sharesViewItemDelegateModel.model.append({"object": iface.mountedShares[i]})
      }
      else {
        // Do nothing
      }
    }
  }
}
