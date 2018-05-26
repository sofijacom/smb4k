/***************************************************************************
    Bookmark editor for Smb4K
                             -------------------
    begin                : Sun Apr 14 2018
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application includes
#include "smb4kbookmarkeditor.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QTimer>
#include <QDragEnterEvent>
#include <QMenu>
#include <QInputDialog>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KConfigCore/KConfigGroup>
#include <KIconThemes/KIconLoader>
#include <KConfigGui/KWindowConfig>


Smb4KBookmarkEditor::Smb4KBookmarkEditor(QWidget *parent)
: QDialog(parent)
{
  setWindowTitle(i18n("Edit Bookmarks"));
  
  setupView();
  loadBookmarks();

  setMinimumWidth(sizeHint().height() > sizeHint().width() ? sizeHint().height() : sizeHint().width());

  KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkEditor");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
  m_label_edit->completionObject()->setItems(group.readEntry("LabelCompletion", QStringList()));
  m_login_edit->completionObject()->setItems(group.readEntry("LoginCompletion", QStringList()));
  m_ip_edit->completionObject()->setItems(group.readEntry("IPCompletion", QStringList()));
  m_group_combo->completionObject()->setItems(group.readEntry("GroupCompletion", m_groups));

  connect(KIconLoader::global(), SIGNAL(iconChanged(int)), SLOT(slotIconSizeChanged(int)));
}


Smb4KBookmarkEditor::~Smb4KBookmarkEditor()
{
  while (!m_bookmarks.isEmpty())
  {
    m_bookmarks.takeFirst().clear();
  }
}


bool Smb4KBookmarkEditor::eventFilter(QObject *obj, QEvent *e)
{
  if (obj == m_tree_widget->viewport())
  {
    switch (e->type())
    {
      case QEvent::DragEnter:
      {
        QDragEnterEvent *ev = static_cast<QDragEnterEvent *>(e);
        
        if (ev->source() == m_tree_widget->viewport())
        {
          e->accept();
        }
        else
        {
          e->ignore();
        }
        break;
      }
      case QEvent::DragLeave:
      {
        e->ignore();
        break;
      }
      case QEvent::Drop:
      {
        QTimer::singleShot(50, this, SLOT(slotAdjust()));
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  return QDialog::eventFilter(obj, e);
}


void Smb4KBookmarkEditor::setupView()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);

  m_tree_widget = new QTreeWidget(this);
  m_tree_widget->setColumnCount(2);
  m_tree_widget->hideColumn((m_tree_widget->columnCount() - 1)); // for sorting purposes
  m_tree_widget->headerItem()->setHidden(true);
  m_tree_widget->setRootIsDecorated(true);
  m_tree_widget->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tree_widget->setContextMenuPolicy(Qt::CustomContextMenu);
  m_tree_widget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_tree_widget->setDragDropMode(QTreeWidget::InternalMove);
  int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
  m_tree_widget->setIconSize(QSize(icon_size, icon_size));
  m_tree_widget->viewport()->installEventFilter(this);

  m_add_group = new QAction(KDE::icon("bookmark-add-folder"), i18n("Add Group"), m_tree_widget);
  m_delete = new QAction(KDE::icon("edit-delete"), i18n("Remove"), m_tree_widget);
  m_clear = new QAction(KDE::icon("edit-clear"), i18n("Clear"), m_tree_widget);
  
  m_menu = new KActionMenu(m_tree_widget);
  m_menu->addAction(m_add_group);
  m_menu->addAction(m_delete);
  m_menu->addAction(m_clear);

  m_editors = new QWidget(this);
  m_editors->setEnabled(false);

  QGridLayout *editors_layout = new QGridLayout(m_editors);
  editors_layout->setSpacing(5);
  editors_layout->setMargin(0);

  QLabel *l_label = new QLabel(i18n("Label:"), m_editors);
  m_label_edit = new KLineEdit(m_editors);
  m_label_edit->setClearButtonShown(true);

  QLabel *lg_label = new QLabel(i18n("Login:"), m_editors);
  m_login_edit = new KLineEdit(m_editors);
  m_login_edit->setClearButtonShown(true);

  QLabel *i_label = new QLabel(i18n("IP Address:"), m_editors);
  m_ip_edit = new KLineEdit(m_editors);
  m_ip_edit->setClearButtonShown(true);
  
  QLabel *g_label = new QLabel(i18n("Group:"), m_editors);
  m_group_combo = new KComboBox(true, m_editors);
  m_group_combo->setDuplicatesEnabled(false);

  editors_layout->addWidget(l_label, 0, 0, 0);
  editors_layout->addWidget(m_label_edit, 0, 1, 0);
  editors_layout->addWidget(lg_label, 1, 0, 0);
  editors_layout->addWidget(m_login_edit, 1, 1, 0);
  editors_layout->addWidget(i_label, 2, 0, 0);
  editors_layout->addWidget(m_ip_edit, 2, 1, 0);
  editors_layout->addWidget(g_label, 3, 0, 0);
  editors_layout->addWidget(m_group_combo, 3, 1, 0);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_ok_button->setShortcut(Qt::CTRL|Qt::Key_Return);
  m_cancel_button->setShortcut(Qt::Key_Escape);

  m_ok_button->setDefault(true);

  layout->addWidget(m_tree_widget);
  layout->addWidget(m_editors);
  layout->addWidget(buttonBox);

  connect(m_tree_widget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), 
          this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));
  
  connect(m_tree_widget, SIGNAL(customContextMenuRequested(QPoint)), 
          this, SLOT(slotContextMenuRequested(QPoint)));
  
  connect(m_label_edit, SIGNAL(editingFinished()), 
          this, SLOT(slotLabelEdited()));
  
  connect(m_ip_edit, SIGNAL(editingFinished()), 
          this, SLOT(slotIPEdited()));
  
  connect(m_login_edit, SIGNAL(editingFinished()), 
          this, SLOT(slotLoginEdited()));
  
  connect(m_group_combo->lineEdit(), SIGNAL(editingFinished()), 
          this, SLOT(slotGroupEdited()));
  
  connect(m_add_group, SIGNAL(triggered(bool)), 
          this, SLOT(slotAddGroupTriggered(bool)));
  
  connect(m_delete, SIGNAL(triggered(bool)), 
          this, SLOT(slotDeleteTriggered(bool)));
  
  connect(m_clear, SIGNAL(triggered(bool)), 
          this, SLOT(slotClearTriggered(bool)));
  
  connect(m_ok_button, SIGNAL(clicked()), 
          this, SLOT(slotDialogAccepted()));
  
  connect(m_cancel_button, SIGNAL(clicked()), 
          this, SLOT(slotDialogRejected()));
}


void Smb4KBookmarkEditor::loadBookmarks()
{
  //
  // Clear the tree widget and the group combo box
  //
  m_tree_widget->clear();
  m_group_combo->clear();
  
  //
  // Copy the global bookmarks list to the internal one
  // 
  m_bookmarks = Smb4KBookmarkHandler::self()->bookmarksList();
    
  // 
  // Copy the groups into the internal list
  // 
  m_groups.clear();
  
  for (const BookmarkPtr &bookmark : m_bookmarks)
  {
    if (!m_groups.contains(bookmark->groupName()))
    {
      m_groups << bookmark->groupName();
    }
    else
    {
      // Do nothing
    }
  }
  
  //
  // Insert the groups into the tree widget
  // 
  for (const QString &group : m_groups)
  {
    if (!group.isEmpty())
    {
      QTreeWidgetItem *groupItem = new QTreeWidgetItem(QTreeWidgetItem::UserType);
      groupItem->setIcon(0, KDE::icon("folder-bookmark"));
      groupItem->setText(0, group);
      groupItem->setText((m_tree_widget->columnCount() - 1), QString("00_%1").arg(group));
      groupItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDropEnabled);
      m_tree_widget->addTopLevelItem(groupItem);
    }
    else
    {
      // Do nothing
    }
  }
  
  // 
  // Insert the bookmarks info the tree widget
  // 
  for (const BookmarkPtr &bookmark : m_bookmarks)
  {
    QTreeWidgetItem *bookmarkItem = new QTreeWidgetItem(QTreeWidgetItem::UserType);
    bookmarkItem->setData(0, QTreeWidgetItem::UserType, static_cast<QUrl>(bookmark->url()));
    bookmarkItem->setIcon(0, bookmark->icon());
    bookmarkItem->setText(0, bookmark->unc());
    bookmarkItem->setText((m_tree_widget->columnCount() - 1), QString("01_%1").arg(bookmark->unc()));
    bookmarkItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
    
    if (!bookmark->groupName().isEmpty())
    {
      QList<QTreeWidgetItem *> items = m_tree_widget->findItems(bookmark->groupName(), Qt::MatchFixedString|Qt::MatchCaseSensitive, 0);
      
      if (!items.isEmpty())
      {
        items.first()->addChild(bookmarkItem);
        items.first()->setExpanded(true);
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      m_tree_widget->addTopLevelItem(bookmarkItem);
    }
  }

  // 
  // Sort
  // 
  for (int i = 0; i < m_tree_widget->topLevelItemCount(); ++i)
  {
    m_tree_widget->topLevelItem(i)->sortChildren((m_tree_widget->columnCount() - 1), Qt::AscendingOrder);
  }
  
  m_tree_widget->sortItems((m_tree_widget->columnCount() - 1), Qt::AscendingOrder);
  
  //
  // Check that an empty group entry is also present. If it is not there,
  // add it now and insert the groups to the group combo box afterwards.
  // 
  if (!m_groups.contains("") && !m_groups.contains(QString()))
  {
    m_groups << "";
  }
  else
  {
    // Do nothing
  }
  
  m_group_combo->addItems(m_groups);
  m_group_combo->setCurrentItem("");
}


BookmarkPtr Smb4KBookmarkEditor::findBookmark(const QUrl &url)
{
  BookmarkPtr bookmark;

  for (const BookmarkPtr &b : m_bookmarks)
  {
    if (b->url() == url)
    {
      bookmark = b;
      break;
    }
    else
    {
      continue;
    }
  }

  return bookmark;
}


void Smb4KBookmarkEditor::slotItemClicked(QTreeWidgetItem *item, int /*col*/)
{
  if (item)
  {
    if (m_tree_widget->indexOfTopLevelItem(item) != -1)
    {
      // This is a top-level item, i.e. it is either a bookmark without
      // group or a group entry.
      // Bookmarks have an URL stored, group folders not.
      if (!item->data(0, QTreeWidgetItem::UserType).toUrl().isEmpty())
      {
        BookmarkPtr bookmark = findBookmark(item->data(0, QTreeWidgetItem::UserType).toUrl());

        if (bookmark)
        {
          m_label_edit->setText(bookmark->label());
          m_login_edit->setText(bookmark->login());
          m_ip_edit->setText(bookmark->hostIP());
          m_group_combo->setCurrentItem(bookmark->groupName());
          m_editors->setEnabled(true);
        }
        else
        {
          m_label_edit->clear();
          m_login_edit->clear();
          m_ip_edit->clear();
          m_group_combo->clearEditText();
          m_editors->setEnabled(false);
        }
      }
      else
      {
        m_label_edit->clear();
        m_login_edit->clear();
        m_ip_edit->clear();
        m_group_combo->clearEditText();
        m_editors->setEnabled(false);
      }
    }
    else
    {
      // This can only be a bookmark.
      BookmarkPtr bookmark = findBookmark(item->data(0, QTreeWidgetItem::UserType).toUrl());

      if (bookmark)
      {
        m_label_edit->setText(bookmark->label());
        m_login_edit->setText(bookmark->login());
        m_ip_edit->setText(bookmark->hostIP());
        m_group_combo->setCurrentItem(bookmark->groupName());
        m_editors->setEnabled(true);
      }
      else
      {
        m_label_edit->clear();
        m_login_edit->clear();
        m_ip_edit->clear();
        m_group_combo->clearEditText();
        m_editors->setEnabled(false);
      }
    }
  }
  else
  {
    m_label_edit->clear();
    m_login_edit->clear();
    m_ip_edit->clear();
    m_group_combo->clearEditText();
    m_editors->setEnabled(false);
  }
}


