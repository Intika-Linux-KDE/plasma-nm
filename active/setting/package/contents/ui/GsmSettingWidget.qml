/*
    Copyright 2013 Jan Grulich <jgrulich@redhat.com>

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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasmanm 0.1 as PlasmaNm

Item {
    id: gsmSettingWidget;

    property int gsmType: PlasmaNm.Enums.Any;

    height: childrenRect.height;

    PlasmaExtras.Heading {
        id: gsmSettingLabel;

        anchors {
            right: parent.horizontalCenter;
            rightMargin: 12;
        }
        text: i18n("Gsm Settings");
        level: 3;
    }

    Item {
        id: gsmNumber;

        height: childrenRect.height;
        anchors {
            left: parent.left;
            right: parent.right;
            top: gsmSettingLabel.bottom;
            topMargin: 12;
        }

        PlasmaComponents.Label {
            id: gsmNumberLabel;

            anchors {
                right: parent.horizontalCenter;
                rightMargin: 12;
            }
            text: i18n("Number:");
        }

        PlasmaComponents.TextField {
            id: gsmNumberInput;

            width: 200;
            anchors {
                left: parent.horizontalCenter;
            }
            text: "*99#";
        }
    }

    Item {
        id: gsmUsername;

        height: childrenRect.height;
        anchors {
            left: parent.left;
            right: parent.right;
            top: gsmNumber.bottom;
            topMargin: 12;
        }

        PlasmaComponents.Label {
            id: gsmUsernameLabel;

            anchors {
                right: parent.horizontalCenter;
                rightMargin: 12;
            }
            text: i18n("Username:");
        }

        PlasmaComponents.TextField {
            id: gsmUsernameInput;

            width: 200;
            anchors {
                left: parent.horizontalCenter;
            }
        }
    }

    Item {
        id: gsmPassword;

        height: childrenRect.height;
        anchors {
            left: parent.left;
            right: parent.right;
            top: gsmUsername.bottom;
            topMargin: 12;
        }

        PlasmaComponents.Label {
            id: gsmPasswordLabel;

            anchors {
                right: parent.horizontalCenter;
                rightMargin: 12;
            }
            text: i18n("Password:");
        }

        PlasmaComponents.TextField {
            id: gsmPasswordInput;

            width: 200;
            anchors {
                left: parent.horizontalCenter;
            }
            echoMode: TextInput.Password;
        }
    }

    Item {
        id: gsmApn;

        height: childrenRect.height;
        anchors {
            left: parent.left;
            right: parent.right;
            top: gsmPassword.bottom;
            topMargin: 12;
        }

        PlasmaComponents.Label {
            id: gsmApnLabel;

            anchors {
                right: parent.horizontalCenter;
                rightMargin: 12;
            }
            text: i18n("APN:");
        }

        PlasmaComponents.TextField {
            id: gsmApnInput;

            width: 200;
            anchors {
                left: parent.horizontalCenter;
            }
        }
    }

    Item {
        id: gsmType;

        height: childrenRect.height;
        anchors {
            left: parent.left;
            right: parent.right;
            top: gsmApn.bottom;
            topMargin: 12;
        }

        PlasmaComponents.Label {
            id: gsmTypeLabel;

            anchors {
                right: parent.horizontalCenter;
                rightMargin: 12;
            }
            text: i18n("Type:");
        }

        PlasmaComponents.Button {
            id: gsmTypeSelectionCombo;

            width: 200;
            anchors {
                left: parent.horizontalCenter;
            }
            text: i18n("Any");


            MouseArea {
                anchors.fill: parent;

                onClicked: {
                    gsmTypeSelectionDialog.open();
                }
            }
        }
    }

    PlasmaComponents.Dialog {
        id: gsmTypeSelectionDialog;

        visualParent: gsmTypeSelectionCombo;
        content: Item {
            height: 200;
            width: 200;
            ListModel {
                id: gsmTypeModel;

                Component.onCompleted: {
                    append({"name": i18n("Any"), "type": PlasmaNm.Enums.Any});
                    append({"name": i18n("3G Only (UMTS/HSPA)"), "type": PlasmaNm.Enums.Only3G});
                    append({"name": i18n("2G Only (GPRS/EDGE)"), "type": PlasmaNm.Enums.Only2G});
                    append({"name": i18n("Prefer 3G (UMTS/HSPA)"), "type": PlasmaNm.Enums.Prefer3G});
                    append({"name": i18n("Prefer 2G (GPRS/EDGE)"), "type": PlasmaNm.Enums.Prefer2G});
                    append({"name": i18n("Prefer 4G (LTE)"), "type": PlasmaNm.Enums.Prefer4G});
                    append({"name": i18n("4G Only (LTE)"), "type": PlasmaNm.Enums.Only4G});
                }
            }

            ListView {
                id: gsmTypeSelectionView;

                anchors.fill: parent;
                model: gsmTypeModel;
                delegate: PlasmaComponents.ListItem {
                    enabled: true
                    height: 32;

                    PlasmaComponents.Label {
                        anchors {
                            horizontalCenter: parent.horizontalCenter;
                            verticalCenter: parent.verticalCenter;
                        }
                        width: parent.width;
                        text: name;
                        horizontalAlignment: Text.AlignHCenter;
                    }

                    onClicked: {
                        gsmTypeSelectionDialog.close();
                        gsmTypeSelectionCombo.text = name;
                        gsmSettingWidget.gsmType = type;
                    }
                }
            }
        }
    }

    Item {
        id: gsmHomeOnly;

        height: childrenRect.height;

        anchors {
            left: parent.left;
            right: parent.right;
            top: gsmType.bottom;
            topMargin: 12;
        }

        PlasmaComponents.Label {
            id: gsmHomeOnlyLabel;

            anchors {
                right: parent.horizontalCenter;
                rightMargin: 12;
            }
            text: i18n("Allow only home networks:");
        }

        PlasmaComponents.CheckBox {
            id: gsmHomeOnlyCheckbox;

            anchors {
                left: parent.horizontalCenter;
            }
            checked: false;
        }
    }

    Item {
        id: gsmPin;

        height: childrenRect.height;
        anchors {
            left: parent.left;
            right: parent.right;
            top: gsmHomeOnly.bottom;
            topMargin: 12;
        }

        PlasmaComponents.Label {
            id: gsmPinLabel;

            anchors {
                right: parent.horizontalCenter;
                rightMargin: 12;
            }
            text: i18n("Pin:");
        }

        PlasmaComponents.TextField {
            id: gsmPinInput;

            width: 200;
            anchors {
                left: parent.horizontalCenter;
            }
            echoMode: TextInput.Password;
        }
    }

    Item {
        id: gsmShowPasswords;

        height: childrenRect.height;

        anchors {
            left: parent.left;
            right: parent.right;
            top: gsmPin.bottom;
            topMargin: 12;
        }

        PlasmaComponents.Label {
            id: gsmShowPasswordsLabel;

            anchors {
                right: parent.horizontalCenter;
                rightMargin: 12;
            }
            text: i18n("Show passwords:");
        }

        PlasmaComponents.CheckBox {
            id: gsmShowPasswordsCheckbox;

            anchors {
                left: parent.horizontalCenter;
            }
            checked: false;
            onCheckedChanged: {
                if (checked) {
                    gsmPasswordInput.echoMode = TextInput.Normal;
                    gsmPinInput.echoMode = TextInput.Normal;
                } else {
                    gsmPasswordInput.echoMode = TextInput.Password;
                    gsmPinInput.echoMode = TextInput.Password;
                }
            }
        }
    }

    function resetSetting() {
        gsmNumberInput.text = "*99#";
        gsmUsernameInput.text = "";
        gsmPasswordInput.text = "";
        gsmApnInput.text = "";
        gsmTypeSelectionCombo.text = i18n("Any");
        gsmType = PlasmaNm.Enums.Any;
        gsmHomeOnlyCheckbox.checked = false;
        gsmPinInput.text = "";
        gsmShowPasswords.checked = false;
    }

    function loadSetting(settingMap) {
        resetSetting();

        if (settingMap["number"]) {
            gsmNumberInput.text = settingMap["number"];
        }
        if (settingMap["username"]) {
            gsmUsernameInput.text = settingMap["username"];
        }
        if (settingMap["password"]) {
            gsmPasswordInput.text = settingMap["password"];
        }
        if (settingMap["apn"]) {
            gsmApnInput.text = settingMap["apn"];
        }
        if (settingMap["pin"]) {
            gsmPinInput.text = settingMap["pin"];
        }
        if (settingMap["network-type"] >= 0) {
            if (settingMap["network-type"] == 0) {
                gsmTypeSelectionCombo.text = i18n("3G Only (UMTS/HSPA)");
                gsmType = PlasmaNm.Enums.Only3G;
            } else if (settingMap["network-type"] == 1) {
                gsmTypeSelectionCombo.text = i18n("2G Only (GPRS/EDGE)");
                gsmType = PlasmaNm.Enums.Only2G;
            } else if (settingMap["network-type"] == 2) {
                gsmTypeSelectionCombo.text = i18n("Prefer 3G (UMTS/HSPA)");
                gsmType = PlasmaNm.Enums.Prefer3G;
            } else if (settingMap["network-type"] == 3) {
                gsmTypeSelectionCombo.text = i18n("Prefer 2G (GPRS/EDGE)");
                gsmType = PlasmaNm.Enums.Prefer2G;
            } else if (settingMap["network-type"] == 4) {
                gsmTypeSelectionCombo.text = i18n("Prefer 4G (LTE)");
                gsmType = PlasmaNm.Enums.Prefer4G;
            } else if (settingMap["network-type"] == 5) {
                gsmTypeSelectionCombo.text = i18n("4G Only (LTE)");
                gsmType = PlasmaNm.Enums.Only4G;
            }
        }
        if (settingMap["home-only"]) {
            gsmHomeOnlyCheckbox.checked = true;
        }
    }

    function loadSecrets(secretsMap) {
        if (secretsMap["password"]) {
            gsmPasswordInput.text = secretsMap["password"];
        } else if (secretsMap["pin"]) {
            gsmPinInput.text = secretsMap["pin"];
        }
    }

    function getSetting() {
        var settingMap = {};

        if (gsmNumberInput.text) {
            settingMap["number"] = gsmNumberInput.text;
        }
        if (gsmUsernameInput.text) {
            settingMap["username"] = gsmUsernameInput.text;
        }
        if (gsmPasswordInput.text) {
            settingMap["password"] = gsmPasswordInput.text;
        }
        if (gsmApnInput.text) {
            settingMap["apn"] = gsmApnInput.text;
        }
        if (gsmPinInput.text) {
            settingMap["pin"] = gsmPinInput.text;
        }
        if (gsmType == PlasmaNm.Enums.Only3G) {
            settingMap["network-type"] = 0;
        } else if (gsmType == PlasmaNm.Enums.Only2G) {
            settingMap["network-type"] = 1;
        } else if (gsmType == PlasmaNm.Enums.Prefer3G) {
            settingMap["network-type"] = 2;
        } else if (gsmType == PlasmaNm.Enums.Prefer2G) {
            settingMap["network-type"] = 3;
        } else if (gsmType == PlasmaNm.Enums.Prefer4G) {
            settingMap["network-type"] = 4;
        } else if (gsmType == PlasmaNm.Enums.Only4G) {
            settingMap["network-type"] = 5;
        }
        if (gsmHomeOnlyCheckbox.checked == true) {
            settingMap["home-ony"] = true;
        }

        return settingMap;
    }
}