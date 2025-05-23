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

#include <modules/base/processors/layercombiner.h>

#include <inviwo/core/datastructures/image/layerram.h>
#include <modules/base/algorithm/combinechannels.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerCombiner::processorInfo_{
    "org.inviwo.LayerCombiner",  // Class identifier
    "Layer Combiner",            // Display name
    "Layer Operation",           // Category
    CodeState::Experimental,     // Code state
    Tags::CPU | Tag{"Layer"},    // Tags
    R"(
Combines multiple layers into a single layer with multiple channels. All layers must 
share the same dimensions. The resulting data format depends on the common data type
and precision of the inputs.
)"_unindentHelp,
};

const ProcessorInfo& LayerCombiner::getProcessorInfo() const { return processorInfo_; }

LayerCombiner::LayerCombiner()
    : Processor{}
    , source_{LayerInport{"source1", "Input for the first channel (red)"_help},
              LayerInport{"source2", "Input for the second channel (green, optional)"_help},
              LayerInport{"source3", "Input for the third channel (blue, optional)"_help},
              LayerInport{"source4", "Input for the fourth channel (alpha, optional)"_help}}
    , outport_{"outport", "Resulting Layer with combined channels"_help}

    , channel_{OptionPropertyInt{"dest1", "Channel 1 Out",
                                 "Selected channel of the first input"_help,
                                 util::enumeratedOptions("Channel", 4)},
               OptionPropertyInt{"dest2", "Channel 2 Out",
                                 "Selected channel of the second input"_help,
                                 util::enumeratedOptions("Channel", 4)},
               OptionPropertyInt{"dest3", "Channel 3 Out",
                                 "Selected channel of the third input"_help,
                                 util::enumeratedOptions("Channel", 4)},
               OptionPropertyInt{"dest4", "Channel 4 Out",
                                 "Selected channel of the fourth input"_help,
                                 util::enumeratedOptions("Channel", 4)}}
    , normalizeChannels_{"normalizeChannels", "Normalize Channels",
                         "If true, the individual channels of the output volume will be normalized "
                         "using the data ranges of their corresponding input volumes. Otherwise, "
                         "the Data Range of the first input volume is used."_help,
                         false}
    , dataRange_{"dataRange", "Data Range", source_[0], true}
    , valueAxis_{"valueAxis", "Value Axis", source_[0], true} {

    addPorts(source_[0], source_[1], source_[2], source_[3], outport_);
    for (auto& port : std::span(source_.begin() + 1, 3)) {
        port.setOptional(true);
    }

    addProperties(channel_[0], channel_[1], channel_[2], channel_[3], normalizeChannels_,
                  dataRange_, valueAxis_);
}

void LayerCombiner::process() {
    std::shared_ptr<Layer> layer;
    if (normalizeChannels_) {
        layer = util::combineChannels<Layer, LayerRAM, util::detail::ChannelNormalization::Yes>(
            source_, {{channel_[0], channel_[1], channel_[2], channel_[3]}});
    } else {
        layer = util::combineChannels<Layer, LayerRAM>(
            source_, {{channel_[0], channel_[1], channel_[2], channel_[3]}});
        layer->dataMap.dataRange = dataRange_.getDataRange();
        layer->dataMap.valueRange = dataRange_.getValueRange();
        layer->dataMap.valueAxis.name = valueAxis_.getValueName();
        layer->dataMap.valueAxis.unit = valueAxis_.getValueUnit();
    }

    outport_.setData(layer);
}

}  // namespace inviwo
