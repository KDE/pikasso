import QtQuick 2.1
import org.kde.kirigami 2.4 as Kirigami
import QtQuick.Controls 2.0 as Controls
import org.kde.drawingarea 1.0
import Qt.labs.platform 1.1

Kirigami.ApplicationWindow {
    id: root

    title: i18n("KirigamiDraw")

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: mainPageComponent
    pageStack.interactive: false // we need to own the drag interaction

    Component {
        id: mainPageComponent

        Kirigami.Page {
            title: i18n("KirigamiDraw")
            padding: 0

            DrawingArea {
                id: drawingarea
                anchors.fill: parent
                antialiasing: true
                layer.enabled: true
                layer.samples: 4
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
                        text: i18n("Undo")
                        icon.name: "edit-undo"
                        enabled: drawingarea.canUndo
                        onTriggered: drawingarea.undo()
                    },
                    Kirigami.Action {
                        icon.name: "draw-brush"
                        checkable: true
                        checked: drawingarea.tool == DrawingArea.Drawing
                        onTriggered: drawingarea.tool = DrawingArea.Drawing
                    },
                    Kirigami.Action {
                        icon.name: "draw-rectangle"
                        checkable: true
                        checked: drawingarea.tool == DrawingArea.Rectangle
                        onTriggered: drawingarea.tool = DrawingArea.Rectangle
                    },
                    Kirigami.Action {
                        icon.name: "draw-ellipse"
                        checkable: true
                        checked: drawingarea.tool == DrawingArea.Circle
                        onTriggered: drawingarea.tool = DrawingArea.Circle
                    },
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
