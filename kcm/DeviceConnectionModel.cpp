/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti                                *
 *   dantti12@gmail.com                                                    *
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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "DeviceConnectionModel.h"

#include <uiutils.h>

#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ActiveConnection>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>
#include <QStringBuilder>

#include <KDebug>
#include <KLocale>
#include <KDateTime>
#include <KIcon>

using namespace NetworkManager;

DeviceConnectionModel::DeviceConnectionModel(QObject *parent) :
    QStandardItemModel(parent)
{
    connect(NetworkManager::notifier(), SIGNAL(serviceAppeared()),
            this, SLOT(initConnections()));
    connect(NetworkManager::notifier(), SIGNAL(serviceDisappeared()),
            this, SLOT(removeConnections()));
    connect(NetworkManager::settingsNotifier(), SIGNAL(connectionAdded(QString)),
            this, SLOT(connectionAdded(QString)));
    connect(NetworkManager::settingsNotifier(), SIGNAL(connectionRemoved(QString)),
            this, SLOT(connectionRemoved(QString)));

    connect(NetworkManager::notifier(), SIGNAL(deviceAdded(QString)),
            this, SLOT(deviceAdded(QString)));
    connect(NetworkManager::notifier(), SIGNAL(deviceRemoved(QString)),
            this, SLOT(deviceRemoved(QString)));

    connect(NetworkManager::notifier(), SIGNAL(activeConnectionAdded(QString)),
            this, SLOT(activeConnectionAdded(QString)));
    connect(NetworkManager::notifier(), SIGNAL(activeConnectionRemoved(QString)),
            this, SLOT(activeConnectionRemoved(QString)));

    QStandardItem *parentItem = new QStandardItem(i18n("Network devices"));
    parentItem->setData(true, RoleIsDeviceParent);
    parentItem->setSelectable(false);
    QFont font = parentItem->font();
    font.setPointSizeF(font.pointSizeF() * 1.5);
    parentItem->setFont(font);
    parentItem->setFlags(Qt::NoItemFlags);
    appendRow(parentItem);
}

void DeviceConnectionModel::init()
{
    foreach (const Device::Ptr &device, NetworkManager::networkInterfaces()) {
        addDevice(device);
    }
    initConnections();
}

Qt::ItemFlags DeviceConnectionModel::flags(const QModelIndex &index) const
{
    QStandardItem *stdItem = itemFromIndex(index);
    if (stdItem->flags() == Qt::NoItemFlags) {
        return Qt::NoItemFlags;
    } else if (stdItem && stdItem->isCheckable() && stdItem->checkState() == Qt::Unchecked) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void DeviceConnectionModel::initConnections()
{
    foreach (const Connection::Ptr &connection, NetworkManager::listConnections()) {
        addConnection(connection);
    }
}

void DeviceConnectionModel::removeConnections()
{
    int i = 0;
    while (i < rowCount()) {
        QStandardItem *stdRootItem = item(i);
        if (stdRootItem->data(RoleIsConnectionParent).toBool()) {
            stdRootItem->removeRows(0, stdRootItem->rowCount());
            removeRow(i);
            ++i;
        } else {
            ++i;
        }
    }
}

void DeviceConnectionModel::deviceAdded(const QString &uni)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(uni);
    if (device) {
        addDevice(device);
    }
}

void DeviceConnectionModel::deviceChanged()
{
    NetworkManager::Device *caller = qobject_cast<NetworkManager::Device*>(sender());
    if (caller) {
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(caller->uni());
        if (device) {
            QStandardItem *stdItem = findDeviceItem(device->uni());
            if (!stdItem) {
                kWarning() << "Device not found" << device->uni();
                return;
            }
            changeDevice(stdItem, device);
        }
    }
}

void DeviceConnectionModel::deviceRemoved(const QString &uni)
{
    QStandardItem *stdItem = findDeviceItem(uni);
    if (stdItem) {
        removeRow(stdItem->row());
    }
}

void DeviceConnectionModel::addDevice(const NetworkManager::Device::Ptr &device)
{
    QStandardItem *stdItem = findDeviceItem(device->uni());
    if (stdItem) {
        return;
    }

    connect(device.data(), SIGNAL(stateChanged(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)),
            this, SLOT(deviceChanged()));

    stdItem = new QStandardItem;
    stdItem->setData(true, RoleIsDevice);
    stdItem->setData(device->uni(), RoleDeviceUNI);
    changeDevice(stdItem, device);
    appendRow(stdItem);
}

