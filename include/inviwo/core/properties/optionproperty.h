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

#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/optionpropertytraits.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/introspection.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/enumtraits.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/isstreaminsertable.h>
#include <inviwo/core/util/docutils.h>

#include <type_traits>
#include <iterator>
#include <algorithm>
#include <utility>

namespace inviwo {

/**
 *  Base class for the option properties
 *  @see OptionProperty
 */
class IVW_CORE_API BaseOptionProperty : public Property {
public:
    BaseOptionProperty(std::string_view identifier, std::string_view displayName, Document help,
                       InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                       PropertySemantics semantics = PropertySemantics::Default);

    BaseOptionProperty(const BaseOptionProperty& rhs);

    virtual ~BaseOptionProperty();

    virtual std::string_view getClassIdentifier() const override = 0;

    virtual BaseOptionProperty& clearOptions() = 0;

    bool empty() const;
    virtual size_t size() const = 0;
    virtual size_t getSelectedIndex() const = 0;
    virtual const std::string& getSelectedIdentifier() const = 0;
    virtual const std::string& getSelectedDisplayName() const = 0;
    virtual const std::string& getOptionIdentifier(size_t index) const = 0;
    virtual const std::string& getOptionDisplayName(size_t index) const = 0;
    virtual std::vector<std::string> getIdentifiers() const = 0;
    virtual std::vector<std::string> getDisplayNames() const = 0;

    virtual bool setSelectedIndex(size_t index) = 0;
    virtual bool setSelectedIdentifier(std::string_view identifier) = 0;
    virtual bool setSelectedDisplayName(std::string_view name) = 0;

    virtual bool isSelectedIndex(size_t index) const = 0;
    virtual bool isSelectedIdentifier(std::string_view identifier) const = 0;
    virtual bool isSelectedDisplayName(std::string_view name) const = 0;

    void set(const BaseOptionProperty* srcProperty);
    virtual void set(const Property* srcProperty) override;
};

template <typename T>
class OptionPropertyOption : public Serializable {
public:
    OptionPropertyOption();
    OptionPropertyOption(const OptionPropertyOption& rhs);
    OptionPropertyOption(OptionPropertyOption&& rhs) noexcept;
    OptionPropertyOption& operator=(const OptionPropertyOption& that);
    OptionPropertyOption& operator=(OptionPropertyOption&& that) noexcept;

    OptionPropertyOption(std::string_view id, std::string_view name, T value);

    OptionPropertyOption(std::string_view id, std::string_view name)
        requires std::is_same_v<T, std::string>;

    OptionPropertyOption& operator=(const T& that)
        requires(fmt::is_formattable<T>::value);

    OptionPropertyOption& operator=(const T& that)
        requires(!fmt::is_formattable<T>::value && util::is_stream_insertable<T>::value);

    explicit OptionPropertyOption(const T& val)
        requires(fmt::is_formattable<T>::value);

    explicit OptionPropertyOption(const T& val)
        requires(!fmt::is_formattable<T>::value && util::is_stream_insertable<T>::value);

    std::string id_;
    std::string name_;
    T value_ = T{};

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    bool operator==(const OptionPropertyOption<T>& rhs) const;
    bool operator!=(const OptionPropertyOption<T>& rhs) const;
};

/**
 * A helper struct to construct ordinal properties @see OrdinalProperty
 */
template <typename T>
struct OptionPropertyState {
    std::vector<OptionPropertyOption<T>> options = {};
    size_t selectedIndex = 0;
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput;
    PropertySemantics semantics = PropertySemantics::Default;
    Document help = {};

