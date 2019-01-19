//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_STREAM_HELPERS_H
#define ICE_STREAM_HELPERS_H

#include <Ice/ObjectF.h>

#ifndef ICE_CPP11_MAPPING
#   include <IceUtil/ScopedArray.h>
#   include <IceUtil/Iterator.h>
#endif

namespace Ice
{

/// \cond STREAM

/**
 * The stream helper category allows streams to select the desired StreamHelper for a given streamable object.
 */
typedef int StreamHelperCategory;

/** For types with no specialized trait. */
const StreamHelperCategory StreamHelperCategoryUnknown = 0;
/** For built-in types. */
const StreamHelperCategory StreamHelperCategoryBuiltin = 1;
/** For struct types. */
const StreamHelperCategory StreamHelperCategoryStruct = 2;
/** For struct types with cpp:class metadata. */
const StreamHelperCategory StreamHelperCategoryStructClass = 3;
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
#ifdef ICE_CPP11_MAPPING
enum class OptionalFormat : unsigned char
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
#else
enum OptionalFormat
{
    /** Fixed 1-byte encoding. */
    OptionalFormatF1 = 0,
    /** Fixed 2-byte encoding. */
    OptionalFormatF2 = 1,
    /** Fixed 4-byte encoding. */
    OptionalFormatF4 = 2,
    /** Fixed 8-byte encoding. */
    OptionalFormatF8 = 3,
    /** "Size encoding" using 1 to 5 bytes, e.g., enum, class identifier. */
    OptionalFormatSize = 4,
    /**
     * "Size encoding" using 1 to 5 bytes followed by data, e.g., string, fixed size
     * struct, or containers whose size can be computed prior to marshaling.
     */
    OptionalFormatVSize = 5,
    /** Fixed size using 4 bytes followed by data, e.g., variable-size struct, container. */
    OptionalFormatFSize = 6,
    /** Class instance. */
    OptionalFormatClass = 7
};
#endif

/**
 * Determines whether the provided type is a container. For now, the implementation only checks
 * if there is a T::iterator typedef using SFINAE.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct IsContainer
{
    template<typename C>
    static char test(typename C::iterator*);

    template<typename C>
    static long test(...);

    static const bool value = sizeof(test<T>(0)) == sizeof(char);
};

/**
 * Determines whether the provided type is a map. For now, the implementation only checks if there
 * is a T::mapped_type typedef using SFINAE.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct IsMap
{
    template<typename C>
    static char test(typename C::mapped_type*);

    template<typename C>
    static long test(...);

    static const bool value = IsContainer<T>::value && sizeof(test<T>(0)) == sizeof(char);
};

#ifdef ICE_CPP11_MAPPING

/**
 * Base traits template. Types with no specialized trait use this trait.
 * \headerfile Ice/Ice.h
 */
template<typename T,  typename Enabler = void>
struct StreamableTraits
{
    static const StreamHelperCategory helper = StreamHelperCategoryUnknown;

    //
    // When extracting a sequence<T> from a stream, we can ensure the
    // stream has at least StreamableTraits<T>::minWireSize * size bytes
    // For containers, the minWireSize is 1 (just 1 byte for an empty container).
    //
    //static const int minWireSize = 1;

    //
    // Is this type encoded on a fixed number of bytes?
    // Used only for marshaling/unmarshaling optional data members and parameters.
    //
    //static const bool fixedLength = false;
};

/**
 * Specialization for sequence and dictionary types.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamableTraits<T, typename ::std::enable_if<IsMap<T>::value || IsContainer<T>::value>::type>
{
    static const StreamHelperCategory helper = IsMap<T>::value ? StreamHelperCategoryDictionary : StreamHelperCategorySequence;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};

/**
 * Specialization for exceptions.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamableTraits<T, typename ::std::enable_if<::std::is_base_of<::Ice::UserException, T>::value>::type>
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
template<typename T>
struct StreamableTraits<std::pair<T*, T*>>
{
    static const StreamHelperCategory helper = StreamHelperCategorySequence;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};

#else

/**
 * Base traits template. Types with no specialized trait use this trait, including sequence and
 * dictionary types.
 * \headerfile Ice/Ice.h
 */
template<typename T,  typename Enabler = void>
struct StreamableTraits
{
    static const StreamHelperCategory helper = IsMap<T>::value ? StreamHelperCategoryDictionary :
        (IsContainer<T>::value ? StreamHelperCategorySequence : StreamHelperCategoryUnknown);

