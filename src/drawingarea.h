// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QQuickItem>
#include <QTime>
#include <tessellation.rs.h>
#include <QList>
#include <QCursor>
#include <optional>
#include <QPainterPath>

class DrawEvent
{
public:
    DrawEvent()
        : penWidth(10)
        , penColor(Qt::black)
        , fill(false)
    {
    }

    DrawEvent(float width, const QColor& color)
        : penWidth(width)
        , penColor(color)
        , fill(false)
    {
    }

    void lineTo(const QPoint& pos)
    {
        path.lineTo(pos);
    }

    float penWidth;
    QColor penColor;
    QPainterPath path;
    bool fill;
};

/**
 * @class DrawingArea
 * @brief The document view
 * 
 * This @c QQuickItem should be used in a @c Scene only.
 */
class DrawingArea : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penColorChanged)
    Q_PROPERTY(float penWidth READ penWidth WRITE setPenWidth NOTIFY penWidthChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(Tool tool READ tool WRITE setTool NOTIFY toolChanged)

public:
    enum Tool {
        Drawing,
        Rectangle,
        Circle,
    };
    Q_ENUM(Tool);

    explicit DrawingArea(QQuickItem *parent = nullptr);
    ~DrawingArea();

    Q_INVOKABLE void saveSvg(const QUrl &file);

    QColor penColor() const;
    void setPenColor(const QColor &color);

    double penWidth() const;
    void setPenWidth(double penWidth);

    Tool tool() const;
    void setTool(Tool tool);

    bool canUndo() const;

    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;

public Q_SLOTS:
    void undo();

Q_SIGNALS:
    void penColorChanged();
    void penWidthChanged();
    void canUndoChanged();
    void toolChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *, QQuickItem::UpdatePaintNodeData *) override;

private:
    DrawEvent &currentDrawEvent();
    void ensureNewDrawEvent();
    QColor m_penColor;
    double m_penWidth = 4;

    bool m_needUpdate;
    QList<DrawEvent> m_drawEventList;
    QCursor m_drawCursor;
    QPoint m_lastPoint;
    QTime m_drawEventCreationTime;
    int m_eventIndex;
    bool m_drawing;
    int m_lastNumberOfEvent;
    Tool m_tool = Tool::Drawing;
};
