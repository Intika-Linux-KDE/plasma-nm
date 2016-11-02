/*
    Copyright 2016 Jan Grulich <jgrulich@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kcm.h"

#include "debug.h"
#include "connectioneditordialog.h"
#include "mobileconnectionwizard.h"

// KDE
#include <KPluginFactory>
#include <KSharedConfig>
#include <kdeclarative/kdeclarative.h>

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessDevice>

// Qt
#include <QMenu>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QVBoxLayout>

K_PLUGIN_FACTORY(KCMNetworkConfigurationFactory, registerPlugin<KCMNetworkmanagement>();)

KCMNetworkmanagement::KCMNetworkmanagement(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_handler(new Handler(this))
    , m_tabWidget(nullptr)
    , m_ui(new Ui::KCMForm)
    , m_quickView(nullptr)
{
    QWidget *mainWidget = new QWidget(this);
    m_ui->setupUi(mainWidget);

    m_quickView = new QQuickView(0);
    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(m_quickView->engine());
    kdeclarative.setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));
    kdeclarative.setupBindings();

    QWidget *widget = QWidget::createWindowContainer(m_quickView, this);
    QVBoxLayout *layout = new QVBoxLayout(m_ui->connectionView);
    layout->addWidget(widget);

    m_quickView->setResizeMode(QQuickView::SizeRootObjectToView);
    m_quickView->setSource(QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kcm_networkmanagement/qml/main.qml"))));

    QObject *rootItem = m_quickView->rootObject();
    connect(rootItem, SIGNAL(selectedConnectionChanged(QString)), this, SLOT(onSelectedConnectionChanged(QString)));
    connect(rootItem, SIGNAL(requestCreateConnection(int,QString,QString,bool)), this, SLOT(onRequestCreateConnection(int,QString,QString,bool)));
    
    QVBoxLayout *l = new QVBoxLayout(this);
    l->addWidget(mainWidget);

    setButtons(Button::Apply);
}

KCMNetworkmanagement::~KCMNetworkmanagement()
{
}

void KCMNetworkmanagement::defaults()
{
    KCModule::defaults();
}

void KCMNetworkmanagement::load()
{
    // If there is no loaded connection do nothing
    if (m_currentConnectionPath.isEmpty()) {
        return;
    }
    
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(m_currentConnectionPath);
    if (connection) {
        NetworkManager::ConnectionSettings::Ptr connectionSettings = connection->settings();
        // Re-load the connection again to load stored values
        if (m_tabWidget) {
            m_tabWidget->setConnection(connectionSettings);
        }
    }
    
    KCModule::load();
}

void KCMNetworkmanagement::save()
{
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(m_currentConnectionPath);
    
    if (connection) {
        m_handler->updateConnection(connection, m_tabWidget->setting());
    }
    
    KCModule::save();
}

void KCMNetworkmanagement::onRequestCreateConnection(int connectionType, const QString &vpnType, const QString &specificType, bool shared)
{
    NetworkManager::ConnectionSettings::ConnectionType type = static_cast<NetworkManager::ConnectionSettings::ConnectionType>(connectionType);
//     const QString vpnType = action->property("type").toString();
//     const QString vpnSubType = action->property("subtype").toString();

//     if (type == NetworkManager::ConnectionSettings::Vpn && vpnType == "imported") {
//         importVpn();
/*    } else */if (type == NetworkManager::ConnectionSettings::Gsm) { // launch the mobile broadband wizard, both gsm/cdma
#if WITH_MODEMMANAGER_SUPPORT
        QPointer<MobileConnectionWizard> wizard = new MobileConnectionWizard(NetworkManager::ConnectionSettings::Unknown, this);
        connect(wizard.data(), &MobileConnectionWizard::accepted,
                [wizard, this] () {
                    if (wizard->getError() == MobileProviders::Success) {
                        qCDebug(PLASMA_NM) << "Mobile broadband wizard finished:" << wizard->type() << wizard->args();

                        if (wizard->args().count() == 2) {
                            QVariantMap tmp = qdbus_cast<QVariantMap>(wizard->args().value(1));

                            NetworkManager::ConnectionSettings::Ptr connectionSettings;
                            connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(wizard->type()));
                            connectionSettings->setId(wizard->args().value(0).toString());
                            if (wizard->type() == NetworkManager::ConnectionSettings::Gsm) {
                                NetworkManager::GsmSetting::Ptr gsmSetting = connectionSettings->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                                gsmSetting->fromMap(tmp);
                                gsmSetting->setPasswordFlags(NetworkManager::Setting::NotRequired);
                                gsmSetting->setPinFlags(NetworkManager::Setting::NotRequired);
                            } else if (wizard->type() == NetworkManager::ConnectionSettings::Cdma) {
                                connectionSettings->setting(NetworkManager::Setting::Cdma)->fromMap(tmp);
                            } else {
                                qCWarning(PLASMA_NM) << Q_FUNC_INFO << "Unhandled setting type";
                            }
                            // Generate new UUID
                            connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
                            addConnection(connectionSettings);
                        } else {
                            qCWarning(PLASMA_NM) << Q_FUNC_INFO << "Unexpected number of args to parse";
                        }
                    }
                });
        connect(wizard.data(), &MobileConnectionWizard::finished,
                [wizard] () {
                    if (wizard) {
                        wizard->deleteLater();
                    }
                });
        wizard->setModal(true);
        wizard->show();