    /**
     * When extracting a sequence<T> from a stream, we can ensure the
     * stream has at least StreamableTraits<T>::minWireSize * size bytes.
     * For containers, the minWireSize is 1 (just 1 byte for an empty container).
     */
    static const int minWireSize = 1;

    /**
     * Is this type encoded on a fixed number of bytes?
     * Used only for marshaling/unmarshaling optional data members and parameters.
     */
    static const bool fixedLength = false;
};

/**
 * Specialization for exceptions.
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits<UserException>
{
    static const StreamHelperCategory helper = StreamHelperCategoryUserException;

    //
    // There is no sequence/dictionary of UserException (so no need for minWireSize)
    // and no optional UserException (so no need for fixedLength)
    //
};

/**
 * Specialization for array / range mapped sequences. The type can be a std::pair<T, T> or a
 * std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> >.
 * \headerfile Ice/Ice.h
 */
template<typename T, typename U>
struct StreamableTraits< ::std::pair<T, U> >
{
    static const StreamHelperCategory helper = StreamHelperCategorySequence;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};
#endif

/**
 * Specialization for built-in type (this is needed for sequence
 * marshaling to figure out the minWireSize of each type).
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits<bool>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 1;
    static const bool fixedLength = true;
};

/**
 * Specialization for built-in type (this is needed for sequence
 * marshaling to figure out the minWireSize of each type).
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits<Byte>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 1;
    static const bool fixedLength = true;
};

/**
 * Specialization for built-in type (this is needed for sequence
 * marshaling to figure out the minWireSize of each type).
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits<Short>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 2;
    static const bool fixedLength = true;
};

/**
 * Specialization for built-in type (this is needed for sequence
 * marshaling to figure out the minWireSize of each type).
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits<Int>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 4;
    static const bool fixedLength = true;
};

/**
 * Specialization for built-in type (this is needed for sequence
 * marshaling to figure out the minWireSize of each type).
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits<Long>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 8;
    static const bool fixedLength = true;
};

/**
 * Specialization for built-in type (this is needed for sequence
 * marshaling to figure out the minWireSize of each type).
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits<Float>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 4;
    static const bool fixedLength = true;
};

/**
 * Specialization for built-in type (this is needed for sequence
 * marshaling to figure out the minWireSize of each type).
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits<Double>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 8;
    static const bool fixedLength = true;
};

/**
 * Specialization for built-in type (this is needed for sequence
 * marshaling to figure out the minWireSize of each type).
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits< ::std::string>
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
template<>
struct StreamableTraits< ::std::wstring>
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};

/**
 * vector<bool> is a special type in C++: the streams handle it like a built-in type.
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamableTraits< ::std::vector<bool> >
{
    static const StreamHelperCategory helper = StreamHelperCategoryBuiltin;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};

/**
 * Specialization for proxy types.
 * \headerfile Ice/Ice.h
 */
#ifdef ICE_CPP11_MAPPING
template<typename T>
struct StreamableTraits<::std::shared_ptr<T>, typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, T>::value>::type>
{
    static const StreamHelperCategory helper = StreamHelperCategoryProxy;
    static const int minWireSize = 2;
    static const bool fixedLength = false;
};
#else
template<typename T>
struct StreamableTraits< ::IceInternal::ProxyHandle<T> >
{
    static const StreamHelperCategory helper = StreamHelperCategoryProxy;
    static const int minWireSize = 2;
    static const bool fixedLength = false;
};
#endif

/**
 * Specialization for class types.
 * \headerfile Ice/Ice.h
 */
#ifdef ICE_CPP11_MAPPING
template<typename T>
struct StreamableTraits<::std::shared_ptr<T>, typename ::std::enable_if<::std::is_base_of<::Ice::Value, T>::value>::type>
{
    static const StreamHelperCategory helper = StreamHelperCategoryClass;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};
#else
template<typename T>
struct StreamableTraits< ::IceInternal::Handle<T> >
{
    static const StreamHelperCategory helper = StreamHelperCategoryClass;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};
#endif

//
// StreamHelper templates used by streams to read and write data.
//

/** Base StreamHelper template; it must be specialized for each type. */
template<typename T, StreamHelperCategory st>
struct StreamHelper;

/**
 * Helper for built-ins, delegates read/write to the stream.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategoryBuiltin>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        stream->write(v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->read(v);
    }
};

//
// "helpers" for the StreamHelper<T, StreamHelperCategoryStruct[Class]> below
// slice2cpp generates specializations as needed
//

/**
 * General writer. slice2cpp generates specializations as needed.
 * \headerfile Ice/Ice.h
 */
template<typename T, typename S>
struct StreamWriter
{
#ifdef ICE_CPP11_MAPPING
    static inline void write(S* stream, const T& v)
    {
        stream->writeAll(v.ice_tuple());
    }
#else
    static inline void write(S*, const T&)
    {
        // Default is to write nothing for C++98
    }
#endif
};

/**
 * General reader. slice2cpp generates specializations as needed.
 * \headerfile Ice/Ice.h
 */
template<typename T, typename S>
struct StreamReader
{
    static inline void read(S*, T&)
    {
        // Default is to read nothing
    }
};

/**
 * Helper for structs.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategoryStruct>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        StreamWriter<T, S>::write(stream, v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        StreamReader<T, S>::read(stream, v);
    }
};

/**
 * Helper for class structs.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategoryStructClass>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        StreamWriter<T, S>::write(stream, v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        v = new typename T::element_type;
        StreamReader<T, S>::read(stream, v);
    }
};

/**
 * Helper for enums.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategoryEnum>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        if(static_cast<Int>(v) < StreamableTraits<T>::minValue || static_cast<Int>(v) > StreamableTraits<T>::maxValue)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        stream->writeEnum(static_cast<Int>(v), StreamableTraits<T>::maxValue);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        Int value = stream->readEnum(StreamableTraits<T>::maxValue);
        if(value < StreamableTraits<T>::minValue || value > StreamableTraits<T>::maxValue)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        v = static_cast<T>(value);
    }
};

/**
 * Helper for sequences.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategorySequence>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        stream->writeSize(static_cast<Int>(v.size()));
        for(typename T::const_iterator p = v.begin(); p != v.end(); ++p)
        {
            stream->write(*p);
        }
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        Int sz = stream->readAndCheckSeqSize(StreamableTraits<typename T::value_type>::minWireSize);
        T(sz).swap(v);
        for(typename T::iterator p = v.begin(); p != v.end(); ++p)
        {
            stream->read(*p);
        }
    }
};

/**
 * Helper for array custom sequence parameters.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<std::pair<const T*, const T*>, StreamHelperCategorySequence>
{
    template<class S> static inline void
    write(S* stream, const std::pair<const T*, const T*>& v)
    {
        stream->write(v.first, v.second);
    }

    template<class S> static inline void
    read(S* stream, std::pair<const T*, const T*>& v)
    {
        stream->read(v);
    }
};

#ifndef ICE_CPP11_MAPPING

/**
 * Helper for range custom sequence parameters.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<std::pair<T, T>, StreamHelperCategorySequence>
{
    template<class S> static inline void
    write(S* stream, const std::pair<T, T>& v)
    {
        stream->writeSize(static_cast<Int>(IceUtilInternal::distance(v.first, v.second)));
        for(T p = v.first; p != v.second; ++p)
        {
            stream->write(*p);
        }
    }

    template<class S> static inline void
    read(S* stream, std::pair<T, T>& v)
    {
        stream->read(v);
    }
};

/**
 * Specialization for sequence<bool>.
 * \headerfile Ice/Ice.h
 */
template<>
struct StreamHelper<std::pair< ::std::vector<bool>::const_iterator,
                               ::std::vector<bool>::const_iterator>, StreamHelperCategorySequence>
{
    template<class S> static inline void
    write(S* stream, const std::pair< ::std::vector<bool>::const_iterator,
                                      ::std::vector<bool>::const_iterator>& v)
    {
        stream->writeSize(static_cast<Int>(IceUtilInternal::distance(v.first, v.second)));
        for(::std::vector<bool>::const_iterator p = v.first; p != v.second; ++p)
        {
            stream->write(static_cast<bool>(*p));
        }
    }
};

/**
 * Helper for zero-copy array sequence parameters.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> >, StreamHelperCategorySequence>
{
    template<class S> static inline void
    read(S* stream, std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> >& v)
    {
        stream->read(v.second, v.first);
    }

    // no write: only used for unmarshaling
};
#endif

/**
 * Helper for dictionaries.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategoryDictionary>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        stream->writeSize(static_cast<Int>(v.size()));
        for(typename T::const_iterator p = v.begin(); p != v.end(); ++p)
        {
            stream->write(p->first);
            stream->write(p->second);
        }
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        Int sz = stream->readSize();
        v.clear();
        while(sz--)
        {
            typename T::value_type p;
            stream->read(const_cast<typename T::key_type&>(p.first));
            typename T::iterator i = v.insert(v.end(), p);
            stream->read(i->second);
        }
    }
};

/**
 * Helper for user exceptions.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategoryUserException>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        stream->writeException(v);
    }

    // no read: only used for marshaling
};

/**
 * Helper for proxies.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategoryProxy>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        stream->write(v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->read(v);
    }
};

/**
 * Helper for classes.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamHelper<T, StreamHelperCategoryClass>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        stream->write(v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->read(v);
    }
};

//
// Helpers to read/write optional attributes or members.
//

//
// Extract / compute the optionalFormat
// This is used _only_ for the base StreamOptionalHelper below
// /!\ Do not use in StreamOptionalHelper specializations, and do
// not provide specialization not handled by the base StreamOptionalHelper
//
/**
 * Extract / compute the optionalFormat.
 * \headerfile Ice/Ice.h
 */
template<StreamHelperCategory st, int minWireSize, bool fixedLength>
struct GetOptionalFormat;

/**
 * Specialization for 1-byte built-in fixed-length types.
 * \headerfile Ice/Ice.h
 */
template<>
struct GetOptionalFormat<StreamHelperCategoryBuiltin, 1, true>
{
    static const OptionalFormat value = ICE_SCOPED_ENUM(OptionalFormat, F1);
};

/**
 * Specialization for 2-byte built-in fixed-length types.
 * \headerfile Ice/Ice.h
 */
template<>
struct GetOptionalFormat<StreamHelperCategoryBuiltin, 2, true>
{
    static const OptionalFormat value = ICE_SCOPED_ENUM(OptionalFormat, F2);
};

/**
 * Specialization for 4-byte built-in fixed-length types.
 * \headerfile Ice/Ice.h
 */
template<>
struct GetOptionalFormat<StreamHelperCategoryBuiltin, 4, true>
{
    static const OptionalFormat value = ICE_SCOPED_ENUM(OptionalFormat, F4);
};

/**
 * Specialization for 8-byte built-in fixed-length types.
 * \headerfile Ice/Ice.h
 */
template<>
struct GetOptionalFormat<StreamHelperCategoryBuiltin, 8, true>
{
    static const OptionalFormat value = ICE_SCOPED_ENUM(OptionalFormat, F8);
};

/**
 * Specialization for built-in variable-length types.
 * \headerfile Ice/Ice.h
 */
template<>
struct GetOptionalFormat<StreamHelperCategoryBuiltin, 1, false>
{
    static const OptionalFormat value = ICE_SCOPED_ENUM(OptionalFormat, VSize);
};

/**
 * Specialization for class types.
 * \headerfile Ice/Ice.h
 */
template<>
struct GetOptionalFormat<StreamHelperCategoryClass, 1, false>
{
    static const OptionalFormat value = ICE_SCOPED_ENUM(OptionalFormat, Class);
};

/**
 * Specialization for enum types.
 * \headerfile Ice/Ice.h
 */
template<int minWireSize>
struct GetOptionalFormat<StreamHelperCategoryEnum, minWireSize, false>
{
    static const OptionalFormat value = ICE_SCOPED_ENUM(OptionalFormat, Size);
};

/**
 * Base helper for optional values: simply read/write the data.
 * \headerfile Ice/Ice.h
 */
template<typename T, StreamHelperCategory st, bool fixedLength>
struct StreamOptionalHelper
{
    typedef StreamableTraits<T> Traits;

