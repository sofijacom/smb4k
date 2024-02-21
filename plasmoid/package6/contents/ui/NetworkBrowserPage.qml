/*
    SPDX-FileCopyrightText: 2017-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.smb4k.smb4kqmlplugin


PlasmaComponents.Page {
  id: networkBrowserPage
  anchors.fill: parent
  
  property var parentObject: 0
  
  //
  // Tool bar
  //
  ToolBar {
    id: networkBrowserToolBar
    
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
    
    RowLayout {
      ToolButton {
        id: rescanButton
        text: i18n("Rescan")
        icon.name: "view-refresh"
        onClicked: {
          rescan()
        }
      }

      ToolButton {
        id: abortButton
        text: i18n("Abort")
        icon.name: "process-stop"
        onClicked: {
          abort()
        }
      }
      
      ToolButton {
        id: upButton
        text: i18n("Go one level up")
        icon.name: "go-up-symbolic"
        onClicked: {
          up()
        }
      }
      
      ToolButton {
        id: mountDialogButton
        text: i18n("Open the mount dialog")
        icon.name: "view-form"
        onClicked: {
          // FIXME: Use Plasma dialog
          iface.openMountDialog()
        }
      }
    }
  }
  
  //
  // Delegate Model (used for sorting)
  //
  DelegateModel {
    id: networkBrowserItemDelegateModel
    
    function lessThan(left, right) {
      var less = false
      
      switch (left.type) {
        case NetworkObject.Workgroup:
          less = (left.workgroupName < right.workgroupName)
          break
        case NetworkObject.Host:
          less = (left.hostName < right.hostName)
          break
        case NetworkObject.Share:
          less = (left.shareName < right.shareName)
          break
        default:
          break          
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
          networkBrowserItemDelegateModel.sort()
        }
      }
    ]

    filterOnGroup: "items"
    
    model: ListModel {}
    
    delegate: NetworkBrowserItemDelegate {
      id: networkBrowserItemDelegate
        
      onItemClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        networkItemClicked()
      }
        
      onBookmarkClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.addBookmark(object)
      }
        
      onPreviewClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.preview(object)
      }
        
      onConfigureClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.openCustomOptionsDialog(object)
      }
    }
  }
      
  //
  // List view 
  //
  ScrollView {
    id: networkBrowserScrollArea
    
    anchors {
      top: networkBrowserToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    
    ListView {
      id: networkBrowserListView
      anchors.fill: parent
      model: networkBrowserItemDelegateModel
      clip: true
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
//       highlight: Rectangle {
//         color: theme.highlightColor
//         radius: 5
//         opacity: 0.2
//       }
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    function onWorkgroupsChanged() { getWorkgroups() }
    function onHostsChanged() { getHosts() }
    function onSharesChanged() { getShares() }
    function onMountedSharesChanged() { shareMountedOrUnmounted() }
  }
  
  //
  // Initialization
  //
  Component.onCompleted: {
    if (networkBrowserListView.count == 0) {
      getWorkgroups()
    }
  }  
  
  //
  // Functions
  //
  function rescan() {
    if (parentObject !== null) {
      iface.lookup(parentObject)
    }
    else {
      iface.lookup()
    }
  }
  
  function abort() {
    iface.abortClient()
    iface.abortMounter()
  }
  
  function up() {
    if (parentObject !== null) {
      
      switch (parentObject.type) {
        case NetworkObject.Workgroup:
          networkBrowserListView.currentIndex = -1
          iface.lookup()
          break
        case NetworkObject.Host:
          var object = iface.findNetworkItem(parentObject.parentUrl, parentObject.parentType)
          if (object !== null) {
            parentObject = object
            iface.lookup(object)
          }
          break
        default:
          break
      }
    }
  }
  
  function networkItemClicked() {
    if (networkBrowserListView.currentIndex != -1) {
      var object = networkBrowserItemDelegateModel.items.get(networkBrowserListView.currentIndex).model.object
      
      if (object.type == NetworkObject.Share) {
        if (!object.isPrinter) {
          iface.mountShare(object)
        }
        else {
          iface.print(object)
        }
      }
      else {
        parentObject = object
        iface.lookup(object)
      }
    }
  }
  
  function getWorkgroups() {
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.workgroups.length != 0) {
      for (var i = 0; i < iface.workgroups.length; i++) {
        networkBrowserItemDelegateModel.model.append({"object": iface.workgroups[i]})
      }
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function getHosts() {
    var workgroupName = parentObject.workgroupName
    
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.hosts.length != 0) {
      for (var i = 0; i < iface.hosts.length; i++) {
        if (workgroupName.length != 0 && workgroupName == iface.hosts[i].workgroupName) {
          networkBrowserItemDelegateModel.model.append({"object": iface.hosts[i]})
        }
      }
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function getShares() {
    var hostName = parentObject.hostName
    
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.shares.length != 0) {
      for (var i = 0; i < iface.shares.length; i++) {
        if (hostName.length != 0 && hostName == iface.shares[i].hostName) {
          networkBrowserItemDelegateModel.model.append({"object": iface.shares[i]})
        }
      }
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function shareMountedOrUnmounted() {
    for (var i = 0; i < networkBrowserItemDelegateModel.model.count; i++) {
      var object = networkBrowserItemDelegateModel.model.get(i).object
      
      if (object !== null && object.type == NetworkObject.Share) {
        object.isMounted = iface.isShareMounted(object.url)
        networkBrowserItemDelegateModel.model.set(i, {"object": object})
      }
    }
  }
}
