/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/exception.h>

#include <iostream>

#include <inviwo/core/io/serialization/ticpp.h>

namespace inviwo {

namespace {

int getVersionAttribute(TiXmlElement* elem) {
    int version{};
    if (const auto ver = elem->Attribute(SerializeConstants::VersionAttribute)) {
        detail::fromStr(*ver, version);
        return version;
    } else {
        throw AbortException("Missing version tag");
    }
}

TiXmlElement* getRootElement(TiXmlDocument& doc, std::string_view rootElement) {
    auto* root = doc.FirstChildElement(rootElement);
    if (root) return root;

    root = doc.FirstChildElement();
    if (root) {
        log::warn("Deserializer expected to find root element of type: '{}' but found '{}'",
                  rootElement, root->Value());
        return root;
    }

    throw AbortException(SourceContext{}, "Unable to find a root element, expected a '{}'",
                         rootElement);
}

}  // namespace

Deserializer::Deserializer(const std::filesystem::path& fileName, std::string_view rootElement,
                           allocator_type alloc)
    : SerializeBase(fileName, alloc), registeredFactories_{alloc} {
    try {
        doc_->LoadFile();
        rootElement_ = getRootElement(*doc_, rootElement);
        version_ = getVersionAttribute(rootElement_);
    } catch (const TiXmlError& e) {
        throw AbortException(e.what());
    }
}

Deserializer::Deserializer(std::istream& stream, const std::filesystem::path& refPath,
                           std::string_view rootElement, allocator_type alloc)
    : SerializeBase(refPath, alloc), registeredFactories_{alloc} {
    try {
        std::pmr::string data{alloc};
        stream.seekg(0, std::ios::end);
        data.reserve(stream.tellg());
        stream.seekg(0, std::ios::beg);
        data.assign(std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{});

        doc_->Parse(data.c_str(), nullptr, alloc);
        rootElement_ = getRootElement(*doc_, rootElement);
        version_ = getVersionAttribute(rootElement_);
    } catch (const TiXmlError& e) {
        throw AbortException(e.what());
    }
}

Deserializer::Deserializer(std::istream& stream, std::string_view rootElement, allocator_type alloc)
    : Deserializer{stream, std::filesystem::path{}, rootElement, alloc} {}

Deserializer::Deserializer(const std::pmr::string& content, const std::filesystem::path& refPath,
                           std::string_view rootElement, allocator_type alloc)
    : SerializeBase(refPath, alloc), registeredFactories_{alloc} {
    try {
        doc_->Parse(content.c_str(), nullptr, alloc);
        rootElement_ = getRootElement(*doc_, rootElement);
        version_ = getVersionAttribute(rootElement_);
    } catch (const TiXmlError& e) {
        throw AbortException(e.what());
    }
}

Deserializer::Deserializer(const std::pmr::string& content, std::string_view rootElement,
                           allocator_type alloc)
    : Deserializer{content, std::filesystem::path{}, rootElement, alloc} {}

void Deserializer::deserialize(std::string_view key, std::filesystem::path& path,
                               const SerializationTarget& target) {

    try {
        if (target == SerializationTarget::Attribute) {
            if (const auto val = rootElement_->Attribute(key)) {
                path = *val;
            }
        } else {
            if (const NodeSwitch ns{*this, key}) {
                if (const auto val =
                        rootElement_->Attribute(SerializeConstants::ContentAttribute)) {
                    path = *val;
                }
                return;
            }
            if (const NodeSwitch ns{*this, key, true}) {
                if (const auto val =
                        rootElement_->Attribute(SerializeConstants::ContentAttribute)) {
                    path = *val;
                }
                return;
            }
        }
    } catch (...) {
        handleError();
    }
}

void Deserializer::deserialize(std::string_view key, Serializable& sObj) {
    if (const NodeSwitch ns{*this, key}) {
        sObj.deserialize(*this);
    }
}

void Deserializer::deserialize(std::string_view key, signed char& data,
                               const SerializationTarget& target) {
    int val = data;
    deserialize(key, val, target);
    data = static_cast<char>(val);
}
void Deserializer::deserialize(std::string_view key, char& data,
                               const SerializationTarget& target) {
    int val = data;
    deserialize(key, val, target);
    data = static_cast<char>(val);
}
void Deserializer::deserialize(std::string_view key, unsigned char& data,
                               const SerializationTarget& target) {
    unsigned int val = data;
    deserialize(key, val, target);
    data = static_cast<unsigned char>(val);
}

void Deserializer::setExceptionHandler(ExceptionHandler handler) { exceptionHandler_ = handler; }

void Deserializer::convertVersion(VersionConverter* converter) { converter->convert(rootElement_); }

void Deserializer::handleError(const SourceContext& context) {
    if (exceptionHandler_) {
        exceptionHandler_(context);
    } else {  // If no error handler found:
        try {
            throw;
        } catch (SerializationException& e) {
            log::report(*this, LogLevel::Warn, e.getContext(), e.getMessage());
        }
    }
}

TiXmlElement* Deserializer::retrieveChild(std::string_view key) const {
    return retrieveChild_ ? rootElement_->FirstChildElement(key) : rootElement_;
}

void Deserializer::registerFactory(FactoryBase* factory) {
    registeredFactories_.push_back(factory);
}

int Deserializer::getVersion() const { return version_; }

std::optional<std::string_view> detail::attribute(const TiXmlElement* node, std::string_view key) {
    return node->Attribute(key);
}

std::string_view detail::getAttribute(const TiXmlElement& node, std::string_view key) {
    static const std::string empty;

    if (auto str = node.Attribute(key)) {
        return *str;
    } else {
        return empty;
    }
}

std::optional<std::string_view> Deserializer::attribute(std::string_view key) const {
    return detail::attribute(rootElement_, key);
}

std::optional<std::string_view> Deserializer::attribute(std::string_view child,
                                                        std::string_view key) const {
    if (auto childNode = retrieveChild(child)) {
        return detail::attribute(childNode, key);
    } else {
        return std::nullopt;
    }
}

bool Deserializer::hasElement(std::string_view key) const {
    return rootElement_->FirstChildElement(key) != nullptr;
}

TiXmlElement* detail::firstChild(TiXmlElement* parent, std::string_view key) {
    return parent->FirstChildElement(key);
}
TiXmlElement* detail::nextChild(TiXmlElement* child, std::string_view key) {
    return child->NextSiblingElement(key);
}

}  // namespace inviwo
