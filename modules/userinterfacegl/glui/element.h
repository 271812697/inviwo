/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_GLUIELEMENT_H
#define IVW_GLUIELEMENT_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/interaction/pickingmapper.h>

#include <functional>
#include <string>
#include <vector>

namespace inviwo {

class Texture2D;

namespace glui {

class Renderer;

enum class ItemType { Unknown, Checkbox, Slider, Button };

/**
 * \class glui::Element
 * \brief graphical UI element for use in combination with glui::Manager
 *
 * \see glui::Manager
 */
class IVW_MODULE_USERINTERFACEGL_API Element {
public:
    enum class UIState { Normal, Pressed, Checked };

    Element(ItemType type, const std::string &label, Renderer *uiRenderer);
    virtual ~Element();

    ItemType getType() const;

    void setLabel(const std::string &str);
    const std::string &getLabel() const;

    bool isDirty() const;

    void setVisible(bool visible = true);
    bool isVisible() const;

    const ivec2 &getExtent();

    /**
     * \brief a UI element may consist of several, separate components.
     * Each with a unique picking ID.
     *
     * @return number of widget components / picking IDs
     */
    virtual int getNumWidgetComponents() const;

    /**
     * \brief update all picking IDs starting at the given index.
     *
     * @param startIndex  first picking ID of the UI element
     * @return first unused picking ID, i.e. startIndex + number of widget components
     *
     * \see getNumWidgetComponents
     */
    int updatePickingIDs(const int startIndex);

    /**
     * \brief check whether the UI element uses a certain picking ID.
     *
     * @return true if picking ID is assigned to this UI element
     */
    bool hasPickingID(int id);

    /**
     * \brief render the widget and its label at the given position
     *
     * @param origin         defines the lower left corner where the widget is positioned
     * @param pickingMapper  picking mapper provided by the glui::manager
     * @param canvasDim      dimensions of the output canvas
     */
    void render(const ivec2 &origin, const PickingMapper &pickingMapper, const ivec2 &canvasDim);

    void renderLabel(const ivec2 &origin, const size2_t &canvasDim);

    void setHoverState(bool enable);

    void setPushedState(bool pushed);
    bool isPushed() const;

    /**
     * \brief sets the callback action when the user releases the mouse button
     */
    void setAction(const std::function<void()> &action);

    /**
     * \brief updates the UI state and triggers the callback action set by setAction().
     * This function is called when the user releases the mouse button.
     *
     * \see setAction
     */
    void triggerAction();

    /**
     * \brief set callback function for handling mouse movements based on a delta position.
     * This callback gets called on mouse move events.
     *
     * @param action   function taking one argument (2D delta position in screen coords) returning
     * true if the movement triggers an update of the element
     */
    void setMouseMoveAction(const std::function<bool(const dvec2 &)> &action);

    /**
     * \brief gets called on mouse move events
     *
     * @param delta    delta mouse position in screen coord, i.e. pixels, relative to pressed
     * position
     * @return true if the movement triggers an update of the element
     */
    bool moveAction(const dvec2 &delta);

protected:
    void updateExtent();
    void updateLabelPos();
    void updateLabel();
    virtual ivec2 computeLabelPos(int descent) const = 0;
    virtual UIState uiState() const;
    virtual vec2 marginScale() const;
    /**
     * \brief is called before the action is triggered to update the internal UI state
     *
     * \see triggerAction, setAction
     */
    virtual void updateState(){};

    /**
     * \brief  It is called by setPushState after the internal push state has been updated
     */
    virtual void pushStateChanged(){};

    virtual void renderWidget(const ivec2 &origin, const PickingMapper &pickingMapper) = 0;

    ItemType itemType_;

    std::function<void()>
        action_;  //<! is called by triggerAction() after the internal state has been updated
    std::function<bool(const dvec2 &)> moveAction_;  //!< is called by mouseMoved()

    // UI interaction states
    bool hovered_;  // true as long as the element is under the mouse
    bool pushed_;  // true as long as the mouse button is not released, mouse might not be on top of
                   // UI element any more
    bool checked_;

    bool visible_;

    // Layout of a UI element:
    //                                                         extent
    //   +----------------------------------------------------+
    //   |                                                    |
    //   |                 widgetExtent                       |
    //   |     +----------+                      labelExtent  |
    //   |     | rendered |        +-----------------+        |
    //   |     | textures |        |  label          |        |
    //   |     |    +     |        +-----------------+        |
    //   |     | picking  |    labelPos                       |
    //   |     +----------+                                   |
    //   |  widgetPos                                         |
    //   |                                                    |
    //   +----------------------------------------------------+
    // (0,0)
    //

    ivec2 extent_;
    ivec2 widgetPos_;
    ivec2 widgetExtent_;
    ivec2 labelPos_;
    ivec2 labelExtent_;

    std::string labelStr_;
    bool labelDirty_;

    std::vector<int> pickingIDs_;

    std::shared_ptr<Texture2D> labelTexture_;

    Renderer *uiRenderer_;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_GLUIELEMENT_H
