/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/properties/propertyeditorwidget.h>  // for PropertyEditorWidget
#include <inviwo/core/properties/propertyobserver.h>      // for PropertyObserver
#include <inviwo/core/util/glmvec.h>                      // for ivec2
#include <modules/qtwidgets/inviwodockwidget.h>           // for InviwoDockWidget

#include <string>  // for string

class QCloseEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;

namespace inviwo {

class Property;

// PropertyEditorWidget owned by PropertyWidget
class IVW_MODULE_QTWIDGETS_API PropertyEditorWidgetQt : public InviwoDockWidget,
                                                        public PropertyEditorWidget,
                                                        public PropertyObserver {
public:
    PropertyEditorWidgetQt(Property* property, const std::string& widgetName);
    PropertyEditorWidgetQt(Property* property, const std::string& widgetName,
                           const std::string& objName);
    virtual ~PropertyEditorWidgetQt();

    // PropertyEditorWidget overrides
    virtual Property* getProperty() const override = 0;
    virtual bool isVisible() const override;
    virtual void setVisible(bool visible) override;

    virtual ivec2 getPosition() const override;
    virtual void setPosition(const ivec2& pos) override;

    virtual ivec2 getDimensions() const override;
    virtual void setDimensions(const ivec2& dimensions) override;

    virtual void saveState() override;
    virtual void loadState() override;

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void showEvent(QShowEvent*) override;
    virtual void closeEvent(QCloseEvent*) override;
    virtual void moveEvent(QMoveEvent* event) override;

    // PropertyObserver overrides
    virtual void onSetReadOnly(Property* property, bool readonly) override;

    virtual void setReadOnly(bool readonly);

    static constexpr std::string_view visibleKey{"PropertyEditorWidgetVisible"};
    static constexpr std::string_view floatingKey{"PropertyEditorWidgetFloating"};
    static constexpr std::string_view stickyKey{"PropertyEditorWidgetSticky"};
    static constexpr std::string_view sizeKey{"PropertyEditorWidgetSize"};
    static constexpr std::string_view positionKey{"PropertyEditorWidgetPosition"};
    static constexpr std::string_view dockareaKey{"PropertyEditorWidgetDockStatus"};
};

}  // namespace inviwo
