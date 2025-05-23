/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/properties/integrallineproperties.h>

#include <inviwo/core/datastructures/coordinatetransformer.h>  // for CoordinateSpace, Coordinat...
#include <inviwo/core/properties/boolproperty.h>               // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>          // for CompositeProperty
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>            // for FloatProperty, IntProperty
#include <inviwo/core/util/staticstring.h>                     // for operator+

namespace inviwo {

IntegralLineProperties::IntegralLineProperties(std::string_view identifier,
                                               std::string_view displayName)
    : CompositeProperty(identifier, displayName)
    , numberOfSteps_("steps", "Number of Steps", util::ordinalCount(100, 1000))
    , stepSize_("stepSize", "Step size", util::ordinalScale(0.001f, 1.0f))
    , normalizeSamples_("normalizeSamples", "Normalize Samples", true)
    , stepDirection_("stepDirection", "Step Direction")
    , integrationScheme_("integrationScheme", "Integration Scheme")
    , seedPointsSpace_("seedPointsSpace", "Seed Points Space") {
    setUpProperties();
}

IntegralLineProperties::IntegralLineProperties(const IntegralLineProperties& rhs)
    : CompositeProperty(rhs)
    , numberOfSteps_(rhs.numberOfSteps_)
    , stepSize_(rhs.stepSize_)
    , normalizeSamples_(rhs.normalizeSamples_)
    , stepDirection_(rhs.stepDirection_)
    , integrationScheme_(rhs.integrationScheme_)
    , seedPointsSpace_(rhs.seedPointsSpace_) {
    setUpProperties();
}

IntegralLineProperties* IntegralLineProperties::clone() const {
    return new IntegralLineProperties(*this);
}

IntegralLineProperties::~IntegralLineProperties() = default;

int IntegralLineProperties::getNumberOfSteps() const { return numberOfSteps_.get(); }

float IntegralLineProperties::getStepSize() const { return stepSize_.get(); }

IntegralLineProperties::Direction IntegralLineProperties::getStepDirection() const {
    return stepDirection_.get();
}

IntegralLineProperties::IntegrationScheme IntegralLineProperties::getIntegrationScheme() const {
    return integrationScheme_.get();
}

CoordinateSpace IntegralLineProperties::getSeedPointsSpace() const {
    return seedPointsSpace_.get();
}

bool IntegralLineProperties::getNormalizeSamples() const { return normalizeSamples_; }

void IntegralLineProperties::setUpProperties() {
    stepDirection_.addOption("fwd", "Forward", IntegralLineProperties::Direction::Forward);
    stepDirection_.addOption("bwd", "Backwards", IntegralLineProperties::Direction::Backward);
    stepDirection_.addOption("bi", "Bi Directional",
                             IntegralLineProperties::Direction::Bidirectional);

    integrationScheme_.addOption("euler", "Euler",
                                 IntegralLineProperties::IntegrationScheme::Euler);
    integrationScheme_.addOption("rk4", "Runge-Kutta (RK4)",
                                 IntegralLineProperties::IntegrationScheme::RK4);
    integrationScheme_.setSelectedValue(IntegralLineProperties::IntegrationScheme::RK4);

    seedPointsSpace_.addOption("data", "Data", CoordinateSpace::Data);
    seedPointsSpace_.addOption("model", "Model", CoordinateSpace::Model);
    seedPointsSpace_.addOption("world", "World", CoordinateSpace::World);

    addProperty(numberOfSteps_);
    addProperty(stepSize_);
    addProperty(stepDirection_);
    addProperty(integrationScheme_);
    addProperty(seedPointsSpace_);
    addProperty(normalizeSamples_);

    setAllPropertiesCurrentStateAsDefault();
}

}  // namespace inviwo