void DeviceConnectionModel::changeDevice(QStandardItem *stdItem, const NetworkManager::Device::Ptr &device)
{
    stdItem->setIcon(KIcon(UiUtils::iconName(device)));

    QString connectionName;
    NetworkManager::ActiveConnection::Ptr activeConnection = device->activeConnection();
    if (activeConnection) {
        connectionName = activeConnection->connection()->name();
    }
    stdItem->setData(UiUtils::connectionStateToString(device->state(), connectionName), RoleState);
    stdItem->setText(UiUtils::prettyInterfaceName(device->type(), device->interfaceName()));
}

void DeviceConnectionModel::connectionAdded(const QString &path)
{
    Connection::Ptr connection = NetworkManager::findConnection(path);
    if (connection) {
        addConnection(connection);
    }
}

void DeviceConnectionModel::connectionUpdated()
{
    Connection *caller = qobject_cast<Connection*>(sender());
    if (caller) {
        Connection::Ptr connection = findConnection(caller->path());
        if (connection) {
            QStandardItem *stdItem = findConnectionItem(connection->path(), RoleConnectionPath);
            if (!stdItem) {
                kWarning() << "Connection not found" << connection->path();
                return;
            }
            stdItem->setText(connection->name());
        }
    }
}

void DeviceConnectionModel::connectionRemoved(const QString &path)
{
    QStandardItem *stdItem = findConnectionItem(path, RoleConnectionPath);
    if (stdItem) {
        QStandardItem *stdParentItem = stdItem->parent();
        removeRow(stdItem->row(), stdParentItem->index());
        if (stdParentItem->rowCount() == 0) {
            removeRow(stdParentItem->row(), stdParentItem->parent()->index());
        }
    }
}

void DeviceConnectionModel::activeConnectionAdded(const QString &path)
{
    ActiveConnection::Ptr activeConnection = NetworkManager::findActiveConnection(path);
    if (!activeConnection) {
        return;
    }

    Connection::Ptr connection = activeConnection->connection();
    if (connection) {
        QStandardItem *stdItem = findConnectionItem(connection->path(), RoleConnectionPath);
        if (!stdItem) {
            kWarning() << "Connection not found" << connection->path();
            return;
        }
        changeConnectionActive(stdItem, activeConnection->path());
    }
}

void DeviceConnectionModel::activeConnectionRemoved(const QString &path)
{
    QStandardItem *stdItem = findConnectionItem(path, RoleConnectionActivePath);
    if (stdItem) {
        changeConnectionActive(stdItem);
    }
}

void DeviceConnectionModel::addConnection(const Connection::Ptr &connection)
{
    QStandardItem *stdItem = findConnectionItem(connection->path(), RoleConnectionPath);
    if (stdItem) {
        return;
    }

    ConnectionSettings::ConnectionType type = connection->settings()->connectionType();
    QStandardItem *parentItem = findOrCreateConnectionType(type);
    connect(connection.data(), SIGNAL(updated()),
            this, SLOT(connectionUpdated()));

    stdItem = new QStandardItem;
    stdItem->setData(true, RoleIsConnection);
    stdItem->setData(connection->path(), RoleConnectionPath);
    stdItem->setIcon(KIcon("network-disconnect"));
    stdItem->setText(connection->name());
    parentItem->appendRow(stdItem);

    foreach (const ActiveConnection::Ptr &activeConnection, NetworkManager::activeConnections()) {
        if (activeConnection->connection() == connection) {
            changeConnectionActive(stdItem, activeConnection->path());
            break;
        }
    }
}

void DeviceConnectionModel::changeConnectionActive(QStandardItem *stdItem, const QString &activePath)
{
    kDebug() << stdItem->text() << activePath;
    QVariant previousActive = stdItem->data(RoleConnectionActivePath);
    if (previousActive.isNull() || previousActive.toString() != activePath) {
        if (activePath.isNull()) {
            stdItem->setIcon(KIcon("network-disconnect"));
        } else {
            stdItem->setIcon(KIcon("network-connect"));
        }
        stdItem->setData(activePath, RoleConnectionActivePath);

        uint count = stdItem->parent()->data(RoleIsConnectionCategoryActiveCount).toUInt();
        if (previousActive.isNull()) {
            stdItem->parent()->setData(++count, RoleIsConnectionCategoryActiveCount);
            if (count == 1) {
                emit parentAdded(stdItem->parent()->index());
            }
        } else {
            if (activePath.isNull()) {
                stdItem->parent()->setData(--count, RoleIsConnectionCategoryActiveCount);
            } else {
                stdItem->parent()->setData(++count, RoleIsConnectionCategoryActiveCount);
            }
        }
    }
}

