// Copyright (c) ZeroC, Inc.

#ifndef ICE_STREAMABLE_TRAITS_H
#define ICE_STREAMABLE_TRAITS_H

#include "ValueF.h"

#include <optional>
#include <string_view>

namespace Ice
{
    class ObjectPrx;
    class UserException;

    /// The stream helper category allows to select a StreamHelper specialization for a specific category of types.
    /// The possible values are #StreamHelperCategoryUnknown, #StreamHelperCategoryBuiltinValue,
    /// #StreamHelperCategoryBuiltin, #StreamHelperCategoryStruct, #StreamHelperCategoryEnum,
    /// #StreamHelperCategorySequence, #StreamHelperCategoryDictionary, #StreamHelperCategoryProxy,
    /// #StreamHelperCategoryClass, or #StreamHelperCategoryUserException. You can also define your own value if you
    /// want to add support for a new type category.
    using StreamHelperCategory = int;

    /// Types with no StreamHelper specialization.
    constexpr StreamHelperCategory StreamHelperCategoryUnknown = 0;

    /// Built-in types usually passed by value.
    constexpr StreamHelperCategory StreamHelperCategoryBuiltinValue = 1;

    /// Built-in types usually passed by reference.
    constexpr StreamHelperCategory StreamHelperCategoryBuiltin = 2;

    /// Generated struct types.
    constexpr StreamHelperCategory StreamHelperCategoryStruct = 3;

    /// Generated enum types.
    constexpr StreamHelperCategory StreamHelperCategoryEnum = 4;

    /// Sequence types.
    constexpr StreamHelperCategory StreamHelperCategorySequence = 5;

    /// Dictionary types.
    constexpr StreamHelperCategory StreamHelperCategoryDictionary = 6;

    /// Proxy types.
    constexpr StreamHelperCategory StreamHelperCategoryProxy = 7;

    /// Generated class types.
    constexpr StreamHelperCategory StreamHelperCategoryClass = 8;

    /// User exception types.
    constexpr StreamHelperCategory StreamHelperCategoryUserException = 9;

    /// The optional format, used for marshaling optional fields and arguments.
    /// It describes how the data is marshaled and how it can be skipped by the unmarshaling code when the tag isn't
    /// known to the receiver.
    enum class OptionalFormat : std::uint8_t
    {
        /// Fixed 1-byte encoding.
        F1 = 0,

        /// Fixed 2-byte encoding.
        F2 = 1,

        /// Fixed 4-byte encoding.
        F4 = 2,

        /// Fixed 8-byte encoding.
        F8 = 3,

        /// "Size encoding" using 1 to 5 bytes, e.g., enum, class identifier.
        Size = 4,

        /// "Size encoding" using 1 to 5 bytes followed by data, e.g., string, fixed size struct, or containers whose
        /// size can be computed prior to marshaling.
        VSize = 5,

        /// Fixed size using 4 bytes followed by data, e.g., variable-size struct, container.
        FSize = 6,

        /// Class instance. Not longer supported.
        Class = 7
    };

    /// @private
    /// Determines whether the provided type is a container. The implementation checks if there is a @p T::iterator
    /// typedef using SFINAE.
    /// @tparam T The type to check
    template<typename T> struct IsContainer
    {
        template<typename C> static char test(typename C::iterator*) noexcept;

        template<typename C> static long test(...) noexcept;

        static constexpr bool value = sizeof(test<T>(nullptr)) == sizeof(char);
    };

    /// @private
    /// Determines whether the provided type is a map. The implementation checks if there is a @p T::mapped_type typedef
    /// using SFINAE.
    /// @tparam T The type to check
    template<typename T> struct IsMap
    {
        template<typename C> static char test(typename C::mapped_type*) noexcept;

        template<typename C> static long test(...) noexcept;

        static constexpr bool value = IsContainer<T>::value && sizeof(test<T>(nullptr)) == sizeof(char);
    };

