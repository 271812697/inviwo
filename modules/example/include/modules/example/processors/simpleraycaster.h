/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/example/examplemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

class Volume;

/** \docpage{org.inviwo.SimpleRaycaster, Simple Raycaster}
 * ![](org.inviwo.SimpleRaycaster.png?classIdentifier=org.inviwo.SimpleRaycaster)
 * Exemplary processor for direct volume rendering using raycasting.
 * Besides volume data, entry and exit point locations of the bounding box are required. These
 * can be created with the EntryExitPoints processor. The camera properties between these
 * two processors need to be linked.
 *
 * ### Inports
 *   * __volume__ input volume
 *   * __entry__  entry point locations of input volume (generated by EntryExitPoints processor)
 *   * __exit__   exit point positions of input volume (generated by EntryExitPoints processor)
 *
 * ### Outports
 *   * __outport__ output image containing volume rendering of the input
 *
 * ### Properties
 *   * __Render Channel__    selects which channel of the input volume is rendered
 *   * __Transfer function__ transfer function properties
 *   * __Sampling Rate__     determines the step length of the raycasting based on volume dimensions
 *   * __Camera__            camera properties (to be linked with EntryExitPoints processor)
 */

/**
 * \class SimpleRaycaster
 * \brief Exemplary processor for direct volume rendering using raycasting.
 *
 * This processor features the basic functionality for a shader-based volume raycasting. For
 * a more advanced version including shading and isosurfaces check out the VolumeRaycaster
 * processor.
 *
 * \see VolumeRaycaster
 */
class IVW_MODULE_EXAMPLE_API SimpleRaycaster : public Processor {
public:
    SimpleRaycaster();
    virtual ~SimpleRaycaster() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    Shader shader_;
    VolumeInport volumePort_;
    ImageInport entryPort_;
    ImageInport exitPort_;
    ImageOutport outport_;

    OptionPropertyInt channel_;
    TransferFunctionProperty transferFunction_;
    FloatProperty samplingRate_;
    CameraProperty camera_;

    std::shared_ptr<const Volume> loadedVolume_;
};

}  // namespace inviwo
