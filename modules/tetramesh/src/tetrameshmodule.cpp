/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <inviwo/tetramesh/tetrameshmodule.h>

#include <inviwo/tetramesh/datastructures/tetramesh.h>
#include <inviwo/tetramesh/processors/tetrameshboundingbox.h>
#include <inviwo/tetramesh/processors/tetrameshvolumeraycaster.h>
#include <inviwo/tetramesh/processors/tetrameshboundaryextractor.h>
#include <inviwo/tetramesh/processors/transformtetramesh.h>
#include <inviwo/tetramesh/processors/volumetotetramesh.h>

#include <inviwo/tetramesh/ports/tetrameshport.h>

#include <modules/base/processors/inputselector.h>
#include <modules/opengl/shader/shadermanager.h>

namespace inviwo {

TetraMeshModule::TetraMeshModule(InviwoApplication* app) : InviwoModule(app, "TetraMesh") {
    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    registerProcessor<TetraMeshBoundaryExtractor>();
    registerProcessor<TetraMeshBoundingBox>();
    registerProcessor<TetraMeshVolumeRaycaster>();
    registerProcessor<TransformTetraMesh>();
    registerProcessor<VolumeToTetraMesh>();

    registerProcessor<InputSelector<MultiDataInport<TetraMesh>, DataOutport<TetraMesh>>>();

    registerDefaultsForDataType<TetraMesh>();
}

}  // namespace inviwo