QStandardItem *DeviceConnectionModel::findDeviceItem(const QString &uni)
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *stdItem = item(i);
        if (stdItem->data(RoleIsDevice).toBool() == true &&
                stdItem->data(RoleDeviceUNI).toString() == uni) {
            return stdItem;
        }
    }

    return 0;
}

QStandardItem *DeviceConnectionModel::findConnectionItem(const QString &path, DeviceRoles role)
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *stdRootItem = item(i);

        if (stdRootItem->data(RoleIsConnectionParent).toBool()) {
            for (int i = 0; i < stdRootItem->rowCount(); ++i) {
                QStandardItem *stdParentItem = stdRootItem->child(i);

                for (int i = 0; i < stdParentItem->rowCount(); ++i) {
                    QStandardItem *stdItem = stdParentItem->child(i);
                    if (stdItem->data(role).toString() == path) {
                        return stdItem;
                    }
                }
            }
            break;
        }
    }

    return 0;
}

QStandardItem *DeviceConnectionModel::findOrCreateConnectionType(ConnectionSettings::ConnectionType type)
{
    QStandardItem *parentItem = 0;
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *stdItem = item(i);
        if (stdItem->data(RoleIsConnectionParent).toBool()) {
            parentItem = stdItem;
            break;
        }
    }

    if (!parentItem) {
        parentItem = new QStandardItem(i18n("Network connections"));
        parentItem->setData(true, RoleIsConnectionParent);
        parentItem->setSelectable(false);
        QFont font = parentItem->font();
        font.setPointSizeF(font.pointSizeF() * 1.5);
        parentItem->setFont(font);
        parentItem->setFlags(Qt::NoItemFlags);
        appendRow(parentItem);
        emit parentAdded(parentItem->index());
    }

    for (int i = 0; i < parentItem->rowCount(); ++i) {
        QStandardItem *stdItem = parentItem->child(i);
        uint itemType = stdItem->data(RoleIsConnectionCategory).toUInt();
        if (itemType == type ||
                (type == ConnectionSettings::Gsm && itemType == ConnectionSettings::Cdma) ||
                (type == ConnectionSettings::Cdma && itemType == ConnectionSettings::Gsm)) {
            return stdItem;
        }
    }

    QStandardItem *ret = new QStandardItem();
    QString text;
    KIcon icon;
    switch (type) {
    case ConnectionSettings::Adsl:
        text = i18n("ADSL");
        icon = KIcon("modem");
        break;
    case ConnectionSettings::Pppoe:
        text = i18n("DSL");
        icon = KIcon("modem");
        break;
    case ConnectionSettings::Bluetooth:
        text = i18n("Bluetooth");
        icon = KIcon("preferences-system-bluetooth");
        break;
    case ConnectionSettings::Bond:
        text = i18n("Bond");
        break;
    case ConnectionSettings::Bridge:
        text = i18n("Bridge");
        break;
    case ConnectionSettings::Gsm:
    case ConnectionSettings::Cdma:
        text = i18n("Mobile broadband");
        icon = KIcon("phone");
        break;
    case ConnectionSettings::Infiniband:
        text = i18n("Infiniband");
        break;
    case ConnectionSettings::OLPCMesh:
        text = i18n("Olpc mesh");
        break;
    case ConnectionSettings::Vlan:
        text = i18n("VLAN");
        break;
    case ConnectionSettings::Vpn:
        text = i18n("VPN");
        icon = KIcon("secure-card");
        break;
    case ConnectionSettings::Wimax:
        text = i18n("WiMAX");
        icon = KIcon("network-wireless");
        break;
    case ConnectionSettings::Wired:
        text = i18n("Wired");
        icon = KIcon("network-wired");
        break;
    case ConnectionSettings::Wireless:
        text = i18n("Wireless");
        icon = KIcon("network-wireless");
        break;
    default:
        text = i18n("Unknown connection type");
        break;
    }

    ret->setText(text);
    ret->setIcon(icon);
    ret->setData(type, RoleIsConnectionCategory);
    ret->setData(0, RoleIsConnectionCategoryActiveCount);
    parentItem->appendRow(ret);

    return ret;
}
