import QtQuick 2.1
import org.kde.kirigami 2.4 as Kirigami
import QtQuick.Controls 2.0 as Controls
import org.kde.drawingarea 1.0
import Qt.labs.platform 1.1

Kirigami.ApplicationWindow {
    id: root

    title: i18n("KirigamiDraw")

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("KirigamiDraw")
        titleIcon: "applications-graphics"
        actions: [
            Kirigami.Action {
                text: i18n("View")
                icon.name: "view-list-icons"
                Kirigami.Action {
                    text: i18n("View Action 1")
                    onTriggered: showPassiveNotification(i18n("View Action 1 clicked"))
                }
                Kirigami.Action {
                    text: i18n("View Action 2")
                    onTriggered: showPassiveNotification(i18n("View Action 2 clicked"))
                }
            },
            Kirigami.Action {
                text: i18n("Action 1")
                onTriggered: showPassiveNotification(i18n("Action 1 clicked"))
            },
            Kirigami.Action {
                text: i18n("Action 2")
                onTriggered: showPassiveNotification(i18n("Action 2 clicked"))
            }
        ]
    }

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: mainPageComponent

    Component {
        id: mainPageComponent

        Kirigami.Page {
            title: i18n("KirigamiDraw")
            padding: 0

            DrawingArea {
                id: drawingarea
                anchors.fill: parent
            }

            ColorDialog {
                id: colorDialog
                currentColor: drawingarea.penColor
                onAccepted: drawingarea.penColor = color
            }

            FileDialog {
                id: fileDialog
                fileMode: FileDialog.SaveFile
                onAccepted: {
                    drawingarea.saveSvg(fileDialog.file)
                }
            }

            actions {
                contextualActions: [
                    Kirigami.Action {
                        text: i18n("Color")
                        icon.name: "color-management"
                        onTriggered: colorDialog.open()
                    },
                    Kirigami.Action {
                        text: i18n("Width")
                        displayComponent: Controls.SpinBox {
                            from: 1
                            to: 100
                            onValueChanged: drawingarea.penWidth = value
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Save as")
                        icon.name: "document-save-as"
                        onTriggered: fileDialog.open()
                    }
                ]
            }
        }
    }
}
