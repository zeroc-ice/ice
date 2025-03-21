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

    /// The stream helper category allows streams to select the desired StreamHelper for a given streamable object.
    using StreamHelperCategory = int;

    /// For types with no specialized trait.
    const StreamHelperCategory StreamHelperCategoryUnknown = 0;
    /// For built-in types usually passed by value.
    const StreamHelperCategory StreamHelperCategoryBuiltinValue = 1;
    /// For built-in types usually passed by reference.
    const StreamHelperCategory StreamHelperCategoryBuiltin = 2;
    /// For struct types.
    const StreamHelperCategory StreamHelperCategoryStruct = 3;
    /// For enum types.
    const StreamHelperCategory StreamHelperCategoryEnum = 4;
    /// For sequence types.
    const StreamHelperCategory StreamHelperCategorySequence = 5;
    /// For dictionary types.
    const StreamHelperCategory StreamHelperCategoryDictionary = 6;
    /// For proxy types.
    const StreamHelperCategory StreamHelperCategoryProxy = 7;
    /// For class types.
    const StreamHelperCategory StreamHelperCategoryClass = 8;
    /// For exception types.
    const StreamHelperCategory StreamHelperCategoryUserException = 9;

    /// The optional format.
    ///
    /// Optional data members and parameters are encoded with a specific
    /// optional format. This optional format describes how the data is encoded
    /// and how it can be skipped by the unmarshaling code if the optional ID
    /// isn't known to the receiver.
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
        /// "Size encoding" using 1 to 5 bytes followed by data, e.g., string, fixed size
        /// struct, or containers whose size can be computed prior to marshaling.
        VSize = 5,
        /// Fixed size using 4 bytes followed by data, e.g., variable-size struct, container.
        FSize = 6,
        /// Class instance.
        Class = 7
    };

    /// Determines whether the provided type is a container. The implementation checks if there is a @p T::iterator
    /// typedef using SFINAE.
    /// @tparam T The type to check
    /// \headerfile Ice/Ice.h
    template<typename T> struct IsContainer
    {
        /// @private
        template<typename C> static char test(typename C::iterator*) noexcept;

        /// @private
        template<typename C> static long test(...) noexcept;

        /// `true` when @p T is a container, otherwise `false`.
        static const bool value = sizeof(test<T>(nullptr)) == sizeof(char);
    };

    /// Determines whether the provided type is a map. The implementation checks if there is a @p T::mapped_type typedef
    /// using SFINAE.
    /// @tparam T The type to check
    /// \headerfile Ice/Ice.h
    template<typename T> struct IsMap
    {
        /// @private
        template<typename C> static char test(typename C::mapped_type*) noexcept;

        /// @private
        template<typename C> static long test(...) noexcept;

        /// `true` when @p T is a map, otherwise `false`.
        static const bool value = IsContainer<T>::value && sizeof(test<T>(nullptr)) == sizeof(char);
    };

    /// Base traits template. Types with no specialized trait use this trait.
    /// @tparam T The type.
    /// @tparam Enabler The specialization enabler.
    /// \headerfile Ice/Ice.h
    template<typename T, typename Enabler = void> struct StreamableTraits
    {
        /// The category trait, used for selecting the appropriate StreamHelper.
        static const StreamHelperCategory helper = StreamHelperCategoryUnknown;

        //
        // When extracting a sequence<T> from a stream, we can ensure the
        // stream has at least StreamableTraits<T>::minWireSize * size bytes
        // For containers, the minWireSize is 1 (just 1 byte for an empty container).
        //
        // static const int minWireSize = 1;

        //
        // Is this type encoded on a fixed number of bytes?
        // Used only for marshaling/unmarshaling optional data members and parameters.
        //
        // static const bool fixedLength = false;
    };

    /// Specialization for sequence and dictionary types.
    /// \headerfile Ice/Ice.h
    template<typename T> struct StreamableTraits<T, std::enable_if_t<IsMap<T>::value || IsContainer<T>::value>>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper =
            IsMap<T>::value ? StreamHelperCategoryDictionary : StreamHelperCategorySequence;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = false;
    };

    /// Specialization for exceptions.
    /// \headerfile Ice/Ice.h
    template<typename T> struct StreamableTraits<T, std::enable_if_t<std::is_base_of_v<UserException, T>>>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryUserException;

        //
        // There is no sequence/dictionary of UserException (so no need for minWireSize)
        // and no optional UserException (so no need for fixedLength)
        //
    };

    /// Specialization for arrays (std::pair<const T*, const T*>).
    /// \headerfile Ice/Ice.h
    template<typename T> struct StreamableTraits<std::pair<T*, T*>>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategorySequence;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = false;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<bool>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = true;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::byte>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = true;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::uint8_t>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = true;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::int16_t>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 2;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = true;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::int32_t>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 4;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = true;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::int64_t>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 8;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = true;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<float>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 4;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = true;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<double>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 8;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = true;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::string>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = false;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::string_view>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = false;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::wstring>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    /// Specialization for built-in type (this is needed for sequence
    /// marshaling to figure out the minWireSize of each type).
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::wstring_view>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = false;
    };

    /// vector<bool> is a special type in C++: the streams handle it like a built-in type.
    /// \headerfile Ice/Ice.h
    template<> struct StreamableTraits<std::vector<bool>>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = false;
    };

    /// Specialization for proxy types.
    /// \headerfile Ice/Ice.h
    template<typename T> struct StreamableTraits<std::optional<T>, std::enable_if_t<std::is_base_of_v<ObjectPrx, T>>>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryProxy;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 2;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = false;
    };

    /// Specialization for class types.
    /// \headerfile Ice/Ice.h
    template<typename T> struct StreamableTraits<std::shared_ptr<T>, std::enable_if_t<std::is_base_of_v<Value, T>>>
    {
        /// @copydoc StreamableTraits::helper
        static const StreamHelperCategory helper = StreamHelperCategoryClass;

        /// The minimum number of bytes needed to marshal this type.
        static const int minWireSize = 1;

        /// Indicates if the type is always encoded on a fixed number of bytes.
        static const bool fixedLength = false;
    };

    template<typename T, StreamHelperCategory st> struct StreamHelper;
    template<typename T, StreamHelperCategory st, bool fixedLength> struct StreamOptionalHelper;
}

#endif
