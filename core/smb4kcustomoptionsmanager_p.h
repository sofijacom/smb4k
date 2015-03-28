/***************************************************************************
    smb4kcustomoptionsmanager_p - Private helper classes for 
    Smb4KCustomOptionsManager class
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2015 by Alexander Reinholdt
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

#ifndef SMB4KCUSTOMOPTIONSMANAGER_P_H
#define SMB4KCUSTOMOPTIONSMANAGER_P_H

// application specific includes
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"

// Qt includes
#include <QtCore/QList>
#include <QtGui/QCheckBox>

// KDE includes
#include <kdialog.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kcombobox.h>

class Smb4KCustomOptionsDialog : public KDialog
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KCustomOptionsDialog( Smb4KCustomOptions *options,
                                       QWidget *parent = 0 );
    
    /**
     * Destructor
     */
    ~Smb4KCustomOptionsDialog();
    
  protected Q_SLOTS:
    void slotSetDefaultValues();
    void slotCheckValues();
    void slotOKClicked();
    void slotEnableWOLFeatures( const QString &mac );
    
  private:
    void setupView();
    bool defaultValues();
    Smb4KCustomOptions *m_options;
    QCheckBox *m_remount;
    KIntNumInput *m_smb_port;
#ifdef Q_OS_LINUX
    KIntNumInput *m_fs_port;
    KComboBox *m_write_access;
    KComboBox *m_security_mode;
#endif
    KComboBox *m_protocol_hint;
    KComboBox *m_user_id;
    KComboBox *m_group_id;
    QCheckBox *m_kerberos;
    KLineEdit *m_mac_address;
    QCheckBox *m_send_before_scan;
    QCheckBox *m_send_before_mount;
};


class Smb4KCustomOptionsManagerPrivate
{
  public:
    QList<Smb4KCustomOptions *> options;
};


class Smb4KCustomOptionsManagerStatic
{
  public:
    Smb4KCustomOptionsManager instance;
};

#endif
