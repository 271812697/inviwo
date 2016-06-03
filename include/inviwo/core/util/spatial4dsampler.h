/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_SPATIAL4DSAMPLER_H
#define IVW_SPATIAL4DSAMPLER_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/util/spatialsampler.h>

namespace inviwo {

    class IVW_CORE_API Spatial4DSamplerBase {
    public:
        static uvec3 COLOR_CODE;
        virtual std::string getDataInfo() const {
            return "";
        }
    };


template <unsigned DataDims, typename T>
class Spatial4DSampler : public Spatial4DSamplerBase {
public:
    using Space = typename SpatialCoordinateTransformer<3>::Space;

    Spatial4DSampler(std::shared_ptr<const SpatialEntity<3>> spatialEntity) 
        : spatialEntity_(spatialEntity) {}
    virtual ~Spatial4DSampler() = default;

    virtual std::string getDataInfo() const override {
        return "Spatial4DSampler" + toString(DataDims) +
               inviwo::parseTypeIdName(std::string(typeid(T).name()));
    }


    virtual Vector<DataDims, T> sample(const dvec4 &pos,
        Space space = Space::Data) const {
        auto dataPos = pos.xyz();
        if (space != Space::Data) {
            auto m = spatialEntity_->getCoordinateTransformer().getMatrix(space, Space::Data);
            auto p = m * vec4(
                static_cast<vec3>(pos), 1.0);
            dataPos = p.xyz() / p.w;
        }

        return sampleDataSpace(dvec4(dataPos, pos.w));
    }

    virtual Vector<DataDims, T> sample(const vec4 &pos,
        Space space = Space::Data) const {
        return sample(static_cast<dvec4>(pos), space);
    }

    virtual bool withinBounds(const dvec4 &pos,
        Space space = Space::Data) const {
        auto dataPos = pos.xyz();
        if (space != Space::Data) {
            auto m = spatialEntity_->getCoordinateTransformer().getMatrix(space, Space::Data);
            auto p = m * vec4(
                static_cast<vec3>(dataPos), 1.0f);
            dataPos = p.xyz() / p.w;
        }

        return withinBoundsDataSpace(dvec4(dataPos,pos.w));
    }

    virtual bool withinBounds(const vec4 &pos,
        Space space = Space::Data) const {
        return withinBounds(static_cast<dvec4>(pos), space);
    }

    const SpatialCoordinateTransformer<3> &getCoordinateTransformer()const {
        return spatialEntity_->getCoordinateTransformer();
    }

    mat4 getModelMatrix()const {
        return spatialEntity_->getModelMatrix();
    }


    mat4 getWorldMatrix()const {
        return spatialEntity_->getWorldMatrix();
    }

protected:
    virtual Vector<DataDims, T> sampleDataSpace(const dvec4 &pos) const = 0;
    virtual bool withinBoundsDataSpace(const dvec4 &pos) const = 0;

    std::shared_ptr<const SpatialEntity<3>> spatialEntity_;
};


}  // namespace

#endif  // IVW_SPATIALSAMPLER_H
