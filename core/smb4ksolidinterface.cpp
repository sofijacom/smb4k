/***************************************************************************
    smb4ksolidinterface  -  This class provides an interface to KDE's
    Solid framework.
                             -------------------
    begin                : So Sep 14 2008
    copyright            : (C) 2008-2012 by Alexander Reinholdt
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
#include "smb4ksolidinterface.h"

// Qt includes
#include <QtCore/QMap>

// KDE includes
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/acadapter.h>
#include <solid/battery.h>
#include <solid/powermanagement.h>

class Smb4KSolidInterfacePrivate
{
  public:
    Smb4KSolidInterface::ButtonType buttonPressed;
    Smb4KSolidInterface::ConnectionStatus networkStatus;
    QMap<QString,Smb4KSolidInterface::ButtonType> removedDevices;
    int sleepCookie;
};


class Smb4KSolidInterfaceStatic
{
  public:
    Smb4KSolidInterface instance;
};


K_GLOBAL_STATIC( Smb4KSolidInterfaceStatic, p );


Smb4KSolidInterface::Smb4KSolidInterface( QObject *parent )
: QObject( parent ), d( new Smb4KSolidInterfacePrivate )
{
  d->buttonPressed = UnknownButton;
  d->networkStatus = Unknown;
  d->sleepCookie   = 0;
  init();
}


Smb4KSolidInterface::~Smb4KSolidInterface()
{
}


Smb4KSolidInterface *Smb4KSolidInterface::self()
{
  return &p->instance;
}


Smb4KSolidInterface::ConnectionStatus Smb4KSolidInterface::networkStatus() const
{
  return d->networkStatus;
}


void Smb4KSolidInterface::beginSleepSuppression(const QString &reason)
{
  d->sleepCookie = Solid::PowerManagement::beginSuppressingSleep(reason);
}


void Smb4KSolidInterface::endSleepSuppression()
{
  Solid::PowerManagement::stopSuppressingSleep(d->sleepCookie);
}



void Smb4KSolidInterface::init()
{
  // Connect to device notifier.
  connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
           this,                              SLOT(slotDeviceAdded(QString)) );

  connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
           this,                              SLOT(slotDeviceRemoved(QString)) );

  // Get the buttons
  QList<Solid::Device> list_btn = Solid::Device::listFromType( Solid::DeviceInterface::Button, QString() );

  foreach ( const Solid::Device &device_btn, list_btn )
  {
    if ( device_btn.isValid() )
    {
      const Solid::Button *button = device_btn.as<Solid::Button>();
      connect( button, SIGNAL(pressed(Solid::Button::ButtonType,QString)),
               this,   SLOT(slotButtonPressed(Solid::Button::ButtonType,QString)) );

      continue;
    }
    else
    {
      continue;
    }
  }

  // FIXME: Check for hibernation, etc. as well as for the system waking
  // up again.

  // Get the AC adapter(s)
  QList<Solid::Device> list_ac = Solid::Device::listFromType( Solid::DeviceInterface::AcAdapter, QString() );

  foreach ( const Solid::Device &device_ac, list_ac )
  {
    if ( device_ac.isValid() )
    {
      const Solid::AcAdapter *acadapter = device_ac.as<Solid::AcAdapter>();
      connect( acadapter, SIGNAL(plugStateChanged(bool,QString)),
               this,     SLOT(slotAcPlugStateChanged(bool,QString)) );

      // FIXME: Check the current state.

      continue;
    }
    else
    {
      continue;
    }
  }

  // Get the primary batteries
  QList<Solid::Device> list_bat = Solid::Device::listFromType( Solid::DeviceInterface::Battery, QString() );

  foreach ( const Solid::Device &device_bat, list_bat )
  {
    if ( device_bat.isValid() )
    {
      const Solid::Battery *battery = device_bat.as<Solid::Battery>();

      switch ( battery->type() )
      {
        case Solid::Battery::PrimaryBattery:
        {
          connect( battery, SIGNAL(chargeStateChanged(int,QString)),
                   this,    SLOT(slotBatteryChargeStateChanged(int,QString)) );

          connect( battery, SIGNAL(chargePercentChanged(int,QString)),
                   this,    SLOT(slotBatteryChargePercentChanged(int,QString)) );

          // FIXME: Check charge state and value

          break;
        }
        default:
        {
          break;
        }
      }

      continue;
    }
    else
    {
      continue;
    }
  }

  // Check network status and connect to the notifier
  slotNetworkStatusChanged( Solid::Networking::status() );

  connect( Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
           this,                          SLOT(slotNetworkStatusChanged(Solid::Networking::Status)) );
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSolidInterface::slotDeviceAdded( const QString &udi )
{
  // Work around the fact that there is no signal that tells
  // us when the computer woke up. Check the UDIs in the map
  // and emit the wokeUp()
  if ( d->buttonPressed != UnknownButton && d->removedDevices[udi] == d->buttonPressed )
  {
    d->removedDevices.remove( udi );

    if ( d->removedDevices.isEmpty() )
    {
      // Work around: Computer woke up.
      d->buttonPressed = UnknownButton;
      emit wokeUp();
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KSolidInterface::slotDeviceRemoved( const QString &udi )
{
  // Work around the fact that there is no signal that tells us,
  // that the computer has been woken up. Store the removed device(s)
  // and the pressed button to figure out later (in slotDeviceAdded())
  // if the computer became active again.
  if ( d->buttonPressed != UnknownButton )
  {
    d->removedDevices.insert( udi, d->buttonPressed );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSolidInterface::slotButtonPressed(Solid::Button::ButtonType type, const QString &/*udi*/)
{
  switch ( type )
  {
    case Solid::Button::LidButton:
    {
      d->buttonPressed = LidButton;
      break;
    }
    case Solid::Button::SleepButton:
    {
      d->buttonPressed = SleepButton;
      break;
    }
    case Solid::Button::PowerButton:
    {
      d->buttonPressed = PowerButton;
      break;
    }
    default:
    {
      d->buttonPressed = UnknownButton;
      break;
    }
  }

  emit buttonPressed(d->buttonPressed);
}


