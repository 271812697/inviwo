/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>

#include <modules/qtwidgets/properties/propertywidgetqt.h>

#include <memory>

class QFocusEvent;
class QKeyEvent;
class QMouseEvent;

namespace inviwo {

class EditableLabelQt;
class EventMatcher;
class EventProperty;
class IvwPushButton;
class KeyboardEventMatcher;
class MouseEventMatcher;

class IVW_MODULE_QTWIDGETS_API EventPropertyWidgetQt : public PropertyWidgetQt {

public:
    EventPropertyWidgetQt(EventProperty* eventproperty);
    virtual ~EventPropertyWidgetQt();
    virtual void updateFromProperty() override;

    std::unique_ptr<QMenu> getContextMenu() override;

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

    virtual void focusOutEvent(QFocusEvent* event) override;
    void clickedSlot();

private:
    void setButtonText();

    EventProperty* eventProperty_;
    IvwPushButton* button_;
    EditableLabelQt* label_;

    std::unique_ptr<EventMatcher> matcher_;
    KeyboardEventMatcher* keyMatcher_ = nullptr;
    MouseEventMatcher* mouseMatcher_ = nullptr;
};

}  // namespace inviwo
