/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/marginproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/interaction/cameratrackball.h>

#include <modules/plotting/properties/axisproperty.h>       // for AxisProperty
#include <modules/plotting/properties/axisstyleproperty.h>  // for AxisStyleProperty
#include <modules/plottinggl/utils/axisrenderer.h>          // for AxisRenderer

namespace inviwo {

class IVW_MODULE_PLOTTINGGL_API OrthographicAxis2D : public Processor {
public:
    OrthographicAxis2D();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport inport_;
    MeshInport mesh_;
    ImageOutport outport_;

    plot::AxisStyleProperty style_;
    FloatVec4Property backgroundColor_;
    BoolProperty clipContent_;
    plot::AxisProperty axis1_;
    plot::AxisProperty axis2_;
    MarginProperty margins_;
    FloatProperty axisMargin_;
    BoolProperty antialiasing_;
    CameraProperty camera_;
    CameraTrackball trackball_;

    std::array<plot::AxisRenderer, 2> axisRenderers_;
};

}  // namespace inviwo
