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

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMATIONQT_API

#include <modules/animation/animationcontrollerobserver.h>       // for AnimationControllerObserver
#include <modules/animation/datastructures/animationobserver.h>  // for AnimationObserver

#include <QListView>  // for QListView

class QStandardItemModel;

namespace inviwo {

namespace animation {

class AnimationController;
class Track;
class Animation;

class IVW_MODULE_ANIMATIONQT_API AnimationLabelViewQt : public QListView,
                                                        public AnimationObserver,
                                                        public AnimationControllerObserver {
public:
    AnimationLabelViewQt(AnimationController& controller);
    virtual ~AnimationLabelViewQt() = default;

    virtual void onTrackAdded(Track* track) override;
    virtual void onTrackRemoved(Track* track) override;

    // AnimationControllerObserver overload
    void onAnimationChanged(AnimationController*, Animation* oldAnim, Animation* newAnim) override;

    AnimationController& controller_;
    QStandardItemModel* model_;
};

}  // namespace animation

}  // namespace inviwo
