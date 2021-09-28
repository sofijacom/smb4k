/*
    Private classes for the bookmark handler

    SPDX-FileCopyrightText: 2011-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KBOOKMARKHANDLER_P_H
#define SMB4KBOOKMARKHANDLER_P_H

// application specific includes
#include "smb4kbookmarkhandler.h"
#include "smb4kglobal.h"

// Qt includes
#include <QAction>
#include <QDialog>
#include <QListWidget>
#include <QPointer>
#include <QPushButton>
#include <QString>
#include <QTreeWidget>
#include <QUrl>

// KDE includes
#include <KCompletion/KComboBox>
#include <KCompletion/KLineEdit>
#include <KWidgetsAddons/KActionMenu>

class Q_DECL_EXPORT Smb4KBookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * The constructor
     *
     * @param bookmarks       The list of bookmarks that are to be saved
     *
     * @param categories      The list of available bookmark categories
     *
     * @param parent          The parent widget
     */
    Smb4KBookmarkDialog(const QList<BookmarkPtr> &bookmarks, const QStringList &categories, QWidget *parent);

    /**
     * The destructor
     */
    ~Smb4KBookmarkDialog();

    /**
     * Returns the list of bookmarks including all changes that could
     * be made in the bookmark dialog.
     *
     * @returns the list of bookmarks.
     */
    const QList<BookmarkPtr> &bookmarks();

protected Q_SLOTS:
    /**
     * Called when a bookmark was clicked in the list widget.
     */
    void slotBookmarkClicked(QListWidgetItem *bookmarkItem);

    /**
     * Called when the label is edited by the user
     */
    void slotLabelEdited();

    /**
     * Called when the category is edited by the user
     */
    void slotCategoryEdited();

    /**
     * Called when the OK button was clicked
     */
    void slotDialogAccepted();

    /**
     * Called when the icon size changed
     */
    void slotIconSizeChanged(int group);

private:
    /**
     * Sets up the view
     */
    void setupView();

    /**
     * Load the list of bookmarks and the one of the categories
     */
    void loadLists(const QList<BookmarkPtr> &bookmarks, const QStringList &categories);

    /**
     * Finds the bookmark in the list
     */
    BookmarkPtr findBookmark(const QUrl &url);

    /**
     * The list of bookmarks
     */
    QList<BookmarkPtr> m_bookmarks;

    /**
     * The list of groups
     */
    QStringList m_categories;
};

class Smb4KBookmarkEditor : public QDialog
{
    Q_OBJECT

public:
    /**
     * The constructor.
     *
     * @param bookmarks   The list of all bookmarks
     *
     * @param parent      The parent of this dialog.
     */
    explicit Smb4KBookmarkEditor(const QList<BookmarkPtr> &bookmarks, QWidget *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KBookmarkEditor();

    /**
     * Load the bookmarks into the view
     */
    void loadBookmarks();

    /**
     * Return the list of edited bookmarks
     */
    QList<BookmarkPtr> editedBookmarks();

protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject *obj, QEvent *e) override;

protected Q_SLOTS:
    /**
     * Called when a bookmark was clicked
     */
    void slotItemClicked(QTreeWidgetItem *item, int col);

    /**
     * Called when the context menu was requested
     */
    void slotContextMenuRequested(const QPoint &pos);

    /**
     * Called when the label is edited by the user
     */
    void slotLabelEdited();

    /**
     * Called when the category is edited by the user
     */
    void slotCategoryEdited();

    /**
     * Called when the IP address is edited by the user
     */
    void slotIpEdited();

    /**
     * Called when the workgroup name is edited by the user
     */
    void slotWorkgroupNameEdited();

    /**
     * Called when the login is edited by the user
     */
    void slotLoginEdited();

    /**
     * Called when the add action was triggered
     */
    void slotAddCategoryTriggered(bool checked);

    /**
     * Called when the delete action was triggered
     */
    void slotDeleteTriggered(bool checked);

    /**
     * Called when the clear action was triggered
     */
    void slotClearTriggered(bool checked);

    /**
     * Called when the Ok button was clicked
     */
    void slotDialogAccepted();

    /**
     * Called when the Cancel button was clicked
     */
    void slotDialogRejected();

    /**
     * Called when the icon size changed
     */
    void slotIconSizeChanged(int group);

    /**
     * Do adjustments in the list view
     */
    void slotAdjust();

private:
    /**
     * Set up the view
     */
    void setupView();

    /**
     * Finds the bookmark in the list
     */
    BookmarkPtr findBookmark(const QUrl &url);

    /**
     * List of the bookmarks that are being processed
     */
    QList<BookmarkPtr> m_bookmarks;

    /**
     * The list of groups
     */
    QStringList m_categories;
};

class Smb4KBookmarkHandlerPrivate
{
public:
    QList<BookmarkPtr> bookmarks;
    QPointer<Smb4KBookmarkEditor> editor;
};

class Smb4KBookmarkHandlerStatic
{
public:
    Smb4KBookmarkHandler instance;
};

#endif
