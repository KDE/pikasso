/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2020  Carl Schwan <carl@carlschwan.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

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
    {
    }

    DrawEvent(float width, const QColor& color)
        : penWidth(width)
        , penColor(color)
    {
    }

    void lineTo(const QPoint& pos)
    {
        path.lineTo(pos);
    }

    float penWidth;
    QColor penColor;
    QPainterPath path;
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
public:
    explicit DrawingArea(QQuickItem *parent = nullptr);
    ~DrawingArea();

    QColor penColor() const;
    void setPenColor(const QColor &color);

    double penWidth() const;
    void setPenWidth(double penWidth);

    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;

Q_SIGNALS:
    void penColorChanged();
    void penWidthChanged();

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
};
