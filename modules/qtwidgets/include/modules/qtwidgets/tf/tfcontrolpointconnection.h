/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <modules/qtwidgets/tf/tfeditorprimitive.h>  // for TFEditorPrimitive, TFEditorPrimitiv...

#include <QGraphicsItem>  // for QGraphicsItem, QGraphicsItem::UserType
#include <QPointF>        // for QPointF
#include <QRectF>         // for QRectF
#include <QPainterPath>   // for QPainterPath

class QPainter;
class QPointF;
class QRectF;
class QStyleOptionGraphicsItem;
class QWidget;

namespace inviwo {

class TFEditorControlPoint;

class IVW_MODULE_QTWIDGETS_API TFControlPointConnection : public QGraphicsItem {
public:
    TFControlPointConnection();
    virtual ~TFControlPointConnection();

    void updateShape();

    TFEditorPrimitive* left;   // Non-owning reference
    TFEditorPrimitive* right;  // Non-owning reference

    QPointF getStart() const;
    QPointF getStop() const;

protected:
    // Overload
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget);
    QRectF boundingRect() const;
    QPainterPath shape() const;

private:
    QPainterPath path_;
    QPainterPath shape_;
    QRectF rect_;
};

}  // namespace inviwo
