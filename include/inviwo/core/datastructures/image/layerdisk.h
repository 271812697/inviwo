/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>

namespace inviwo {

/**
 * \ingroup datastructures
 */
class IVW_CORE_API LayerDisk : public LayerRepresentation,
                               public DiskRepresentation<LayerRepresentation, LayerDisk> {
public:
    explicit LayerDisk(size2_t dimensions, const DataFormatBase* format,
                       LayerType type = LayerConfig::defaultType,
                       const SwizzleMask& swizzleMask = LayerConfig::defaultSwizzleMask,
                       InterpolationType interpolation = LayerConfig::defaultInterpolation,
                       const Wrapping2D& wrap = LayerConfig::defaultWrapping);
    explicit LayerDisk(const std::filesystem::path& url, size2_t dimensions,
                       const DataFormatBase* format, LayerType type = LayerConfig::defaultType,
                       const SwizzleMask& swizzleMask = LayerConfig::defaultSwizzleMask,
                       InterpolationType interpolation = LayerConfig::defaultInterpolation,
                       const Wrapping2D& wrap = LayerConfig::defaultWrapping);
    explicit LayerDisk(const LayerReprConfig& config, const std::filesystem::path& path = {});
    LayerDisk(const LayerDisk& rhs) = default;
    LayerDisk& operator=(const LayerDisk& that) = default;
    virtual LayerDisk* clone() const override;
    virtual ~LayerDisk();

    virtual const DataFormatBase* getDataFormat() const override;

    virtual const size2_t& getDimensions() const override;

    /**
     * Copy and resize the representations of this onto the target.
     */
    virtual bool copyRepresentationsTo(LayerRepresentation* target) const override;

    /**
     * \brief Updates the data format retrieved during loading
     *
     * @param format the new dataformat
     *
     */
    void updateDataFormat(const DataFormatBase* format);
    virtual std::type_index getTypeIndex() const override final;

    /**
     * \brief update the swizzle mask of the channels for sampling color layers
     * Needs to be overloaded by child classes.
     *
     * @param mask    new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping2D& wrapping) override;
    virtual Wrapping2D getWrapping() const override;

private:
    // clang-format off
    [[deprecated("does not work for DiskRepresentation (deprecated since 2019-06-27)")]]
    virtual void setDimensions(size2_t dimensions) override;
    // clang-format on

    const DataFormatBase* dataFormatBase_;
    size2_t dimensions_;
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping2D wrapping_;
};

}  // namespace inviwo
