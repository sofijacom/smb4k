/***************************************************************************
    sharesviewpage.qml - The shares view of Smb4K's plasmoid
                             -------------------
    begin                : So Apr 28 2013
    copyright            : (C) 2013 by Alexander Reinholdt
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras

PlasmaComponents.Page {
  id: sharesPage
  
  // FIXME: Implement tool bar with unmount all button
  
  PlasmaExtras.ScrollArea {
    id: sharesViewScrollArea
    anchors.fill: parent
    ListView {
      id: sharesView
      delegate: SharesViewItemDelegate {
        id: sharesViewDelegate
        onItemClicked: {
          var object = sharesView.model.get(index).object
          if ( object !== null ) {
            mountedShareClicked( object )
          }
          else {
            // Do nothing
          }            
        }
        onUnmountClicked: {
          var object = sharesView.model.get(index).object
          if ( object !== null ) {
            unmountShare( object, index )
          }
          else {
            // Do nothing
          }
        }
      }
      model: ListModel {}
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: mounter
    onMountedSharesListChanged: shareMountedOrUnmounted()
  }
  
  //
  // (Re-)load the list of mounted shares
  //
  function shareMountedOrUnmounted() {
    while ( sharesView.model.count != 0 ) {
      sharesView.model.remove(0)
    }
    
    for ( var i = 0; i < mounter.mountedShares.length; i++ ) {
      // The unmounted() signal is emitted before the share is
      // actually removed from the list. So, we need to check 
      // here, if the share is still mounted.
      if ( mounter.mountedShares[i].isMounted ) {
        sharesView.model.append( { "object": mounter.mountedShares[i] } )
      }
      else {
        // Do nothing
      }
    }
  }
  
  //
  // A mounted share was clicked
  //
  function mountedShareClicked( object ) {
    Qt.openUrlExternally( object.mountpoint )
  }
  
  //
  // A mounted share is to be unmounted
  //
  function unmountShare( object, index ) {
    sharesView.model.remove( index )
    mounter.unmount( object.mountpoint )
  }
}
