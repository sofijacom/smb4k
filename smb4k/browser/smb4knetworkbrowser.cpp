/***************************************************************************
    smb4knetworkbrowser  -  The network browser widget of Smb4K.
                             -------------------
    begin                : Mo Jan 8 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QMouseEvent>
#include <QFocusEvent>
#include <QWheelEvent>
#include <QHeaderView>
#include <QTimer>
#include <QCursor>
#include <QDesktopWidget>
#include <QHelpEvent>

// KDE includes
#include <klocale.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <kiconloader.h>

// application specific includes
#include <smb4knetworkbrowser.h>
#include <smb4knetworkbrowseritem.h>
#include <../tooltips/smb4ktooltip.h>
#include <core/smb4ksettings.h>


Smb4KNetworkBrowser::Smb4KNetworkBrowser( QWidget *parent )
: QTreeWidget( parent )
{
  setRootIsDecorated( true );
  setAllColumnsShowFocus( false );
  setMouseTracking( true );
  setSelectionMode( SingleSelection );
  
  setContextMenuPolicy( Qt::CustomContextMenu );

  m_tooltip = new Smb4KToolTip( this );
  m_mouse_inside = false;
  m_auto_select_timer = new QTimer( this );

  QStringList header_labels;
  header_labels.append( i18n( "Network" ) );
  header_labels.append( i18n( "Type" ) );
  header_labels.append( i18n( "IP Address" ) );
  header_labels.append( i18n( "Comment" ) );
  setHeaderLabels( header_labels );

  header()->setResizeMode( QHeaderView::ResizeToContents );

  // Add some connections:
  connect( this, SIGNAL( itemExpanded( QTreeWidgetItem * ) ),
           this, SLOT( slotItemExpanded( QTreeWidgetItem * ) ) );

  connect( this, SIGNAL( itemCollapsed( QTreeWidgetItem * ) ),
           this, SLOT( slotItemCollapsed( QTreeWidgetItem * ) ) );

  connect( this, SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ),
           this, SLOT( slotItemExecuted( QTreeWidgetItem *, int ) ) );

  connect( this, SIGNAL( itemEntered( QTreeWidgetItem *, int ) ),
           this, SLOT( slotItemEntered( QTreeWidgetItem *, int ) ) );

  connect( this, SIGNAL( viewportEntered() ),
           this, SLOT( slotViewportEntered() ) );

  // We need to conform with KDE's settings (see also slotKDESettingsChanged(),
  // slotItemEntered() and slotViewportEntered()).
  slotKDESettingsChanged( KGlobalSettings::SETTINGS_MOUSE );

  connect( KGlobalSettings::self(), SIGNAL( settingsChanged( int ) ),
           this,                    SLOT( slotKDESettingsChanged( int ) ) );

  connect( m_auto_select_timer,     SIGNAL( timeout() ),
           this,                    SLOT( slotAutoSelectItem() ) );
}


Smb4KNetworkBrowser::~Smb4KNetworkBrowser()
{
}


bool Smb4KNetworkBrowser::event( QEvent *e )
{
  switch ( e->type() )
  {
    case QEvent::ToolTip:
    {
      // Intercept the tool tip event and show our own tool tip.
      QPoint pos = viewport()->mapFromGlobal( cursor().pos() );
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( itemAt( pos ) );
      
      if ( item )
      {
        if ( Smb4KSettings::showNetworkItemToolTip() )
        {
          int ind = 0;

          switch ( item->type() )
          {
            case Smb4KNetworkBrowserItem::Host:
            {
              ind = 2;
              break;
            }
            case Smb4KNetworkBrowserItem::Share:
            {
              ind = 3;
              break;
            }
            default:
            {
              ind = 1;
              break;
            }
          }
          
          // Check that the tooltip is not over the root decoration.
          // If it is, hide it.
          if ( pos.x() <= ind * indentation() )
          {
            if ( m_tooltip->isVisible() )
            {
              m_tooltip->hide();
            }
            else
            {
              // Do nothing
            }
          }
          else
          {
            if ( !m_tooltip->isVisible() || (m_tooltip->networkItem() && 
                QString::compare( item->networkItem()->key(), m_tooltip->networkItem()->key() ) != 0) )
            {
              switch ( item->type() )
              {
                case Smb4KNetworkBrowserItem::Workgroup:
                {
                  m_tooltip->show( item->workgroupItem(), pos );
                  break;
                }
                case Smb4KNetworkBrowserItem::Host:
                {
                  m_tooltip->show( item->hostItem(), pos );
                  break;
                }
                case Smb4KNetworkBrowserItem::Share:
                {
                  m_tooltip->show( item->shareItem(), pos );
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
        else
        {
          if ( m_tooltip->isVisible() )
          {
            m_tooltip->hide();
          }
          else
          {
            // Do nothing
          }
        }
      }
      else
      {
        if ( m_tooltip->isVisible() )
        {
          m_tooltip->hide();
        }
        else
        {
          // Do nothing
        }
      }
      
      break;
    }
    default:
    {
      break;
    }
  }
  
  return QTreeWidget::event( e );
}


void Smb4KNetworkBrowser::mouseMoveEvent( QMouseEvent *e )
{
  // Find the item over which the user moved the mouse:
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( itemAt( e->pos() ) );

  if ( item )
  {
    emit itemEntered( item, columnAt( e->pos().x() ) );
    
    // Hide tool tip if the items diverge.
    if ( m_tooltip->networkItem() && m_tooltip->networkItem() != item->networkItem() )
    {
      m_tooltip->hide();
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Hide the tool tip
    m_tooltip->hide();
  }

  QTreeWidget::mouseMoveEvent( e );
}


void Smb4KNetworkBrowser::leaveEvent( QEvent *e )
{
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }
  
  m_auto_select_timer->stop();
  m_mouse_inside = false;

  QTreeWidget::leaveEvent( e );
}


void Smb4KNetworkBrowser::enterEvent( QEvent *e )
{
  m_mouse_inside = true;

  QTreeWidget::enterEvent( e );
}


void Smb4KNetworkBrowser::mousePressEvent( QMouseEvent *e )
{
  // Hide the current tool tip so that it is not in the way.
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }

  // Get the item that is under the mouse. If there is no
  // item, unselect the current item.
  QTreeWidgetItem *item = itemAt( e->pos() );

  if ( !item && currentItem() )
  {
    currentItem()->setSelected( false );
    setCurrentItem( NULL );
    emit itemPressed( currentItem(), -1 );
  }
  else
  {
    // Do nothing
  }

  QTreeWidget::mousePressEvent( e );
}


void Smb4KNetworkBrowser::focusOutEvent( QFocusEvent *e )
{
  m_auto_select_timer->stop();
  QTreeWidget::focusOutEvent( e );
}


void Smb4KNetworkBrowser::wheelEvent( QWheelEvent *e )
{
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }
  
  QTreeWidget::wheelEvent( e );
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KNetworkBrowser::slotItemExpanded( QTreeWidgetItem *item )
{
  item->setSelected( true );
}


void Smb4KNetworkBrowser::slotItemCollapsed( QTreeWidgetItem *item )
{
  item->setSelected( false );
}


void Smb4KNetworkBrowser::slotItemEntered( QTreeWidgetItem *item, int /*column*/ )
{
  // Comply with KDE's settings.
  if ( item && m_use_single_click )
  {
    if ( m_change_cursor_over_icon )
    {
      viewport()->setCursor( QCursor( Qt::PointingHandCursor ) );
    }

    if ( m_auto_select_delay > -1 )
    {
      m_auto_select_item = item;
      m_auto_select_timer->setSingleShot( true );
      m_auto_select_timer->start( m_auto_select_delay );
    }
  }
  else
  {
    // Do nothing
  }

  if ( m_tooltip->isVisible() )
  {
    Smb4KNetworkBrowserItem *browser_item = static_cast<Smb4KNetworkBrowserItem *>( item );
  
    if ( browser_item && m_tooltip->networkItem() &&
         QString::compare( browser_item->networkItem()->key(), m_tooltip->networkItem()->key() ) != 0 )
    {
      m_tooltip->hide();
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


void Smb4KNetworkBrowser::slotViewportEntered()
{
  // Comply with KDE's settings.
  if ( m_change_cursor_over_icon )
  {
    viewport()->unsetCursor();
  }
  else
  {
    // Do nothing
  }

  m_auto_select_timer->stop();
  m_auto_select_item = 0;
  
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowser::slotItemExecuted( QTreeWidgetItem *item, int /*column*/ )
{
  if ( m_tooltip->isVisible() )
  {
    m_tooltip->hide();
  }
  else
  {
    // Do nothing
  }

  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Workgroup:
      case Smb4KNetworkBrowserItem::Host:
      {
        if ( !item->isExpanded() )
        {
          expandItem( item );
        }
        else
        {
          collapseItem( item );
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


void Smb4KNetworkBrowser::slotKDESettingsChanged( int category )
{
  // Adjust to KDE's default mouse settings.
  if ( category != KGlobalSettings::SETTINGS_MOUSE )
  {
    return;
  }

  disconnect( this, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ) );
  disconnect( this, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ) );

  m_use_single_click        = KGlobalSettings::singleClick();
  m_change_cursor_over_icon = KGlobalSettings::changeCursorOverIcon();
  m_auto_select_delay       = KGlobalSettings::autoSelectDelay();

  if ( m_use_single_click )
  {
    connect( this, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
             this, SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ) );
  }
  else
  {
    connect( this, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ) );
  }

  if ( !m_use_single_click || !m_change_cursor_over_icon )
  {
    viewport()->unsetCursor();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowser::slotAutoSelectItem()
{
  // Check that the item is still valid.
  QPoint pos = viewport()->mapFromGlobal( cursor().pos() );
  QTreeWidgetItem *item = itemAt( pos );

  if ( !m_auto_select_item || !item || m_auto_select_item != item )
  {
    return;
  }
  else
  {
    // Do nothing. We are OK.
  }

  // Give the widget the keyboard focus.
  if ( !hasFocus() )
  {
    setFocus();
  }
  else
  {
    // Do nothing
  }

  // Now set up the auto selection. Most of this has been "stolen" from
  // the KListWidget code.
  Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

  QTreeWidgetItem *previousItem = currentItem();
  setCurrentItem( m_auto_select_item );

  if ( m_auto_select_item )
  {
    if ( (modifiers & Qt::ShiftModifier) )
    {
      bool block = signalsBlocked();
      blockSignals( true );

      // If no CTRL is pressed, clear before.
      if ( !(modifiers & Qt::ControlModifier ) )
      {
        clearSelection();
      }
      else
      {
        // Do nothing
      }

      bool select = !m_auto_select_item->isSelected();
      bool update = viewport()->updatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = indexFromItem( previousItem ).row() < indexFromItem( m_auto_select_item ).row();

      QTreeWidgetItem *indexItem = down ? previousItem : m_auto_select_item;

      QTreeWidgetItemIterator it( indexItem );

      while ( *it )
      {
        if ( down && *it == m_auto_select_item )
        {
          m_auto_select_item->setSelected( select );

          break;
        }
        else
        {
          // Do nothing
        }

        if ( !down && *it == previousItem )
        {
          previousItem->setSelected( select );
          break;
        }
        else
        {
          // Do nothing
        }

        indexItem->setSelected( select );

        ++it;
      }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );

      emit itemSelectionChanged();

      if ( selectionMode() == QTreeWidget::SingleSelection )
      {
        emit itemSelectionChanged();
      }
      else
      {
        // Do nothing
      }
    }
    else if ( (modifiers & Qt::ControlModifier) )
    {
      m_auto_select_item->setSelected( !m_auto_select_item->isSelected() );
    }
    else
    {
      bool block = signalsBlocked();
      blockSignals( true );

      if ( !m_auto_select_item->isSelected() )
      {
        clearSelection();
      }
      else
      {
        // Do nothing
      }

      blockSignals( block );

      m_auto_select_item->setSelected( true );
    }
  }
  else
  {
    // Do nothing. This should never happen, however.
  }
}


#include "smb4knetworkbrowser.moc"