void Smb4KSolidInterface::slotAcPlugStateChanged( bool state, const QString &/*udi*/ )
{
  if ( state )
  {
    kDebug() << "AC adapter plugged ..." << endl;
  }
  else
  {
    kDebug() << "AC adapter unplugged ..." << endl;
  }
}


void Smb4KSolidInterface::slotBatteryChargeStateChanged( int state, const QString &/*udi*/ )
{
  switch ( state )
  {
    case Solid::Battery::Discharging:
    {
      kDebug() << "Battery is discharging ..." << endl;
      break;
    }
    case Solid::Battery::Charging:
    {
      kDebug() << "Battery is charging ..." << endl;
      break;
    }
    default:
    {
      kDebug() << "Unknown battery state ..." << endl;
      break;
    }
  }
}


void Smb4KSolidInterface::slotBatteryChargePercentChanged( int value, const QString &/*udi*/ )
{
  kDebug() << "Battery charge percent value: " << value << endl;
}


void Smb4KSolidInterface::slotNetworkStatusChanged( Solid::Networking::Status status )
{
  switch ( status )
  {
    case Solid::Networking::Connecting:
    {
      d->networkStatus = Connecting;
      break;
    }
    case Solid::Networking::Connected:
    {
      d->networkStatus = Connected;
      break;
    }
    case Solid::Networking::Disconnecting:
    {
      d->networkStatus = Disconnecting;
      break;
    }
    case Solid::Networking::Unconnected:
    {
      d->networkStatus = Disconnected;
      break;
    }
    case Solid::Networking::Unknown:
    default:
    {
      d->networkStatus = Unknown;
      break;
    }
  }

  emit networkStatusChanged( d->networkStatus );
}

#include "smb4ksolidinterface.moc"