    auto set(std::vector<OptionPropertyOption<T>> newOptions) -> OptionPropertyState {
        options = std::move(newOptions);
        return *this;
    }
    auto set(size_t newSelectedIndex) -> OptionPropertyState {
        selectedIndex = newSelectedIndex;
        return *this;
    }
    auto setSelectedValue(const T& newSelectedValue) -> OptionPropertyState {
        auto it = std::ranges::find(options, newSelectedValue,
                                    [](auto& option) { return option.value_; });
        if (it != std::end(options)) {
            selectedIndex = std::distance(std::begin(options), it);
        }
        return *this;
    }
    auto set(InvalidationLevel newInvalidationLevel) -> OptionPropertyState {
        invalidationLevel = newInvalidationLevel;
        return *this;
    }
    auto set(PropertySemantics newSemantics) -> OptionPropertyState {
        semantics = std::move(newSemantics);
        return *this;
    }
    auto set(Document newHelp) -> OptionPropertyState {
        help = std::move(newHelp);
        return *this;
    }
};

/**
 * OptionProperty with a custom type @p T.
 *
 * For dynamic template option properties, @p T needs to have some traits defining the class
 * identifier. Default identifiers are provided for the standard types defined in defaultvalues.h.
 * If the template parameter @p T is a custom enum class, there needs to be a <tt>EnumTraits<T></tt>
 * struct defining the name of the enum. This enum name then becomes part of the property
 * identifier.
 *
 * @code
 * #include <inviwo/core/util/enumtraits.h>
 *
 * enum class MyEnum { Value1, Value2, Value3 };
 *
 * template <>
 * struct EnumTraits<MyEnum> {
 *     static std::string_view name() { return "MyEnum"; }
 * };
 *
 * registerProperty<OptionProperty<MyEnum>>();
 * @endcode
 *
 * For other data types, the class identifier must be provided using @c PropertyTraits.
 * @code
 * template <>
 * struct PropertyTraits<OptionProperty<MyType>>
 *     static constexpr std::string_view classIdentifier() {
 *         return "org.inviwo.OptionPropertyMyType";
 *     }
 * };
 * @endcode
 *
 * @tparam T  internal value type of the option property
 * @see defaultvalues.h enumtraits.h propertytraits.h
 */
template <typename T>
class OptionProperty : public BaseOptionProperty {

public:
    using value_type = T;