void Smb4KBookmarkEditor::slotContextMenuRequested(const QPoint &pos)
{
  QTreeWidgetItem *item = m_tree_widget->itemAt(pos);
  m_delete->setEnabled((item));
  m_menu->menu()->popup(m_tree_widget->viewport()->mapToGlobal(pos));  
}


void Smb4KBookmarkEditor::slotLabelEdited()
{
  // Set the label
  QUrl url = m_tree_widget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();

  BookmarkPtr bookmark = findBookmark(url);

  if (bookmark)
  {
    bookmark->setLabel(m_label_edit->userText());
  }
  else
  {
    // Do nothing
  }

  // Add label to completion object
  KCompletion *completion = m_label_edit->completionObject();

  if (!m_label_edit->userText().isEmpty())
  {
    completion->addItem(m_label_edit->userText());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotLoginEdited()
{
  // Set the login
  QUrl url = m_tree_widget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();

  BookmarkPtr bookmark = findBookmark(url);

  if (bookmark)
  {
    bookmark->setLogin(m_login_edit->userText());
  }
  else
  {
    // Do nothing
  }

  // Add login to completion object
  KCompletion *completion = m_login_edit->completionObject();

  if (!m_login_edit->userText().isEmpty())
  {
    completion->addItem(m_login_edit->userText());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotIPEdited()
{
  // Set the ip address
  QUrl url = m_tree_widget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();

  BookmarkPtr bookmark = findBookmark(url);

  if (bookmark)
  {
    bookmark->setHostIP(m_ip_edit->userText());
  }
  else
  {
    // Do nothing
  }

  // Add login to completion object
  KCompletion *completion = m_ip_edit->completionObject();

  if (!m_ip_edit->userText().isEmpty())
  {
    completion->addItem(m_ip_edit->userText());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotGroupEdited()
{
  //
  // Get the URL of the current item.
  //
  QUrl url = m_tree_widget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();
  
  //
  // Return here, if the current item is a group
  //
  if (url.isEmpty())
  {
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // Set the group name to the bookmark
  //
  BookmarkPtr bookmark = findBookmark(url);
  
  if (bookmark)
  {
    bookmark->setGroupName(m_group_combo->currentText());
  }
  else
  {
    // Do nothing
  }
  
  //
  // Reload the bookmarks (The current item is cleared by this!)
  //
  loadBookmarks();
  
  //
  // Reset the current item
  // 
  QTreeWidgetItemIterator it(m_tree_widget);
  
  while (*it)
  {
    if ((*it)->data(0, QTreeWidgetItem::UserType).toUrl() == url)
    {
      m_tree_widget->setCurrentItem(*it);
      slotItemClicked(*it, 0);
      break;
    }
    else
    {
      // Do nothing
    }
    
    ++it;
  }

  // 
  // Add the group to the completion object
  // 
  KCompletion *completion = m_group_combo->completionObject();

  if (!m_group_combo->currentText().isEmpty())
  {
    completion->addItem(m_group_combo->currentText());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotAddGroupTriggered(bool /*checked*/)
{
  bool ok = false;
  
  QString group_name = QInputDialog::getText(this, i18n("Add Group"), i18n("Group name:"), QLineEdit::Normal, QString(), &ok);

  if (ok && !group_name.isEmpty() && m_tree_widget->findItems(group_name, Qt::MatchFixedString|Qt::MatchCaseSensitive, 0).isEmpty())
  {
    // Create a new group item and add it to the widget
    QTreeWidgetItem *group = new QTreeWidgetItem(QTreeWidgetItem::UserType);
    group->setIcon(0, KDE::icon("folder-bookmark"));
    group->setText(0, group_name);
    group->setText((m_tree_widget->columnCount() - 1), QString("00_%1").arg(group_name));
    group->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsDropEnabled) ;
    m_tree_widget->addTopLevelItem(group);
    m_tree_widget->sortItems((m_tree_widget->columnCount() - 1), Qt::AscendingOrder);

    // Add the group to the combo box
    m_group_combo->addItem(group_name);
    m_group_combo->completionObject()->addItem(group_name);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkEditor::slotDeleteTriggered(bool /*checked*/)
{
  //
  // Remove the bookmarks from the view and the internal list
  //
  QList<QTreeWidgetItem *> selected = m_tree_widget->selectedItems();
  
  while (!selected.isEmpty())
  {
    QTreeWidgetItem *item = selected.takeFirst();    
    QUrl url = item->data(0, QTreeWidgetItem::UserType).toUrl();
    
    QMutableListIterator<BookmarkPtr> it(m_bookmarks);
    
    while (it.hasNext())
    {
      BookmarkPtr bookmark = it.next();
      
      if (bookmark->url() == url)
      {
        it.remove();
        break;
      }
      else
      {
        // Do nothing
      }
    }
    
    delete item;
  }
}


void Smb4KBookmarkEditor::slotClearTriggered(bool /*checked*/)
{
  m_tree_widget->clear();
  m_bookmarks.clear();
  m_groups.clear();
}


void Smb4KBookmarkEditor::slotDialogAccepted()
{
  //
  // Write the bookmarks
  //
  Smb4KBookmarkHandler::self()->addBookmarks(m_bookmarks, true);
  
  //
  // Write the dialog properties to the config file
  // 
  KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkEditor");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  group.writeEntry("LabelCompletion", m_label_edit->completionObject()->items());
  group.writeEntry("LoginCompletion", m_login_edit->completionObject()->items());
  group.writeEntry("IPCompletion", m_ip_edit->completionObject()->items());
  group.writeEntry("GroupCompletion", m_group_combo->completionObject()->items());
  
  //
  // Accept the dialog
  // 
  accept();
}


void Smb4KBookmarkEditor::slotDialogRejected()
{
  //
  // Reset the changes made to the bookmarks
  // 
  Smb4KBookmarkHandler::self()->resetBookmarks();
  
  //
  // Reject the dialog
  // 
  reject();
}



void Smb4KBookmarkEditor::slotIconSizeChanged(int group)
{
  switch (group)
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
      m_tree_widget->setIconSize(QSize(icon_size, icon_size));
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KBookmarkEditor::slotAdjust()
{
  // Do the necessary adjustments:
  QTreeWidgetItemIterator it(m_tree_widget);
  while (*it)
  {
    if (!(*it)->parent())
    {
      if ((*it)->data(0, QTreeWidgetItem::UserType).toUrl().isEmpty())
      {
        if ((*it)->childCount() == 0)
        {
          delete *it;
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        BookmarkPtr bookmark = findBookmark((*it)->data(0, QTreeWidgetItem::UserType).toUrl());
      
        if (bookmark)
        {
          bookmark->setGroupName("");
        }
        else
        {
          // Do nothing
        }        
      }
    }
    else
    {
      BookmarkPtr bookmark = findBookmark((*it)->data(0, QTreeWidgetItem::UserType).toUrl());
      
      if (bookmark)
      {
        bookmark->setGroupName((*it)->parent()->text(0));
      }
      else
      {
        // Do nothing
      }
    }
    ++it;
  }
}
