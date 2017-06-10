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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.smb4k.smb4kqmlplugin 2.0

PlasmaComponents.Page {
  id: profilesPage
  
  //
  // Tool bar
  //
  // FIXME: Include tool bar
  
  //
  // List view
  //
  PlasmaExtras.ScrollArea {
    id: profilesScrollArea
    
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    
    ListView {
      id: profilesListView
      delegate: ProfileItemDelegate {
        id: profileItemDelegate
        
        onItemClicked: {
          profilesListView.currentIndex = index
          iface.activeProfile = object.profileName
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
    target: iface
    onProfilesListChanged: fillView()
    onActiveProfileChanged: fillView()
  }
  
  //
  // Initialization
  //
  Component.onCompleted: {
    fillView()
  }
  
  //
  // Functions
  //
  function fillView() {
    while (profilesListView.model.count != 0) {
      profilesListView.model.remove(0)
    }
    
    if (iface.profileUsage && iface.profiles.length != 0) {
      for (var i = 0; i < iface.profiles.length; i++) {
        profilesListView.model.append({"object": iface.profiles[i]})
      }
    }
    else {
      // Do nothing
    }
  }  
}