    OptionProperty(std::string_view identifier, std::string_view displayName,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    OptionProperty(std::string_view identifier, std::string_view displayName, Document help,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    OptionProperty(std::string_view identifier, std::string_view displayName,
                   const std::vector<OptionPropertyOption<T>>& options, size_t selectedIndex = 0,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    OptionProperty(std::string_view identifier, std::string_view displayName, Document help,
                   const std::vector<OptionPropertyOption<T>>& options, size_t selectedIndex = 0,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    OptionProperty(std::string_view identifier, std::string_view displayName,
                   OptionPropertyState<T> state);

    template <typename U = T>
    OptionProperty(std::string_view identifier, std::string_view displayName,
                   const std::vector<U>& options, size_t selectedIndex = 0,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default)
        requires(fmt::is_formattable<U>::value || util::is_stream_insertable<U>::value);

    template <typename U = T>
    OptionProperty(std::string_view identifier, std::string_view displayName, Document help,
                   const std::vector<T>& options, size_t selectedIndex = 0,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default)
        requires(fmt::is_formattable<U>::value || util::is_stream_insertable<U>::value);

    OptionProperty(const OptionProperty<T>& rhs);

    virtual OptionProperty<T>* clone() const override;
    virtual ~OptionProperty();

    virtual std::string_view getClassIdentifier() const override;

    /**
     * Implicit conversion operator. The OptionProperty will implicitly be converted to T when
     * possible.
     */
    operator const T&() const;

    operator std::string_view() const
        requires std::is_same_v<T, std::string>;

    /**
     * \brief Adds an option to the property
     *
     * Adds a option to the property and stores it as a struct in the options_
     * The option name is the name of the option that will be displayed in the widget.
     */
    OptionProperty& addOption(std::string_view identifier, std::string_view displayName,
                              const T& value);

    OptionProperty& addOption(std::string_view identifier, std::string_view displayName)
        requires std::is_same_v<T, std::string>;

    OptionProperty& addOption(std::string_view identifier, std::string_view displayName,
                              std::string_view value)
        requires std::is_same_v<T, std::string>;

    OptionProperty& addOption(T value)
        requires(fmt::is_formattable<T>::value || util::is_stream_insertable<T>::value);

    virtual OptionProperty& removeOption(std::string_view identifier);
    virtual OptionProperty& removeOption(size_t index);
    virtual OptionProperty& clearOptions() override;

    virtual size_t size() const override;
    virtual size_t getSelectedIndex() const override;
    virtual const std::string& getSelectedIdentifier() const override;
    virtual const std::string& getSelectedDisplayName() const override;
    const T& getSelectedValue() const;
    virtual std::vector<std::string> getIdentifiers() const override;
    virtual std::vector<std::string> getDisplayNames() const override;
    std::vector<T> getValues() const;
    const std::vector<OptionPropertyOption<T>>& getOptions() const;

    virtual const std::string& getOptionIdentifier(size_t index) const override;
    virtual const std::string& getOptionDisplayName(size_t index) const override;
    const T& getOptionValue(size_t index) const;
    const OptionPropertyOption<T>& getOptions(size_t index) const;

    virtual bool setSelectedIndex(size_t index) override;
    virtual bool setSelectedIdentifier(std::string_view identifier) override;
    virtual bool setSelectedDisplayName(std::string_view name) override;

    /**
     * @brief Set the selected index to that of the value provided if found in the list of options
     * @param val to set
     * @return True if the value was found in the list of options else false.
     */
    bool setSelectedValue(const T& val);

    /**
     * @brief Set the selected index to that of the value provided if found in the list of options
     * @param value to set
     * @return True if the value was found in the list of options else false.
     */
    bool setSelectedValue(std::string_view value)
        requires std::is_same_v<T, std::string>;

    virtual OptionProperty& replaceOptions(const std::vector<std::string>& ids,
                                           const std::vector<std::string>& displayNames,
                                           const std::vector<T>& values);
    virtual OptionProperty& replaceOptions(std::vector<OptionPropertyOption<T>> options);

    OptionProperty& replaceOptions(const std::vector<T>& values)
        requires(fmt::is_formattable<T>::value || util::is_stream_insertable<T>::value);

    template <typename Func>
        requires(std::is_invocable_r_v<bool, Func, std::vector<OptionPropertyOption<T>>&>)
    OptionProperty& updateOptions(Func&& updater);

    virtual bool isSelectedIndex(size_t index) const override;
    virtual bool isSelectedIdentifier(std::string_view identifier) const override;
    virtual bool isSelectedDisplayName(std::string_view name) const override;
    bool isSelectedValue(const T& val) const;

    const T& get() const;
    const T& operator*() const;
    const T* operator->() const;

    /**
     * @brief Set the selected index to that of the value provided if found in the list of options
     * @param value to set
     * @return True if the value was found in the list of options else false.
     */
    bool set(const T& value);

    /**
     * @brief Set the selected index to that of the value provided if found in the list of options
     * @param value to set
     * @return True if the value was found in the list of options else false.
     */
    template <typename U = T, class = std::enable_if_t<std::is_same_v<U, std::string>, void>>
    bool set(std::string_view value);

    void set(const OptionProperty* srcProperty);
    virtual void set(const Property* srcProperty) override;

    /**
     * Sets the default state, since the constructor can't add any default state, you must call this
     * function after adding all the default options, usually in the processor constructor.
     * @see Property::setCurrentStateAsDefault()
     */
    virtual OptionProperty& setCurrentStateAsDefault() override;
    OptionProperty<T>& setDefault(const T& value);
    OptionProperty<T>& setDefaultSelectedIndex(size_t index);
    virtual OptionProperty& resetToDefaultState() override;
    virtual bool isDefaultState() const override;

    virtual std::string_view getClassIdentifierForWidget() const override;
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual Document getDescription() const override;

protected:
    const std::vector<OptionPropertyOption<T>>& opts() const {
        if (options_) {
            return *options_;
        } else {
            return defaultOptions_;
        }
    }
    const OptionPropertyOption<T>& opt(size_t index) const {
        auto& options = opts();
        if (index >= options.size()) {
            throw Exception{SourceContext{},
                            "Index out of range (number of options: {}, index: {})", options.size(),
                            index};
        }
        return options[index];
    }
    std::vector<OptionPropertyOption<T>>& optsMut() {
        if (!options_) {
            options_ = defaultOptions_;
        }
        return *options_;
    }
    std::optional<size_t> findId(std::string_view id) {
        auto& options = opts();
        auto it = util::find_if(options, [&](auto& opt) { return opt.id_ == id; });
        if (it != options.end()) {
            return std::distance(options.begin(), it);
        } else {
            return std::nullopt;
        }
    }

    size_t selectedIndex_{};

private:
    std::optional<std::vector<OptionPropertyOption<T>>> options_;

    size_t defaultSelectedIndex_{};
    std::vector<OptionPropertyOption<T>> defaultOptions_;
};

template <typename T, typename U>
bool operator==(const OptionProperty<T>& lhs, const U& rhs) {
    return lhs.get() == rhs;
}
template <typename T, typename U>
bool operator==(const U& lhs, const OptionProperty<T>& rhs) {
    return lhs == rhs.get();
}

template <typename T, typename U>
bool operator!=(const OptionProperty<T>& lhs, const U& rhs) {
    return lhs.get() != rhs;
}
template <typename T, typename U>
bool operator!=(const U& lhs, const OptionProperty<T>& rhs) {
    return lhs != rhs.get();
}

template <typename T>
struct PropertyTraits<OptionProperty<T>> {
    static std::string_view classIdentifier() { return OptionPropertyTraits<T>::classIdentifier(); }
};

template <typename T>
std::string_view OptionProperty<T>::getClassIdentifier() const {
    return PropertyTraits<OptionProperty<T>>::classIdentifier();
}

template <typename T>
std::string_view OptionProperty<T>::getClassIdentifierForWidget() const {
    return PropertyTraits<OptionProperty<std::string>>::classIdentifier();
}

// Legacy name
template <typename T>
using TemplateOptionProperty [[deprecated("Use `OptionProperty` instead")]] = OptionProperty<T>;

using OptionPropertyUIntOption = OptionPropertyOption<unsigned int>;
using OptionPropertyIntOption = OptionPropertyOption<int>;
using OptionPropertySize_tOption = OptionPropertyOption<size_t>;
using OptionPropertyFloatOption = OptionPropertyOption<float>;
using OptionPropertyDoubleOption = OptionPropertyOption<double>;
using OptionPropertyStringOption = OptionPropertyOption<std::string>;

using OptionPropertyUInt = OptionProperty<unsigned int>;
using OptionPropertyInt = OptionProperty<int>;
using OptionPropertySize_t = OptionProperty<size_t>;
using OptionPropertyFloat = OptionProperty<float>;
using OptionPropertyDouble = OptionProperty<double>;
using OptionPropertyString = OptionProperty<std::string>;

namespace util {

/**
 * A factory function for setting up a OptionPropertyInt with enumerated options. That is,
 * "@p name 1", "@p name 2", etc. up to @p count.
 */
IVW_CORE_API std::vector<OptionPropertyIntOption> enumeratedOptions(std::string_view name,
                                                                    size_t count, int start = 0,
                                                                    int step = 1);

}  // namespace util

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption() = default;

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(const OptionPropertyOption& rhs) = default;
template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(OptionPropertyOption&& rhs) noexcept = default;
template <typename T>
OptionPropertyOption<T>& OptionPropertyOption<T>::operator=(const OptionPropertyOption& that) =
    default;
template <typename T>
OptionPropertyOption<T>& OptionPropertyOption<T>::operator=(OptionPropertyOption&& that) noexcept =
    default;

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(std::string_view id, std::string_view name, T value)
    : id_(id), name_(name), value_(std::move(value)) {}

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(std::string_view id, std::string_view name)
    requires std::is_same_v<T, std::string>
    : id_(id), name_(name), value_(id) {}

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(const T& val)
    requires(fmt::is_formattable<T>::value)
    : id_(fmt::to_string(val)), name_(camelCaseToHeader(id_)), value_(val) {}

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(const T& val)
    requires(!fmt::is_formattable<T>::value && util::is_stream_insertable<T>::value)
    : id_(toString(val)), name_(camelCaseToHeader(id_)), value_(val) {}

template <typename T>
OptionPropertyOption<T>& OptionPropertyOption<T>::operator=(const T& that)
    requires(fmt::is_formattable<T>::value)
{
    if (value_ != that) {
        id_.clear();
        fmt::format_to(std::back_inserter(id_), "{}", that);
        name_.clear();
        camelCaseToHeader(id_, name_);
        value_ = that;
    }
    return *this;
}

template <typename T>
OptionPropertyOption<T>& OptionPropertyOption<T>::operator=(const T& that)
    requires(!fmt::is_formattable<T>::value && util::is_stream_insertable<T>::value)
{
    if (value_ != that) {
        id_ = toString(that);
        name_.clear();
        camelCaseToHeader(id_, name_);
        value_ = that;
    }
    return *this;
}

template <typename T>
void OptionPropertyOption<T>::serialize(Serializer& s) const {
    s.serialize("id", id_);
    s.serialize("name", name_);
    s.serialize("value", value_);
}

template <typename T>
void OptionPropertyOption<T>::deserialize(Deserializer& d) {
    d.deserialize("id", id_);
    d.deserialize("name", name_);
    d.deserialize("value", value_);
}

template <typename T>
bool OptionPropertyOption<T>::operator==(const OptionPropertyOption<T>& rhs) const {
    return id_ == rhs.id_ && name_ == rhs.name_ && value_ == rhs.value_;
}

template <typename T>
bool OptionPropertyOption<T>::operator!=(const OptionPropertyOption<T>& rhs) const {
    return !operator==(rhs);
}

template <typename T>
OptionProperty<T>::OptionProperty(std::string_view identifier, std::string_view displayName,
                                  Document help,
                                  const std::vector<OptionPropertyOption<T>>& options,
                                  size_t selectedIndex, InvalidationLevel invalidationLevel,
                                  PropertySemantics semantics)
    : BaseOptionProperty{identifier, displayName, std::move(help), invalidationLevel, semantics}
    , selectedIndex_{std::min(selectedIndex, options.size() - 1)}
    , options_{std::nullopt}
    , defaultSelectedIndex_{selectedIndex_}
    , defaultOptions_{options} {}

template <typename T>
OptionProperty<T>::OptionProperty(std::string_view identifier, std::string_view displayName,
                                  OptionPropertyState<T> state)
    : BaseOptionProperty{identifier, displayName, std::move(state.help), state.invalidationLevel,
                         state.semantics}
    , selectedIndex_{state.selectedIndex}
    , options_{std::nullopt}
    , defaultSelectedIndex_{selectedIndex_}
    , defaultOptions_{std::move(state.options)} {}

template <typename T>
OptionProperty<T>::OptionProperty(std::string_view identifier, std::string_view displayName,
                                  Document help, InvalidationLevel invalidationLevel,
                                  PropertySemantics semantics)
    : OptionProperty{
          identifier, displayName,       std::move(help), std::vector<OptionPropertyOption<T>>{},
          0,          invalidationLevel, semantics} {}

template <typename T>
OptionProperty<T>::OptionProperty(std::string_view identifier, std::string_view displayName,
                                  InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : OptionProperty{identifier, displayName, Document{}, invalidationLevel, semantics} {}

template <typename T>
OptionProperty<T>::OptionProperty(std::string_view identifier, std::string_view displayName,
                                  const std::vector<OptionPropertyOption<T>>& options,
                                  size_t selectedIndex, InvalidationLevel invalidationLevel,
                                  PropertySemantics semantics)
    : OptionProperty{identifier,    displayName,       Document{}, options,
                     selectedIndex, invalidationLevel, semantics} {}

template <typename T>
template <typename U>
OptionProperty<T>::OptionProperty(std::string_view identifier, std::string_view displayName,
                                  const std::vector<U>& options, size_t selectedIndex,
                                  InvalidationLevel invalidationLevel, PropertySemantics semantics)
    requires(fmt::is_formattable<U>::value || util::is_stream_insertable<U>::value)
    : OptionProperty{identifier,    displayName,       {},       options,
                     selectedIndex, invalidationLevel, semantics} {}

template <typename T>
template <typename U>
OptionProperty<T>::OptionProperty(std::string_view identifier, std::string_view displayName,
                                  Document help, const std::vector<T>& options,
                                  size_t selectedIndex, InvalidationLevel invalidationLevel,
                                  PropertySemantics semantics)
    requires(fmt::is_formattable<U>::value || util::is_stream_insertable<U>::value)
    : BaseOptionProperty{identifier, displayName, std::move(help), invalidationLevel, semantics}
    , selectedIndex_{std::min(selectedIndex, options.size() - 1)}
    , options_{std::nullopt}
    , defaultSelectedIndex_{selectedIndex_}
    , defaultOptions_{} {

    for (const auto& option : options) {
        defaultOptions_.emplace_back(option);
    }
}

template <typename T>
OptionProperty<T>::OptionProperty(const OptionProperty<T>& rhs) = default;

template <typename T>
OptionProperty<T>::~OptionProperty() = default;

template <typename T>
OptionProperty<T>& OptionProperty<T>::addOption(std::string_view identifier,
                                                std::string_view displayName, const T& value) {
    auto& options = optsMut();
    options.emplace_back(identifier, displayName, value);

    // in case we add the first option, we also select it
    if (options.size() == 1) {
        selectedIndex_ = 0;
    }

    propertyModified();
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::addOption(std::string_view identifier,
                                                std::string_view displayName)
    requires std::is_same_v<T, std::string>
{
    addOption(identifier, displayName, identifier);
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::addOption(std::string_view identifier,
                                                std::string_view displayName,
                                                std::string_view value)
    requires std::is_same_v<T, std::string>
{
    auto& options = optsMut();
    options.emplace_back(identifier, displayName, std::string{value});

    // in case we add the first option, we also select it
    if (options.size() == 1) {
        selectedIndex_ = 0;
    }

    propertyModified();
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::addOption(T value)
    requires(fmt::is_formattable<T>::value || util::is_stream_insertable<T>::value)
{
    auto& options = optsMut();
    options.emplace_back(value);

    // in case we add the first option, we also select it
    if (options.size() == 1) {
        selectedIndex_ = 0;
    }

    propertyModified();
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::removeOption(size_t index) {
    if (opts().empty()) return *this;

    auto& options = optsMut();
    std::string id = getSelectedIdentifier();
    options.erase(options.begin() + index);
    if (!setSelectedIdentifier(id)) {
        selectedIndex_ = 0;
    }
    propertyModified();
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::removeOption(std::string_view identifier) {
    if (opts().empty()) return *this;

    auto& options = optsMut();
    std::string id = getSelectedIdentifier();
    std::erase_if(options,
                  [&](const OptionPropertyOption<T>& opt) { return opt.id_ == identifier; });
    if (!setSelectedIdentifier(id)) {
        selectedIndex_ = 0;
    }
    propertyModified();
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::clearOptions() {
    if (options_) {
        options_->clear();
    } else {
        options_.emplace();
    }

    selectedIndex_ = 0;
    return *this;
}

// Getters
template <typename T>
size_t OptionProperty<T>::size() const {
    return opts().size();
}

template <typename T>
size_t OptionProperty<T>::getSelectedIndex() const {
    return selectedIndex_;
}

template <typename T>
const std::string& OptionProperty<T>::getSelectedIdentifier() const {
    return opt(selectedIndex_).id_;
}

template <typename T>
const std::string& OptionProperty<T>::getSelectedDisplayName() const {
    return opt(selectedIndex_).name_;
}

template <typename T>
const T& OptionProperty<T>::getSelectedValue() const {
    return opt(selectedIndex_).value_;
}

template <typename T>
OptionProperty<T>::operator const T&() const {
    return opt(selectedIndex_).value_;
}

template <typename T>
OptionProperty<T>::operator std::string_view() const
    requires std::is_same_v<T, std::string>
{
    return opt(selectedIndex_).value_;
}

template <typename T>
std::vector<std::string> OptionProperty<T>::getIdentifiers() const {
    std::vector<std::string> result;
    result.reserve(opts().size());
    std::ranges::transform(opts(), std::back_inserter(result), &OptionPropertyOption<T>::id_);
    return result;
}

template <typename T>
std::vector<std::string> OptionProperty<T>::getDisplayNames() const {
    std::vector<std::string> result;
    result.reserve(opts().size());
    std::ranges::transform(opts(), std::back_inserter(result), &OptionPropertyOption<T>::name_);
    return result;
}

template <typename T>
std::vector<T> OptionProperty<T>::getValues() const {
    std::vector<T> result;
    result.reserve(opts().size());
    std::ranges::transform(opts(), std::back_inserter(result), &OptionPropertyOption<T>::value_);
    return result;
}

template <typename T>
const std::vector<OptionPropertyOption<T>>& OptionProperty<T>::getOptions() const {
    return opts();
}

template <typename T>
const std::string& OptionProperty<T>::getOptionIdentifier(size_t index) const {
    return opt(index).id_;
}
template <typename T>
const std::string& OptionProperty<T>::getOptionDisplayName(size_t index) const {
    return opt(index).name_;
}
template <typename T>
const T& OptionProperty<T>::getOptionValue(size_t index) const {
    return opt(index).value_;
}
template <typename T>
const OptionPropertyOption<T>& OptionProperty<T>::getOptions(size_t index) const {
    return opt(index);
}

// Setters
template <typename T>
bool OptionProperty<T>::setSelectedIndex(size_t option) {
    if (selectedIndex_ == option) {
        return true;
    } else if (option < opts().size()) {
        selectedIndex_ = option;
        propertyModified();
        return true;
    } else {
        return false;
    }
}

template <typename T>
bool OptionProperty<T>::setSelectedIdentifier(std::string_view identifier) {
    auto it = util::find_if(opts(), [&](auto& opt) { return opt.id_ == identifier; });
    if (it != opts().end()) {
        size_t dist = std::distance(opts().begin(), it);
        if (selectedIndex_ != dist) {
            selectedIndex_ = dist;
            propertyModified();
        }
        return true;
    } else {
        return false;
    }
}

template <typename T>
bool OptionProperty<T>::setSelectedDisplayName(std::string_view name) {
    auto it = util::find_if(opts(), [&](auto& opt) { return opt.name_ == name; });
    if (it != opts().end()) {
        size_t dist = std::distance(opts().begin(), it);
        if (selectedIndex_ != dist) {
            selectedIndex_ = dist;
            propertyModified();
        }
        return true;
    } else {
        return false;
    }
}

template <typename T>
bool OptionProperty<T>::setSelectedValue(const T& val) {
    auto it = util::find_if(opts(), [&](auto& opt) { return opt.value_ == val; });
    if (it != opts().end()) {
        size_t dist = std::distance(opts().begin(), it);
        if (selectedIndex_ != dist) {
            selectedIndex_ = dist;
            propertyModified();
        }
        return true;
    } else {
        return false;
    }
}

template <typename T>
bool OptionProperty<T>::setSelectedValue(std::string_view val)
    requires std::is_same_v<T, std::string>
{
    auto it = util::find_if(opts(), [&](auto& opt) { return opt.value_ == val; });
    if (it != opts().end()) {
        size_t dist = std::distance(opts().begin(), it);
        if (selectedIndex_ != dist) {
            selectedIndex_ = dist;
            propertyModified();
        }
        return true;
    } else {
        return false;
    }
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::replaceOptions(std::vector<OptionPropertyOption<T>> options) {
    if (options != opts()) {

        std::string selectId{};
        if (!opts().empty()) selectId = getSelectedIdentifier();

        options_ = std::move(options);
        selectedIndex_ = findId(selectId).value_or(size_t{0});
        propertyModified();
    }
    return *this;
}
template <typename T>
template <typename Func>
    requires(std::is_invocable_r_v<bool, Func, std::vector<OptionPropertyOption<T>>&>)
OptionProperty<T>& OptionProperty<T>::updateOptions(Func&& updater) {

    std::string selectId{};
    if (!opts().empty()) selectId = getSelectedIdentifier();

    if (std::invoke(std::forward<Func>(updater), optsMut())) {
        selectedIndex_ = findId(selectId).value_or(size_t{0});
        propertyModified();
    }
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::replaceOptions(const std::vector<std::string>& ids,
                                                     const std::vector<std::string>& displayNames,
                                                     const std::vector<T>& values) {

    IVW_ASSERT(ids.size() == displayNames.size(), "Arguments has to have the same size");
    IVW_ASSERT(ids.size() == values.size(), "Arguments has to have the same size");

    std::vector<OptionPropertyOption<T>> options;
    for (size_t i = 0; i < ids.size(); i++) {
        options.emplace_back(ids[i], displayNames[i], values[i]);
    }
    return replaceOptions(std::move(options));
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::replaceOptions(const std::vector<T>& values)
    requires(fmt::is_formattable<T>::value || util::is_stream_insertable<T>::value)
{
    std::vector<OptionPropertyOption<T>> options;
    for (size_t i = 0; i < values.size(); i++) {
        options.emplace_back(values[i]);
    }
    return replaceOptions(std::move(options));
}

// Is...
template <typename T>
bool OptionProperty<T>::isSelectedIndex(size_t index) const {
    return index == selectedIndex_;
}

template <typename T>
bool OptionProperty<T>::isSelectedIdentifier(std::string_view identifier) const {
    return identifier == opt(selectedIndex_).id_;
}

template <typename T>
bool OptionProperty<T>::isSelectedDisplayName(std::string_view name) const {
    return name == opt(selectedIndex_).name_;
}

template <typename T>
bool OptionProperty<T>::isSelectedValue(const T& val) const {
    return val == opt(selectedIndex_).value_;
}

// Convenience
template <typename T>
const T& OptionProperty<T>::get() const {
    return opt(selectedIndex_).value_;
}

template <typename T>
const T& OptionProperty<T>::operator*() const {
    return opt(selectedIndex_).value_;
}

template <typename T>
const T* OptionProperty<T>::operator->() const {
    return &(opt(selectedIndex_).value_);
}

template <typename T>
bool OptionProperty<T>::set(const T& value) {
    return setSelectedValue(value);
}

template <typename T>
template <typename U, class>
bool OptionProperty<T>::set(std::string_view value) {
    return setSelectedValue(value);
}

template <typename T>
void OptionProperty<T>::set(const OptionProperty* srcProperty) {
    BaseOptionProperty::set(srcProperty);
}

template <typename T>
void OptionProperty<T>::set(const Property* srcProperty) {
    BaseOptionProperty::set(srcProperty);
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::resetToDefaultState() {
    bool modified = false;

    if (options_) {
        modified = options_ != defaultOptions_;
        options_.reset();
    }
    if (selectedIndex_ != defaultSelectedIndex_) {
        modified = true;
        selectedIndex_ = defaultSelectedIndex_;
    }

    if (defaultOptions_.empty()) {
        log::warn(
            "Resetting option property: {} to an empty option list. Probably the default values "
            "have never been set, Remember to call setCurrentStateAsDefault() after adding all the "
            "options.",
            this->getIdentifier());
    }

    if (modified) this->propertyModified();
    return *this;
}

template <typename T>
bool OptionProperty<T>::isDefaultState() const {
    return selectedIndex_ == defaultSelectedIndex_ && (!options_ || *options_ == defaultOptions_);
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    defaultSelectedIndex_ = selectedIndex_;

    if (options_) {
        defaultOptions_ = std::move(*options_);
        options_.reset();
    }
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::setDefault(const T& value) {
    auto it = util::find_if(opts(), [&](auto& opt) { return opt.value_ == value; });
    if (it != opts().end()) {
        defaultSelectedIndex_ = std::distance(opts().begin(), it);
    }
    return *this;
}

template <typename T>
OptionProperty<T>& OptionProperty<T>::setDefaultSelectedIndex(size_t index) {
    defaultSelectedIndex_ = index;
    return *this;
}

template <typename T>
void OptionProperty<T>::serialize(Serializer& s) const {
    BaseOptionProperty::serialize(s);
    if (this->serializationMode_ == PropertySerializationMode::None) return;

    if ((this->serializationMode_ == PropertySerializationMode::All ||
         (options_ && *options_ != defaultOptions_)) &&
        opts().size() > 0) {
        s.serialize("options", opts(), "option");
    }
    if ((this->serializationMode_ == PropertySerializationMode::All ||
         selectedIndex_ != defaultSelectedIndex_) &&
        opts().size() > 0) {
        s.serialize("selectedIdentifier", getSelectedIdentifier());
    }
}

template <typename T>
void OptionProperty<T>::deserialize(Deserializer& d) {
    BaseOptionProperty::deserialize(d);
    if (serializationMode_ == PropertySerializationMode::None) return;

    const auto indexOf = [this](std::string_view identifier) -> std::optional<size_t> {
        return findId(identifier);
    };

    const auto compareAndSet = [](auto& dst, const auto& src) -> bool {
        if (dst != src) {
            dst = src;
            return true;
        } else {
            return false;
        }
    };

    bool modified = false;
    if (d.hasElement("options")) {
        if (!options_) {
            options_ = defaultOptions_;
        }
        const auto res = d.deserializeTrackChanges("options", *options_, "option");
        modified |= (res == Deserializer::Result::Modified);
    } else if (options_) {
        modified |= *options_ != defaultOptions_;
        options_.reset();
    }

    const size_t newIndex = d.attribute("selectedIdentifier", SerializeConstants::ContentAttribute)
                                .and_then(indexOf)
                                .or_else([&]() -> std::optional<size_t> {
                                    if (serializationMode_ == PropertySerializationMode::Default) {
                                        return defaultSelectedIndex_;
                                    } else if (selectedIndex_ < opts().size()) {
                                        return selectedIndex_;
                                    } else {
                                        return std::nullopt;
                                    }
                                })
                                .value_or(size_t{0});
    modified |= compareAndSet(selectedIndex_, newIndex);
    if (modified) {
        propertyModified();
    }
}

template <typename T>
Document OptionProperty<T>::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = BaseOptionProperty::getDescription();
    if (opts().size() > 0) {
        auto table = doc.get({P("table", {{"class", "propertyInfo"}})});
        utildoc::TableBuilder tb(table);
        tb(H("Number of Options"), opts().size());
        tb(H("Selected Index"), selectedIndex_);
        tb(H("Selected Name"), opt(selectedIndex_).name_);
        tb(H("Selected Value"), opt(selectedIndex_).value_);
    }
    return doc;
}

template <typename T>
OptionProperty<T>* OptionProperty<T>::clone() const {
    return new OptionProperty<T>(*this);
}

/// @cond
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<unsigned int>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<int>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<size_t>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<float>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<double>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<std::string>;

extern template class IVW_CORE_TMPL_EXP OptionProperty<unsigned int>;
extern template class IVW_CORE_TMPL_EXP OptionProperty<int>;
extern template class IVW_CORE_TMPL_EXP OptionProperty<size_t>;
extern template class IVW_CORE_TMPL_EXP OptionProperty<float>;
extern template class IVW_CORE_TMPL_EXP OptionProperty<double>;
extern template class IVW_CORE_TMPL_EXP OptionProperty<std::string>;
/// @endcond

}  // namespace inviwo