    /// @private
    /// Base traits template. Types with no specialized trait use this trait.
    template<typename T, typename Enabler = void> struct StreamableTraits
    {
        /// The category trait, used for selecting the appropriate StreamHelper.
        static constexpr StreamHelperCategory helper = StreamHelperCategoryUnknown;

        // When extracting a sequence<T> from a stream, we can ensure the
        // stream has at least StreamableTraits<T>::minWireSize * size bytes
        // For containers, the minWireSize is 1 (just 1 byte for an empty container).

        /// The minimum number of bytes needed to marshal this type.
        // static constexpr int minWireSize;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        // Only used for marshaling/unmarshaling optional data members and parameters.
        // static constexpr bool fixedLength;
    };

    /// @private
    /// Specialization for sequence and dictionary types.
    template<typename T> struct StreamableTraits<T, std::enable_if_t<IsMap<T>::value || IsContainer<T>::value>>
    {
        static constexpr StreamHelperCategory helper =
            IsMap<T>::value ? StreamHelperCategoryDictionary : StreamHelperCategorySequence;

        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = false;
    };

    /// @private
    template<typename T> struct StreamableTraits<T, std::enable_if_t<std::is_base_of_v<UserException, T>>>
    {
        /// @copydoc StreamableTraits::helper
        static constexpr StreamHelperCategory helper = StreamHelperCategoryUserException;

        // There is no sequence/dictionary of UserException (so no need for minWireSize) and no optional UserException
        // (so no need for fixedLength)
    };

    /// @private
    /// Specialization for arrays (std::pair<const T*, const T*>).
    template<typename T> struct StreamableTraits<std::pair<T*, T*>>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategorySequence;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = false;
    };

    //
    // Specialization for built-in types (this is needed for sequence marshaling to figure out the minWireSize of each
    // type).
    //

    /// @private
    template<> struct StreamableTraits<bool>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = true;
    };

    /// @private
    template<> struct StreamableTraits<std::byte>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = true;
    };

    /// @private
    template<> struct StreamableTraits<std::uint8_t>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = true;
    };

    /// @private
    template<> struct StreamableTraits<std::int16_t>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 2;
        static constexpr bool fixedLength = true;
    };

    /// @private
    template<> struct StreamableTraits<std::int32_t>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 4;
        static constexpr bool fixedLength = true;
    };

    /// @private
    template<> struct StreamableTraits<std::int64_t>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 8;
        static constexpr bool fixedLength = true;
    };

    /// @private
    template<> struct StreamableTraits<float>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 4;
        static constexpr bool fixedLength = true;
    };

    /// @private
    template<> struct StreamableTraits<double>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 8;
        static constexpr bool fixedLength = true;
    };

    /// @private
    template<> struct StreamableTraits<std::string>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltin;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = false;
    };

    /// @private
    template<> struct StreamableTraits<std::string_view>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = false;
    };

    /// @private
    template<> struct StreamableTraits<std::wstring>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltin;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = false;
    };

    /// @private
    template<> struct StreamableTraits<std::wstring_view>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = false;
    };

    /// @private
    /// vector<bool> is a special type in C++: the streams handle it like a built-in type.
    template<> struct StreamableTraits<std::vector<bool>>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryBuiltin;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = false;
    };

    //
    // Specialization for proxies and classes.
    //

    /// @private
    template<typename T> struct StreamableTraits<std::optional<T>, std::enable_if_t<std::is_base_of_v<ObjectPrx, T>>>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryProxy;
        static constexpr int minWireSize = 2;
        static constexpr bool fixedLength = false;
    };

    /// @private
    template<typename T> struct StreamableTraits<std::shared_ptr<T>, std::enable_if_t<std::is_base_of_v<Value, T>>>
    {
        static constexpr StreamHelperCategory helper = StreamHelperCategoryClass;
        static constexpr int minWireSize = 1;
        static constexpr bool fixedLength = false;
    };

    template<typename T, StreamHelperCategory st> struct StreamHelper;

    /// @private
    template<typename T, StreamHelperCategory st, bool fixedLength> struct StreamOptionalHelper;
}

#endif
