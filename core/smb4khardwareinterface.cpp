/*
    Provides an interface to the computer's hardware

    SPDX-FileCopyrightText: 2015-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4khardwareinterface.h"
#include "smb4khardwareinterface_p.h"

// system includes
#include <unistd.h>

// Qt includes
#include <QDBusReply>
#include <QDebug>
#include <QNetworkInterface>
#include <QString>
#include <QTimer>

// KDE includes
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/NetworkShare>
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include <KIOCore/KMountPoint>
#endif

Q_GLOBAL_STATIC(Smb4KHardwareInterfaceStatic, p);

Smb4KHardwareInterface::Smb4KHardwareInterface(QObject *parent)
    : QObject(parent)
    , d(new Smb4KHardwareInterfacePrivate)
{
    //
    // Initialize some members
    //
    d->systemOnline = false;
    d->fileDescriptor.setFileDescriptor(-1);

    //
    // Set up the DBUS interface
    //
    d->dbusInterface.reset(
        new QDBusInterface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus(), this));

    if (!d->dbusInterface->isValid()) {
        d->dbusInterface.reset(new QDBusInterface("org.freedesktop.ConsoleKit",
                                                  "/org/freedesktop/ConsoleKit/Manager",
                                                  "org.freedesktop.ConsoleKit.Manager",
                                                  QDBusConnection::systemBus(),
                                                  this));
    }

    //
    // Get the initial list of udis for network shares
    //
    QList<Solid::Device> allDevices = Solid::Device::allDevices();

    for (const Solid::Device &device : qAsConst(allDevices)) {
        if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare)) {
            d->udis << device.udi();
        }
    }

    //
    // Connections
    //
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)), this, SLOT(slotDeviceAdded(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)), this, SLOT(slotDeviceRemoved(QString)));

    //
    // Check the online state
    //
    checkOnlineState(false);

    //
    // Start the timer to continously check the online state
    // and, under FreeBSD, additionally the mounted shares.
    //
    startTimer(1000);
}

Smb4KHardwareInterface::~Smb4KHardwareInterface()
{
}

Smb4KHardwareInterface *Smb4KHardwareInterface::self()
{
    return &p->instance;
}

bool Smb4KHardwareInterface::isOnline() const
{
    return d->systemOnline;
}

void Smb4KHardwareInterface::inhibit()
{
    if (d->fileDescriptor.isValid()) {
        return;
    }

    if (d->dbusInterface->isValid()) {
        QVariantList args;
        args << QStringLiteral("shutdown:sleep");
        args << QStringLiteral("Smb4K");
        args << QStringLiteral("Mounting or unmounting in progress");
        args << QStringLiteral("delay");

        QDBusReply<QDBusUnixFileDescriptor> descriptor = d->dbusInterface->callWithArgumentList(QDBus::Block, QStringLiteral("Inhibit"), args);

        if (descriptor.isValid()) {
            d->fileDescriptor = descriptor.value();
        }
    }
}

void Smb4KHardwareInterface::uninhibit()
{
    if (!d->fileDescriptor.isValid()) {
        return;
    }

    if (d->dbusInterface->isValid()) {
        close(d->fileDescriptor.fileDescriptor());
        d->fileDescriptor.setFileDescriptor(-1);
    }
}

void Smb4KHardwareInterface::checkOnlineState(bool emitSignal)
{
    bool online = false;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : qAsConst(interfaces)) {
        if (interface.isValid() && interface.type() != QNetworkInterface::Loopback && interface.flags() & QNetworkInterface::IsRunning) {
            online = true;
            break;
        }
    }

    if (online != d->systemOnline) {
        d->systemOnline = online;

        if (emitSignal) {
            emit onlineStateChanged(d->systemOnline);
        }
    }
}

void Smb4KHardwareInterface::timerEvent(QTimerEvent * /*e*/)
{
    //
    // Check for network connectivity
    //
    checkOnlineState();

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    //
    // Using FreeBSD 11 with KF 5.27, Solid is not able to detect mounted shares.
    // Thus, we check here whether shares have been mounted or unmounted.
    // This is a hack and should be removed as soon as possible.
    //
    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded | KMountPoint::NeedMountOptions);
    QStringList mountPointList, alreadyMounted;

    for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : mountPoints) {
        if (mountPoint->mountType() == "smbfs") {
            mountPointList.append(mountPoint->mountPoint());
        }
    }

    QMutableStringListIterator it(mountPointList);

    while (it.hasNext()) {
        QString mp = it.next();
        int index = -1;

        if ((index = d->mountPoints.indexOf(mp)) != -1) {
            d->mountPoints.removeAt(index);
            alreadyMounted.append(mp);
            it.remove();
        }
    }

    if (!d->mountPoints.isEmpty()) {
        emit networkShareRemoved();
    }

    if (!mountPointList.isEmpty()) {
        emit networkShareAdded();
    }

    d->mountPoints.clear();
    d->mountPoints.append(alreadyMounted);
    d->mountPoints.append(mountPointList);

#endif
}

void Smb4KHardwareInterface::slotDeviceAdded(const QString &udi)
{
    //
    // Create a device from the UDI
    //
    Solid::Device device(udi);

    //
    // Check if the device is a network device and emit the networkShareAdded()
    // signal if it is.
    //
    if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare)) {
        d->udis << udi;
        emit networkShareAdded();
    }
}

void Smb4KHardwareInterface::slotDeviceRemoved(const QString &udi)
{
    //
    // Create a device from the UDI
    //
    Solid::Device device(udi);

    //
    // Check if the device is a network device and emit the networkShareRemoved()
    // signal if it is.
    //
    // NOTE:For some reason, the device has no valid type. Thus, we need the check
    // in the second part of the if statement for now.
    //
    if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare) || d->udis.contains(udi)) {
        emit networkShareRemoved();
        d->udis.removeOne(udi);
    }
}