    // If this optionalFormat fails to compile, you must either define your specialization
    // for GetOptionalFormat (in which case the optional data will be marshaled/unmarshaled
    // with straight calls to write/read on the stream), or define your own
    // StreamOptionalHelper specialization (which gives you more control over marshaling)
    //
    static const OptionalFormat optionalFormat = GetOptionalFormat<st, Traits::minWireSize, fixedLength>::value;

    template<class S> static inline void
    write(S* stream, const T& v)
    {
        stream->write(v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->read(v);
    }
};

/**
 * Helper to write fixed-size structs.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalHelper<T, StreamHelperCategoryStruct, true>
{
    static const OptionalFormat optionalFormat = ICE_SCOPED_ENUM(OptionalFormat, VSize);

    template<class S> static inline void
    write(S* stream, const T& v)
    {
        stream->writeSize(StreamableTraits<T>::minWireSize);
        stream->write(v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->skipSize();
        stream->read(v);
    }
};

/**
 * Helper to write variable-size structs.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalHelper<T, StreamHelperCategoryStruct, false>
{
    static const OptionalFormat optionalFormat = ICE_SCOPED_ENUM(OptionalFormat, FSize);

    template<class S> static inline void
    write(S* stream, const T& v)
    {
        typename S::size_type pos = stream->startSize();
        stream->write(v);
        stream->endSize(pos);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->skip(4);
        stream->read(v);
    }
};

/**
 * Class structs are encoded like structs.
 * \headerfile Ice/Ice.h
 */
template<typename T, bool fixedLength>
struct StreamOptionalHelper<T, StreamHelperCategoryStructClass, fixedLength> : StreamOptionalHelper<T, StreamHelperCategoryStruct, fixedLength>
{
};

/**
 * Optional proxies are encoded like variable size structs, using the FSize encoding.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalHelper<T, StreamHelperCategoryProxy, false> : StreamOptionalHelper<T, StreamHelperCategoryStruct, false>
{
};

/**
 * Helper to read/write optional sequences or dictionaries.
 * \headerfile Ice/Ice.h
 */
template<typename T, bool fixedLength, int sz>
struct StreamOptionalContainerHelper;

/**
 * Encode containers of variable-size elements with the FSize optional
 * type, since we can't easily figure out the size of the container
 * before encoding. This is the same encoding as variable size structs
 * so we just re-use its implementation.
 * \headerfile Ice/Ice.h
 */
template<typename T, int sz>
struct StreamOptionalContainerHelper<T, false, sz>
{
    static const OptionalFormat optionalFormat = ICE_SCOPED_ENUM(OptionalFormat, FSize);

