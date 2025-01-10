//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_STREAMABLE_TRAITS_H
#define ICE_STREAMABLE_TRAITS_H

#include "ValueF.h"

#include <optional>
#include <string_view>

namespace Ice
{
    class ObjectPrx;
    class UserException;

    /// \cond STREAM

    /**
     * The stream helper category allows streams to select the desired StreamHelper for a given streamable object.
     */
    using StreamHelperCategory = int;

    /** For types with no specialized trait. */
    const StreamHelperCategory StreamHelperCategoryUnknown = 0;
    /** For built-in types usually passed by value. */
    const StreamHelperCategory StreamHelperCategoryBuiltinValue = 1;
    /** For built-in types usually passed by reference. */
    const StreamHelperCategory StreamHelperCategoryBuiltin = 2;
    /** For struct types. */
    const StreamHelperCategory StreamHelperCategoryStruct = 3;
    /** For enum types. */
    const StreamHelperCategory StreamHelperCategoryEnum = 4;
    /** For sequence types. */
    const StreamHelperCategory StreamHelperCategorySequence = 5;
    /** For dictionary types. */
    const StreamHelperCategory StreamHelperCategoryDictionary = 6;
    /** For proxy types. */
    const StreamHelperCategory StreamHelperCategoryProxy = 7;
    /** For class types. */
    const StreamHelperCategory StreamHelperCategoryClass = 8;
    /** For exception types. */
    const StreamHelperCategory StreamHelperCategoryUserException = 9;

    /**
     * The optional format.
     *
     * Optional data members and parameters are encoded with a specific
     * optional format. This optional format describes how the data is encoded
     * and how it can be skipped by the unmarshaling code if the optional ID
     * isn't known to the receiver.
     */
    enum class OptionalFormat : std::uint8_t
    {
        /** Fixed 1-byte encoding. */
        F1 = 0,
        /** Fixed 2-byte encoding. */
        F2 = 1,
        /** Fixed 4-byte encoding. */
        F4 = 2,
        /** Fixed 8-byte encoding. */
        F8 = 3,
        /** "Size encoding" using 1 to 5 bytes, e.g., enum, class identifier. */
        Size = 4,
        /**
         * "Size encoding" using 1 to 5 bytes followed by data, e.g., string, fixed size
         * struct, or containers whose size can be computed prior to marshaling.
         */
        VSize = 5,
        /** Fixed size using 4 bytes followed by data, e.g., variable-size struct, container. */
        FSize = 6,
        /** Class instance. */
        Class = 7
    };

    /**
     * Determines whether the provided type is a container. For now, the implementation only checks
     * if there is a T::iterator typedef using SFINAE.
     * \headerfile Ice/Ice.h
     */
    template<typename T> struct IsContainer
    {
        template<typename C> static char test(typename C::iterator*) noexcept;

        template<typename C> static long test(...) noexcept;

        static const bool value = sizeof(test<T>(0)) == sizeof(char);
    };

    /**
     * Determines whether the provided type is a map. For now, the implementation only checks if there
     * is a T::mapped_type typedef using SFINAE.
     * \headerfile Ice/Ice.h
     */
    template<typename T> struct IsMap
    {
        template<typename C> static char test(typename C::mapped_type*) noexcept;

        template<typename C> static long test(...) noexcept;

        static const bool value = IsContainer<T>::value && sizeof(test<T>(0)) == sizeof(char);
    };

    /**
     * Base traits template. Types with no specialized trait use this trait.
     * \headerfile Ice/Ice.h
     */
    template<typename T, typename Enabler = void> struct StreamableTraits
    {
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

    /**
     * Specialization for sequence and dictionary types.
     * \headerfile Ice/Ice.h
     */
    template<typename T>
    struct StreamableTraits<T, std::enable_if_t<IsMap<T>::value || IsContainer<T>::value>>
    {
        static const StreamHelperCategory helper =
            IsMap<T>::value ? StreamHelperCategoryDictionary : StreamHelperCategorySequence;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    /**
     * Specialization for exceptions.
     * \headerfile Ice/Ice.h
     */
    template<typename T>
    struct StreamableTraits<T, std::enable_if_t<std::is_base_of_v<UserException, T>>>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryUserException;

        //
        // There is no sequence/dictionary of UserException (so no need for minWireSize)
        // and no optional UserException (so no need for fixedLength)
        //
    };

    /**
     * Specialization for arrays (std::pair<const T*, const T*>).
     * \headerfile Ice/Ice.h
     */
    template<typename T> struct StreamableTraits<std::pair<T*, T*>>
    {
        static const StreamHelperCategory helper = StreamHelperCategorySequence;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<bool>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 1;
        static const bool fixedLength = true;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::byte>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 1;
        static const bool fixedLength = true;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::uint8_t>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 1;
        static const bool fixedLength = true;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::int16_t>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 2;
        static const bool fixedLength = true;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::int32_t>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 4;
        static const bool fixedLength = true;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::int64_t>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 8;
        static const bool fixedLength = true;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<float>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 4;
        static const bool fixedLength = true;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<double>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 8;
        static const bool fixedLength = true;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::string>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::string_view>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::wstring>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    /**
     * Specialization for built-in type (this is needed for sequence
     * marshaling to figure out the minWireSize of each type).
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::wstring_view>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltinValue;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    /**
     * vector<bool> is a special type in C++: the streams handle it like a built-in type.
     * \headerfile Ice/Ice.h
     */
    template<> struct StreamableTraits<std::vector<bool>>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    /**
     * Specialization for proxy types.
     * \headerfile Ice/Ice.h
     */
    template<typename T>
    struct StreamableTraits<std::optional<T>, std::enable_if_t<std::is_base_of_v<ObjectPrx, T>>>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryProxy;
        static const int minWireSize = 2;
        static const bool fixedLength = false;
    };

    /**
     * Specialization for class types.
     * \headerfile Ice/Ice.h
     */
    template<typename T>
    struct StreamableTraits<std::shared_ptr<T>, std::enable_if_t<std::is_base_of_v<Value, T>>>
    {
        static const StreamHelperCategory helper = StreamHelperCategoryClass;
        static const int minWireSize = 1;
        static const bool fixedLength = false;
    };

    template<typename T, StreamHelperCategory st> struct StreamHelper;
    template<typename T, StreamHelperCategory st, bool fixedLength> struct StreamOptionalHelper;
    /// \endcond
}

#endif