#endif
    } else {
        NetworkManager::ConnectionSettings::Ptr connectionSettings;
        connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(type));

        if (type == NetworkManager::ConnectionSettings::Vpn) {
            NetworkManager::VpnSetting::Ptr vpnSetting = connectionSettings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
            vpnSetting->setServiceType(vpnType);
            // Set VPN subtype in case of Openconnect to add support for juniper
            if (vpnType == QLatin1String("org.freedesktop.NetworkManager.openconnect")) {
                NMStringMap data = vpnSetting->data();
                data.insert(QLatin1String("protocol"), specificType);
                vpnSetting->setData(data);
            }
        }

        if (type == NetworkManager::ConnectionSettings::Wired || type == NetworkManager::ConnectionSettings::Wireless) {
            if (shared) {
                if (type == NetworkManager::ConnectionSettings::Wireless) {
                    NetworkManager::WirelessSetting::Ptr wifiSetting = connectionSettings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
                    wifiSetting->setMode(NetworkManager::WirelessSetting::Adhoc);
                    wifiSetting->setSsid(i18n("my_shared_connection").toUtf8());

                    Q_FOREACH (const NetworkManager::Device::Ptr & device, NetworkManager::networkInterfaces()) {
                        if (device->type() == NetworkManager::Device::Wifi) {
                            NetworkManager::WirelessDevice::Ptr wifiDev = device.objectCast<NetworkManager::WirelessDevice>();
                            if (wifiDev) {
                                if (wifiDev->wirelessCapabilities().testFlag(NetworkManager::WirelessDevice::ApCap)) {
                                    wifiSetting->setMode(NetworkManager::WirelessSetting::Ap);
                                    wifiSetting->setMacAddress(NetworkManager::macAddressFromString(wifiDev->hardwareAddress()));
                                }
                            }
                        }
                    }
                }

                NetworkManager::Ipv4Setting::Ptr ipv4Setting = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
                ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Shared);
                connectionSettings->setAutoconnect(false);
            }
        }
        // Generate new UUID
        connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
        addConnection(connectionSettings);
    }
}

void KCMNetworkmanagement::onSelectedConnectionChanged(const QString &connectionPath)
{
    m_currentConnectionPath = connectionPath;
    
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(m_currentConnectionPath);
    if (connection) {
        NetworkManager::ConnectionSettings::Ptr connectionSettings = connection->settings();
        loadConnectionSettings(connectionSettings);
    }
}

void KCMNetworkmanagement::addConnection(const NetworkManager::ConnectionSettings::Ptr &connectionSettings)
{
    // Reset selected connections
    m_currentConnectionPath.clear();
    QObject *rootItem = m_quickView->rootObject();
    QMetaObject::invokeMethod(rootItem, "deselectConnections");
    if (m_tabWidget) {
        delete m_ui->connectionConfiguration->layout();
        delete m_tabWidget;
        m_tabWidget = nullptr;
    }
    
    QPointer<ConnectionEditorDialog> editor = new ConnectionEditorDialog(connectionSettings);
    connect(editor.data(), &ConnectionEditorDialog::accepted,
            [connectionSettings, editor, this] () {
                m_handler->addConnection(editor->setting());
            });
    connect(editor.data(), &ConnectionEditorDialog::finished,
            [editor] () {
                if (editor) {
                    editor->deleteLater();
                }
            });
    editor->setModal(true);
    editor->show();
}

void KCMNetworkmanagement::loadConnectionSettings(const NetworkManager::ConnectionSettings::Ptr& connectionSettings)
{
    if (m_tabWidget) {
        m_tabWidget->setConnection(connectionSettings);
    } else {
        m_tabWidget = new ConnectionEditorTabWidget(connectionSettings);
        QVBoxLayout *layout = new QVBoxLayout(m_ui->connectionConfiguration);
        layout->addWidget(m_tabWidget);
    }

    Q_EMIT changed(true);
}


#include "kcm.moc"

