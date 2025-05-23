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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadatamap.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

template <typename T, typename U>
concept Settable = requires(T item, U value) {
    { item.set(value) };
};
template <typename T, typename U>
concept Gettable = requires(T item) {
    { item.get() } -> std::convertible_to<U>;
};

/** @brief Holds metadata and access functionality for set/get
 *  MetaDataOwner is the base class for all the objects that want to own metadata.
 */
class IVW_CORE_API MetaDataOwner {

public:
    MetaDataOwner() = default;
    MetaDataOwner(const MetaDataOwner& rhs) = default;
    MetaDataOwner(MetaDataOwner& rhs) = default;
    MetaDataOwner& operator=(const MetaDataOwner& rhs) = default;
    MetaDataOwner& operator=(MetaDataOwner&) = default;
    ~MetaDataOwner() = default;

    // copy the meta data from src to *this
    void copyMetaDataFrom(const MetaDataOwner& src);
    // copy the meta data from *this to dst
    void copyMetaDataTo(MetaDataOwner& dst);

    // MetaData
    template <typename T>
        requires std::derived_from<T, MetaData>
    T* createMetaData(std::string_view key);

    template <typename T, typename U>
        requires std::derived_from<T, MetaData> && Settable<T, U>
    void setMetaData(std::string_view key, U value);

    /**
     * @brief unset, i.e. remove the metadata entry matching the given key and type
     * @param key   key of the entry to be removed
     */
    template <typename T>
        requires std::derived_from<T, MetaData>
    bool unsetMetaData(std::string_view key);

    // param val is required to deduce the template argument
    template <typename T, typename U>
        requires std::derived_from<T, MetaData> && Gettable<T, U>
    U getMetaData(std::string_view key, U val) const;
    template <typename T>
        requires std::derived_from<T, MetaData>
    T* getMetaData(std::string_view key);

    template <typename T>
        requires std::derived_from<T, MetaData>
    const T* getMetaData(std::string_view key) const;

    MetaDataMap* getMetaDataMap();
    const MetaDataMap* getMetaDataMap() const;

    bool hasMetaData(std::string_view key) const;

    template <typename T>
        requires std::derived_from<T, MetaData>
    bool hasMetaData(std::string_view key) const;

    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);

protected:
    MetaDataMap metaData_;
};

template <typename T>
    requires std::derived_from<T, MetaData>
T* MetaDataOwner::createMetaData(std::string_view key) {
    if (T* metaData = dynamic_cast<T*>(metaData_.get(key))) {
        return metaData;
    } else {
        return metaData_.add(key, std::make_unique<T>());
    }
}

template <typename T, typename U>
    requires std::derived_from<T, MetaData> && Settable<T, U>
void MetaDataOwner::setMetaData(std::string_view key, U value) {
    if (MetaData* baseMetaData = metaData_.get(key)) {
        if (auto derivedMetaData = dynamic_cast<T*>(baseMetaData)) {
            derivedMetaData->set(value);
            return;
        }
    }
    metaData_.add(key, std::make_unique<T>())->set(value);
}

template <typename T>
    requires std::derived_from<T, MetaData>
bool MetaDataOwner::unsetMetaData(std::string_view key) {
    return metaData_.remove(key);
}

// param val is required to deduce the template argument
template <typename T, typename U>
    requires std::derived_from<T, MetaData> && Gettable<T, U>
U MetaDataOwner::getMetaData(std::string_view key, U val) const {
    if (const MetaData* baseMetadata = metaData_.get(key)) {
        if (auto derivedMetaData = dynamic_cast<const T*>(baseMetadata)) {
            return derivedMetaData->get();
        }
    }
    return val;
}

template <typename T>
    requires std::derived_from<T, MetaData>
const T* MetaDataOwner::getMetaData(std::string_view key) const {
    return dynamic_cast<const T*>(metaData_.get(key));
}

template <typename T>
    requires std::derived_from<T, MetaData>
T* MetaDataOwner::getMetaData(std::string_view key) {
    return dynamic_cast<T*>(metaData_.get(key));
}

inline bool MetaDataOwner::hasMetaData(std::string_view key) const {
    return metaData_.contains(key);
}

template <typename T>
    requires std::derived_from<T, MetaData>
bool MetaDataOwner::hasMetaData(std::string_view key) const {
    if (const MetaData* baseMetadata = metaData_.get(key)) {
        if (const T* derivedMetaData = dynamic_cast<const T*>(baseMetadata)) {
            return true;
        }
    }

    return false;
}

}  // namespace inviwo
