// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "drawingarea.h"

#include <QSGNode>
#include <QPointF>
#include <QSGFlatColorMaterial>

DrawingArea::DrawingArea(QQuickItem *parent)
    : QQuickItem(parent)
{
    setObjectName("DrawingArea");
    setFlag(QQuickItem::ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton);
}

DrawingArea::~DrawingArea()
{}

QColor DrawingArea::penColor() const
{
    return m_penColor;
}

void DrawingArea::setPenColor(const QColor &penColor)
{
    if (m_penColor == penColor) {
        return;
    }
    m_penColor = penColor;
    Q_EMIT penColorChanged();
}


double DrawingArea::penWidth() const
{
    return m_penWidth;
}

void DrawingArea::setPenWidth(double penWidth)
{
    if (m_penWidth == penWidth) {
        return;
    }
    m_penWidth = penWidth;
    Q_EMIT penWidthChanged();
}

DrawEvent &DrawingArea::currentDrawEvent()
{
    QTime currentTime = QTime::currentTime();

    if (m_drawEventCreationTime.isNull() || (m_drawEventCreationTime.msecsTo(currentTime) > 1000)) {
        m_drawEventCreationTime = currentTime;
        DrawEvent drawEvent(m_penWidth, m_penColor);
        drawEvent.path.moveTo(m_drawEventList.last().path.currentPosition());
        m_drawEventList.append(drawEvent);
        ++m_eventIndex;
    }

    return m_drawEventList.last();
}

void DrawingArea::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    m_lastPoint = event->pos();
    m_drawing = true;
    setCursor(m_drawCursor);
    m_drawEventCreationTime = QTime::currentTime();
    DrawEvent drawEvent(m_penWidth, m_penColor);
    drawEvent.path.moveTo(event->pos());
    m_drawEventList.append(drawEvent);

    m_eventIndex = m_drawEventList.count() - 1;
}

void DrawingArea::mouseMoveEvent(QMouseEvent* e)
{
    if (m_drawing || !(e->modifiers() & Qt::CTRL)) {
        setCursor(m_drawCursor);
    }
    if ((e->buttons() & Qt::LeftButton)) {
        QPoint currentPos = e->pos();
        currentDrawEvent().lineTo(currentPos);
        m_drawEventCreationTime = QTime::currentTime();
        update();
    }
}

void DrawingArea::mouseReleaseEvent(QMouseEvent* e)
{
    if ((e->button() == Qt::LeftButton) && m_drawing) {
        QPoint currentPos = e->pos();
        currentDrawEvent().lineTo(currentPos);
        m_drawing = false;
        update();
    }
}


void DrawingArea::ensureNewDrawEvent()
{
    m_drawEventCreationTime = QTime();
}

rust::Box<LyonBuilder> painterPathToBuilder(const QPainterPath &path)
{
    auto lyonBuilder = new_builder();
    for (int i = 0; i < path.elementCount(); i++) {
        const auto element = path.elementAt(i);
        if (element.isLineTo()) {
            lyonBuilder->line_to(LyonPoint { static_cast<float>(element.x), static_cast<float>(element.y) });
        } else if (element.isMoveTo()) {
            lyonBuilder->move_to(LyonPoint { static_cast<float>(element.x), static_cast<float>(element.y) });
        } else if (element.type ==  QPainterPath::ElementType::CurveToElement) {
            // Cubic is encoded with ctrl1 -> CurveToElement, ctrl2 -> CurveToDataElement and to -> CurveToDataElement
            Q_ASSERT(i + 2 < path.elementCount() && "CurveToElement doesn't have data");
            const auto ctrl1 = path.elementAt(i);
            const auto ctrl2 = path.elementAt(i + 1);
            const auto to = path.elementAt(i + 2);
            lyonBuilder->cubic_bezier_to(
                LyonPoint { static_cast<float>(ctrl1.x), static_cast<float>(ctrl1.y) },
                LyonPoint { static_cast<float>(ctrl2.x), static_cast<float>(ctrl2.y) },
                LyonPoint { static_cast<float>(to.x), static_cast<float>(to.y) }
            );
            i += 2; // we analysed tree elements instead of just one
        } else {
            Q_ASSERT(false && "Should not happen");
        }
    }

    return lyonBuilder;
}