    template<class S> static inline void
    write(S* stream, const T& v, Int)
    {
        StreamOptionalHelper<T, StreamHelperCategoryStruct, false>::write(stream, v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        StreamOptionalHelper<T, StreamHelperCategoryStruct, false>::read(stream, v);
    }
};

/**
 * Encode containers of fixed-size elements with the VSize optional
 * type since we can figure out the size of the container before
 * encoding.
 * \headerfile Ice/Ice.h
 */
template<typename T, int sz>
struct StreamOptionalContainerHelper<T, true, sz>
{
    static const OptionalFormat optionalFormat = ICE_SCOPED_ENUM(OptionalFormat, VSize);

    template<class S> static inline void
    write(S* stream, const T& v, Int n)
    {
        //
        // The container size is the number of elements * the size of
        // an element and the size-encoded number of elements (1 or
        // 5 depending on the number of elements).
        //
        stream->writeSize(sz * n + (n < 255 ? 1 : 5));
        stream->write(v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->skipSize();
        stream->read(v);
    }
};

/**
 * Optimization: containers of 1 byte elements are encoded with the
 * VSize optional type. There's no need to encode an additional size
 * for those, the number of elements of the container can be used to
 * skip the optional.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalContainerHelper<T, true, 1>
{
    static const OptionalFormat optionalFormat = ICE_SCOPED_ENUM(OptionalFormat, VSize);

    template<class S> static inline void
    write(S* stream, const T& v, Int)
    {
        stream->write(v);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->read(v);
    }
};

/**
 * Helper to write sequences, delegates to the optional container
 * helper template partial specializations.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalHelper<T, StreamHelperCategorySequence, false>
{
    typedef typename T::value_type E;
    static const int size = StreamableTraits<E>::minWireSize;
    static const bool fixedLength = StreamableTraits<E>::fixedLength;

    // The optional type of a sequence depends on whether or not elements are fixed
    // or variable size elements and their size.
    static const OptionalFormat optionalFormat = StreamOptionalContainerHelper<T, fixedLength, size>::optionalFormat;

    template<class S> static inline void
    write(S* stream, const T& v)
    {
        StreamOptionalContainerHelper<T, fixedLength, size>::write(stream, v, static_cast<Int>(v.size()));
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        StreamOptionalContainerHelper<T, fixedLength, size>::read(stream, v);
    }
};

/**
 * Helper to write sequences, delegates to the optional container
 * helper template partial specializations.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalHelper<std::pair<const T*, const T*>, StreamHelperCategorySequence, false>
{
    typedef std::pair<const T*, const T*> P;
    static const int size = StreamableTraits<T>::minWireSize;
    static const bool fixedLength = StreamableTraits<T>::fixedLength;

    // The optional type of a sequence depends on whether or not elements are fixed
    // or variable size elements and their size.
    static const OptionalFormat optionalFormat = StreamOptionalContainerHelper<P, fixedLength, size>::optionalFormat;

    template<class S> static inline void
    write(S* stream, const P& v)
    {
        Int n = static_cast<Int>(v.second - v.first);
        StreamOptionalContainerHelper<P, fixedLength, size>::write(stream, v, n);
    }

    template<class S> static inline void
    read(S* stream, P& v)
    {
        StreamOptionalContainerHelper<P, fixedLength, size>::read(stream, v);
    }
};

#ifndef ICE_CPP11_MAPPING

/**
 * Helper to write sequences, delegates to the optional container
 * helper template partial specializations.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalHelper<std::pair<T, T>, StreamHelperCategorySequence, false>
{
    typedef std::pair<T, T> P;
    static const int size = StreamableTraits<typename T::value_type>::minWireSize;
    static const bool fixedLength = StreamableTraits<typename T::value_type>::fixedLength;

    // The optional type of a sequence depends on whether or not elements are fixed
    // or variable size elements and their size.
    static const OptionalFormat optionalFormat = StreamOptionalContainerHelper<P, fixedLength, size>::optionalFormat;

    template<class S> static inline void
    write(S* stream, const P& v)
    {
        Int n = static_cast<Int>(v.second - v.first);
        StreamOptionalContainerHelper<P, fixedLength, size>::write(stream, v, n);
    }

    template<class S> static inline void
    read(S* stream, P& v)
    {
        StreamOptionalContainerHelper<P, fixedLength, size>::read(stream, v);
    }
};

/**
 * Helper to write sequences, delegates to the optional container
 * helper template partial specializations.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalHelper<std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> >,
                            StreamHelperCategorySequence, false>
{
    typedef std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> > P;
    static const int size = StreamableTraits<T>::minWireSize;
    static const bool fixedLength = StreamableTraits<T>::fixedLength;

    // The optional type of a sequence depends on whether or not elements are fixed
    // or variable size elements and their size.
    static const OptionalFormat optionalFormat = StreamOptionalContainerHelper<P, fixedLength, size>::optionalFormat;

    template<class S> static inline void
    read(S* stream, P& v)
    {
        StreamOptionalContainerHelper<P, fixedLength, size>::read(stream, v);
    }

    // no write: only used for unmarshaling
};
#endif

/**
 * Helper to write dictionaries, delegates to the optional container
 * helper template partial specializations.
 * \headerfile Ice/Ice.h
 */
template<typename T>
struct StreamOptionalHelper<T, StreamHelperCategoryDictionary, false>
{
    typedef typename T::key_type K;
    typedef typename T::mapped_type V;

    static const int size = StreamableTraits<K>::minWireSize + StreamableTraits<V>::minWireSize;
    static const bool fixedLength = StreamableTraits<K>::fixedLength && StreamableTraits<V>::fixedLength;

    // The optional type of a dictionary depends on whether or not elements are fixed
    // or variable size elements.
    static const OptionalFormat optionalFormat = StreamOptionalContainerHelper<T, fixedLength, size>::optionalFormat;

    template<class S> static inline void
    write(S* stream, const T& v)
    {
        StreamOptionalContainerHelper<T, fixedLength, size>::write(stream, v, static_cast<Int>(v.size()));
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        StreamOptionalContainerHelper<T, fixedLength, size>::read(stream, v);
    }
};

/// \endcond

}

#endif
