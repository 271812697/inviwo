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

#include <modules/qtwidgets/eventconverterqt.h>

#include <inviwo/core/interaction/events/gesturestate.h>  // for GestureState, GestureState::NoG...
#include <inviwo/core/interaction/events/keyboardkeys.h>  // for KeyModifiers, KeyModifier, IvwKey
#include <inviwo/core/interaction/events/mousebuttons.h>  // for MouseButton, MouseButtons, Mous...
#include <modules/qtwidgets/keyboardutils.h>              // for mapKeyFromQt

#include <QFlags>         // for QFlags
#include <QGesture>       // for QGesture
#include <QInputEvent>    // for QInputEvent
#include <QMouseEvent>    // for QMouseEvent
#include <QWheelEvent>    // for QWheelEvent
#include <Qt>             // for LeftButton, MiddleButton, Right...
#include <flags/flags.h>  // for none

namespace inviwo {

namespace utilqt {

MouseButtons getMouseButtons(const QMouseEvent* e) {
    MouseButtons res(flags::none);

    if (e->buttons() & Qt::LeftButton) res |= MouseButton::Left;
    if (e->buttons() & Qt::MiddleButton) res |= MouseButton::Middle;
    if (e->buttons() & Qt::RightButton) res |= MouseButton::Right;

    return res;
}

MouseButton getMouseButtonCausingEvent(const QMouseEvent* e) {
// QMouseEvent::getButtons does not
// include the button that caused the event.
// The QMouseEvent::getButton function
// returns the button that caused the event
#include <warn/push>
#include <warn/ignore/switch-enum>
    switch (e->button()) {
        case Qt::LeftButton:
            return MouseButton::Left;
        case Qt::RightButton:
            return MouseButton::Right;
        case Qt::MiddleButton:
            return MouseButton::Middle;
        default:
            return MouseButton::None;
    }
#include <warn/pop>
}

MouseButtons getMouseWheelButtons(const QWheelEvent* e) {
    MouseButtons res(flags::none);

    if (e->buttons() & Qt::LeftButton) res |= MouseButton::Left;
    if (e->buttons() & Qt::MiddleButton) res |= MouseButton::Middle;
    if (e->buttons() & Qt::RightButton) res |= MouseButton::Right;

    return res;
}

GestureState getGestureState(const QGesture* gesture) {
    switch (gesture->state()) {
        case Qt::NoGesture:
            return GestureState::NoGesture;
        case Qt::GestureStarted:
            return GestureState::Started;
        case Qt::GestureUpdated:
            return GestureState::Updated;
        case Qt::GestureFinished:
            return GestureState::Finished;
        case Qt::GestureCanceled:
            return GestureState::Canceled;
        default:
            return GestureState::NoGesture;
    }
}

KeyModifiers getModifiers(const QInputEvent* e) { return getModifiers(e->modifiers()); }

KeyModifiers getModifiers(Qt::KeyboardModifiers m) {
    KeyModifiers res(flags::none);

    if (m & Qt::ShiftModifier) res |= KeyModifier::Shift;
    if (m & Qt::ControlModifier) res |= KeyModifier::Control;
    if (m & Qt::AltModifier) res |= KeyModifier::Alt;
    if (m & Qt::MetaModifier) res |= KeyModifier::Meta;

    return res;
}

IvwKey getKeyButton(const QKeyEvent* e) { return util::mapKeyFromQt(e); }

}  // namespace utilqt

}  // namespace inviwo