QSGNode *DrawingArea::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData* updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)
    QSGGeometryNode *root = nullptr;
    QSGGeometry *geometry = nullptr;

    if(!oldNode) {
        root = new QSGGeometryNode;
        const int vertexCount = 4;
        const int indexCount = 2 * 3;
        root = new QSGGeometryNode;
        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), vertexCount, indexCount);
        geometry->setDrawingMode(GL_TRIANGLES);

        geometry->vertexDataAsPoint2D()[0].set(0, 0);
        geometry->vertexDataAsPoint2D()[1].set(width(), 0);
        geometry->vertexDataAsPoint2D()[2].set(width(), height());
        geometry->vertexDataAsPoint2D()[3].set(0, height());

        quint16 *indices = geometry->indexDataAsUShort();
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 0;
        indices[4] = 3;
        indices[5] = 2;

        root->setGeometry(geometry);
        root->setFlag(QSGNode::OwnsGeometry);
        root->setFlag(QSGNode::OwnsMaterial);

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
        material->setColor(QColor("white"));
        root->setMaterial(material);

        m_lastNumberOfEvent = m_drawEventList.count();
        // crate drawing nodes
        for (const auto &drawEvent : qAsConst(m_drawEventList)) {
            auto node = new QSGGeometryNode;
            auto builder = painterPathToBuilder(drawEvent.path);
            const auto lyonGeometry = build_stroke(std::move(builder), drawEvent.penWidth);
            QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
                    lyonGeometry.vertices.size(), lyonGeometry.indices.size());

            geometry->setIndexDataPattern(QSGGeometry::StaticPattern);
            geometry->setDrawingMode(GL_TRIANGLES);
            node->setGeometry(geometry);
            node->setFlag(QSGNode::OwnsGeometry);

            QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
            material->setColor(drawEvent.penColor);
            node->setMaterial(material);
            node->setFlag(QSGNode::OwnsMaterial);
            root->appendChildNode(node);

            QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
            std::size_t i = 0;
            for (const auto &vertice: lyonGeometry.vertices) {
                points[i].set(vertice.x, vertice.y);
                i++;
            }

            quint16* indices = geometry->indexDataAsUShort();
            i = 0;
            for (const auto indice: lyonGeometry.indices) {
                indices[i] = indice;
                i++;
            }

            node->markDirty(QSGNode::DirtyGeometry);
        }
    } else {
        root = static_cast<QSGGeometryNode *>(oldNode);
        if (m_needUpdate) {
            auto geometry = root->geometry();
            geometry->vertexDataAsPoint2D()[0].set(0, 0);
            geometry->vertexDataAsPoint2D()[1].set(width(), 0);
            geometry->vertexDataAsPoint2D()[2].set(width(), height());
            geometry->vertexDataAsPoint2D()[3].set(0, height());
        }

        // update already existing child nodes
        for (size_t eventIndex = 0; eventIndex < m_lastNumberOfEvent; eventIndex++) {
            auto node = static_cast<QSGGeometryNode *>(root->childAtIndex(eventIndex));
            const auto &drawEvent = m_drawEventList[eventIndex];

            auto builder = painterPathToBuilder(drawEvent.path);
            const auto lyonGeometry = build_stroke(std::move(builder), drawEvent.penWidth);
            auto geometry = node->geometry();
            geometry->allocate(lyonGeometry.vertices.size(), lyonGeometry.indices.size());

            QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
            std::size_t i = 0;
            for (const auto &vertice: lyonGeometry.vertices) {
                points[i].set(vertice.x, vertice.y);
                i++;
            }

            quint16* indices = geometry->indexDataAsUShort();
            i = 0;
            for (const auto indice: lyonGeometry.indices) {
                indices[i] = indice;
                i++;
            }

            node->markDirty(QSGNode::DirtyGeometry);
        }

        for (size_t eventIndex = m_lastNumberOfEvent; eventIndex < m_drawEventList.count(); eventIndex++) {
            m_lastNumberOfEvent = m_drawEventList.count();
            const auto drawEvent = m_drawEventList[eventIndex];
            auto node = new QSGGeometryNode;
            auto builder = painterPathToBuilder(drawEvent.path);
            const auto lyonGeometry = build_stroke(std::move(builder), drawEvent.penWidth);
            QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
                    lyonGeometry.vertices.size(), lyonGeometry.indices.size());

            geometry->setIndexDataPattern(QSGGeometry::StaticPattern);
            geometry->setDrawingMode(GL_TRIANGLES);
            node->setGeometry(geometry);
            node->setFlag(QSGNode::OwnsGeometry);

            QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
            material->setColor(drawEvent.penColor);
            node->setMaterial(material);
            node->setFlag(QSGNode::OwnsMaterial);
            root->appendChildNode(node);

            QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
            std::size_t i = 0;
            for (const auto &vertice: lyonGeometry.vertices) {
                points[i].set(vertice.x, vertice.y);
                i++;
            }

            quint16* indices = geometry->indexDataAsUShort();
            i = 0;
            for (const auto indice: lyonGeometry.indices) {
                indices[i] = indice;
                i++;
            }

            node->markDirty(QSGNode::DirtyGeometry);
        }
    }

    return root;
}


#include "moc_drawingarea.cpp"
