/*
    The main window of Smb4K

    SPDX-FileCopyrightText: 2008-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KMAINWINDOW_H
#define SMB4KMAINWINDOW_H

// application specific includes
#include "core/smb4kglobal.h"


// Qt includes
#include <QActionGroup>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>
#include <QUrl>

// KDE includes
#include <KXmlGuiWindow>

// forward declarations
class Smb4KSystemTray;
class Smb4KPrintInfo;
class Smb4KSynchronizationInfo;
class Smb4KNetworkBrowserDockWidget;
class Smb4KSharesViewDockWidget;
class Smb4KBookmarkMenu;
class Smb4KPasswordDialog;

/**
 * This is the main window of Smb4K. It provides the network browser, the
 * shares view and all other dialogs as well as a menu, status and tool bar.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KMainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    /**
     * The constructor
     */
    Smb4KMainWindow();

    /**
     * The destructor
     */
    ~Smb4KMainWindow();

protected:
    /**
     * Reimplemented from KMainWindow
     */
    bool queryClose() override;

    /**
     * Reimplemented from KMainWindow
     */
    bool eventFilter(QObject *obj, QEvent *e) override;

    /**
     * Reimplemented from QObject
     */
    void timerEvent(QTimerEvent * event) override;

protected Q_SLOTS:
    /**
     * Quits the application.
     */
    void slotQuit();

    /**
     * Opens the configuration dialog.
     */
    void slotConfigDialog();

    /**
     * Reloads settings, if necessary. This slot is connected to
     * the KConfigDialog::settingsChanged() signal.
     *
     * @param dialogName      The name of the configuration dialog
     */
    void slotSettingsChanged(const QString &dialogName);

    /**
     * This slot is called when a bookmark should be added.
     *
     * @param checked       TRUE if the action is checked
     */
    void slotAddBookmarks();

    /**
     * This slot is connected to the Smb4KWalletManager::initialized() signal.
     * It checks the state of the wallet manager and sets the icon in the status
     * bar accordingly.
     */
    void slotWalletManagerInitialized();

    /**
     * This slot shows a busy bar and a status message according to the action performed by
     * the client. It is connected to the Smb4KClient::aboutToStart() signal.
     *
     * @param item          The network item
     *
     * @param process       The process
     */
    void slotClientAboutToStart(const NetworkItemPtr &item, int process);

    /**
     * this slot shows a status message according to the action that was just finished by the
     * client and hides the status bar if appropriate. It is connected to the Smb4KClient::finished()
     * signal.
     *
     * @param item          The network item
     *
     * @param process       The process
     */
    void slotClientFinished(const NetworkItemPtr &item, int process);

    /**
     * This slot shows a status message according to the action performed by the
     * mounter as well as a busy bar. It is connected to the Smb4KMounter::aboutToStart()
     * signal.
     * @param process       The process
     */
    void slotMounterAboutToStart(int process);

    /**
     * This shows a status message according to the action that was just finished by
     * the mounter and hides the busy bar if appropriate. It is connected to the
     * Smb4KMounter::finished() signal.
     * @param process       The process
     */
    void slotMounterFinished(int process);

    /**
     * This slot gives the visual mount feedback in the status bar. It is
     * connected to the Smb4KMounter::mounted() signal.
     *
     * @param share         The share object
     */
    void slotVisualMountFeedback(const SharePtr &share);

    /**
     * This slot gives the visual unmount feedback in the status bar. It is
     * connected to the Smb4KMounter::unmounted() signal.
     *
     * @param share         The share object
     */
    void slotVisualUnmountFeedback(const SharePtr &share);

    /**
     * This slot shows a message according to the action performed by the synchronizer.
     * It is connected to the Smb4KSynchronizer::aboutToStart() signal.
     *
     * @param dest          The path of the destination
     */
    void slotSynchronizerAboutToStart(const QString &dest);

    /**
     * This slot shows a message according to the finished action that were reported
     * by the synchronizer. It is connected to the Smb4KSynchronizer::finished() signal.
     *
     * @param dest          The path of the destination
     */
    void slotSynchronizerFinished(const QString &dest);

    /**
     * This slot hides the feedback icon in the status bar. It is connected to
     * a QTimer::singleShot() signal.
     */
    void slotEndVisualFeedback();

    /**
     * Enable/disable the "Add Bookmark" action
     */
    void slotEnableBookmarkAction();

    /**
     * This slot is connected to the visibilityChanged() signals of the network browser
     * dock widget. It is used to get the tool bars right.
     *
     * @param visible         If the dock widget is visible.
     */
    void slotNetworkBrowserVisibilityChanged(bool visible);

    /**
     * This slot is connected to the visibilityChanged() signals of the shares view
     * dock widget. It is used to get the tool bars right.
     *
     * @param visible         If the dock widget is visible.
     */
    void slotSharesViewVisibilityChanged(bool visible);

    /**
     * This slot is called when credentials are requested.
     *
     * @param networkItem         The network item
     */
    void slotCredentialsRequested(const NetworkItemPtr &networkItem);

private:
    void setupActions();
    void setupStatusBar();
    void setupView();
    void setupMenuBar();
    void setupSystemTrayWidget();
    void loadSettings();
    void saveSettings();
    void setupMountIndicator();
    void setupDynamicActionList(QDockWidget *dock);

    QProgressBar *m_progressBar;
    QLabel *m_passwordIcon;
    QLabel *m_feedbackIcon;
    Smb4KSystemTray *m_systemTrayWidget;
    QWidget *m_focusWidget;
    QActionGroup *m_dockWidgets;
    Smb4KNetworkBrowserDockWidget *m_networkBrowserDockWidget;
    Smb4KSharesViewDockWidget *m_sharesViewDockWidget;
    Smb4KBookmarkMenu *m_bookmarkMenu;
    Smb4KPasswordDialog *m_passwordDialog;
    QList<NetworkItemPtr> m_requestQueue;
    int m_timerId;
};

#endif
