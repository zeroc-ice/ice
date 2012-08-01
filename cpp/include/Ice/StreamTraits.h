// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_TRAITS_H
#define ICE_STREAM_TRAITS_H

#include <IceUtil/ScopedArray.h>
#include <IceUtil/Iterator.h>

namespace Ice
{

//
// The different types of Slice types supported by the stream
// marshalling/un-marshalling methods.
//
enum StreamTraitType
{
    StreamTraitTypeBuiltin,
    StreamTraitTypeStruct,
    StreamTraitTypeStructClass, // struct with cpp:class metadata
    StreamTraitTypeEnum,
    StreamTraitTypeSequence,
    StreamTraitTypeDictionary,
    StreamTraitTypeProxy,
    StreamTraitTypeClass,
    StreamTraitTypeUserException,
    StreamTraitTypeUnknown
};

//
// The optional type.
//
// Optional data members or attribute is encoded with a specific
// optional type. This optional type describes how the data is encoded
// and how it can be skiped by the un-marhsalling code if the optional
// isn't known to the receiver.
//
enum OptionalType
{
    OptionalTypeF1 = 0,
    OptionalTypeF2 = 1,
    OptionalTypeF4 = 2,
    OptionalTypeF8 = 3,
    OptionalTypeSize = 4,
    OptionalTypeVSize = 5,
    OptionalTypeFSize = 6,
    OptionalTypeEndMarker = 7
};

//
// Base trait template. This doesn't actually do anything -- we just
// use it as a template that we can specialize. 
//
// Note that types which don't define a specialized trait will end up
// using this trait. The marshalling of unknown types assume that 
// such traits are custom sequences. If not appropriate, a trait has
// to defined with the appropriate type.
//
template<typename T>
struct StreamTrait
{
    static const StreamTraitType type = StreamTraitTypeUnknown;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeVSize; // Not used
    static const bool isVariableLength = true;
};

//
// StreamTrait specialization for std::vector
//
template<typename T>
struct StreamTrait< ::std::vector<T> >
{
    static const StreamTraitType type = StreamTraitTypeSequence;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeVSize; // Not used but still need to be defined.
    static const bool isVariableLength = true;
};

template<typename T, typename U>
struct StreamTrait< ::std::pair<T, U> >
{
    static const StreamTraitType type = StreamTraitTypeSequence;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeVSize; // Not used but still need to be defined.
    static const bool isVariableLength = true;
};

//
// StreamTrait specialization for user exceptions.
//
template<>
struct StreamTrait<UserException>
{
    static const StreamTraitType type = StreamTraitTypeUserException;
};

//
// StreamTrait specialization for std::map.
//
template<typename K, typename V>
struct StreamTrait< ::std::map<K, V> >
{
    static const StreamTraitType type = StreamTraitTypeDictionary;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeVSize; // Not used
    static const bool isVariableLength = true;
};

//
// StreamTrait specialization for builtins (these are needed for sequence
// marshalling to figure out the minWireSize of each built-in).
//
template<>
struct StreamTrait< bool>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeF1;
    static const bool isVariableLength = false;
};

template<>
struct StreamTrait< Byte>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeF1;
    static const bool isVariableLength = false;
};

template<>
struct StreamTrait< Short>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 2;
    static const OptionalType optionalType = OptionalTypeF2;
    static const bool isVariableLength = false;
};

template<>
struct StreamTrait< Int>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 4;
    static const OptionalType optionalType = OptionalTypeF4;
    static const bool isVariableLength = false;
};

template<>
struct StreamTrait< Long>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 8;
    static const OptionalType optionalType = OptionalTypeF8;
    static const bool isVariableLength = false;
};

template<>
struct StreamTrait< Float>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 4;
    static const OptionalType optionalType = OptionalTypeF4;
    static const bool isVariableLength = false;
};

template<>
struct StreamTrait< Double>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 8;
    static const OptionalType optionalType = OptionalTypeF8;
    static const bool isVariableLength = false;
};

template<>
struct StreamTrait< ::std::string>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeVSize;
    static const bool isVariableLength = true;
};

template<>
struct StreamTrait< ::std::wstring>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeVSize;
    static const bool isVariableLength = true;
};

template<typename T>
struct StreamTrait< ::IceInternal::ProxyHandle<T> >
{
    static const StreamTraitType type = StreamTraitTypeProxy;
    static const int minWireSize = 2;
    static const OptionalType optionalType = OptionalTypeFSize;
    static const bool isVariableLength = true;
};

template<typename T>
struct StreamTrait< ::IceInternal::Handle<T> >
{
    static const StreamTraitType type = StreamTraitTypeClass;
    static const int minWireSize = 1;
    static const OptionalType optionalType = OptionalTypeSize;
    static const bool isVariableLength = true;
};

//
// StreamHelper templates used by streams to read and write data.
//

template<typename T, StreamTraitType st> 
struct StreamHelper 
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        assert(false);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        assert(false);
    }
};

template<typename T, StreamTraitType st, OptionalType ot>
struct StreamOptionalHelper 
{
    static const OptionalType optionalType = ot;

    template<class S> static inline void 
    write(S* stream, const T& v)
    {
        assert((ot != OptionalTypeVSize && ot != OptionalTypeFSize) || st == StreamTraitTypeBuiltin);
        StreamHelper<T, st>::write(stream, v);
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        assert((ot != OptionalTypeVSize && ot != OptionalTypeFSize) || st == StreamTraitTypeBuiltin);
        StreamHelper<T, st>::read(stream, v);
    }
};

// Helper for builtins, delegates read/write to the stream.
template<typename T>
struct StreamHelper<T, StreamTraitTypeBuiltin>
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

// Helper for structs, uses generated __read/__write methods
template<typename T>
struct StreamHelper<T, StreamTraitTypeStruct>
{
    template<class S> static inline void 
    write(S* stream, const T& v)
    {
        v.__write(stream);
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        v.__read(stream);
    }
};

// Helper for class structs, uses generated __read/__write methods
template<typename T>
struct StreamHelper<T, StreamTraitTypeStructClass>
{
    template<class S> static inline void 
    write(S* stream, const T& v)
    {
        v->__write(stream);
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        v = new typename T::element_type();
        v->__read(stream);
    }
};

// Helper for enums
template<typename T>
struct StreamHelper<T, StreamTraitTypeEnum>
{
    template<class S> static inline void
    write(S* stream, const T& v)
    {
        if(static_cast<Int>(v) < 0 || static_cast<Int>(v) >= StreamTrait<T>::enumLimit)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        stream->writeEnum(static_cast<Int>(v), StreamTrait<T>::enumLimit);
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        Int value = stream->readEnum(StreamTrait<T>::enumLimit);
        if(value < 0 || value >= StreamTrait<T>::enumLimit)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        v = static_cast<T>(value);
    }
};

// Helper for sequences
template<typename T>
struct StreamHelper<T, StreamTraitTypeSequence>
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
        Int sz = stream->readAndCheckSeqSize(StreamTrait<T>::minWireSize);
        T(sz).swap(v);
        for(typename T::iterator p = v.begin(); p != v.end(); ++p)
        {
            stream->read(*p);
        }
    }
};

// Helper for array and range:array custom sequence parameters
template<typename T>
struct StreamHelper<std::pair<const T*, const T*>, StreamTraitTypeSequence>
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

// Helper for range custom sequence parameters
template<typename T>
struct StreamHelper<std::pair<T, T>, StreamTraitTypeSequence>
{
    template<class S> static inline void 
    write(S* stream, const std::pair<T, T>& v)
    {
        stream->writeSize(static_cast< ::Ice::Int>(IceUtilInternal::distance(v.first, v.second)));
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

// Helper for zero-copy array sequence parameters
template<typename T>
struct StreamHelper<std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> >, StreamTraitTypeSequence>
{
    template<class S> static inline void 
    write(S* stream, const std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> > & v)
    {
        assert(false); // Only used to un-marshal
    }

    template<class S> static inline void 
    read(S* stream, std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> >& v)
    {
        v.first.reset(stream->read(v.second));
    }
};

// Helper for dictionaries
template<typename T>
struct StreamHelper<T, StreamTraitTypeDictionary>
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
        while(sz--)
        {
            typename T::value_type p;
            stream->read(const_cast<typename T::key_type&>(p.first));
            typename T::iterator i = v.insert(v.end(), p);
            stream->read(i->second);
        }
    }
};

// Helper for user exceptions
template<typename T>
struct StreamHelper<T, StreamTraitTypeUserException>
{
    template<class S> static inline void 
    write(S* stream, const T& v)
    {
        stream->writeException(v);
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        assert(false);
    }
};

// Helper for proxies
template<typename T>
struct StreamHelper<T, StreamTraitTypeProxy>
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

// Helper for classes
template<typename T>
struct StreamHelper<T, StreamTraitTypeClass>
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

// Helper for unknown types, assume custom sequences.
template<typename T>
struct StreamHelper<T, StreamTraitTypeUnknown> : StreamHelper<T, StreamTraitTypeSequence>
{
};

//
// Helpers to read/write optional attributes or members.
//

// Helper to write fixed size structs
template<typename T>
struct StreamOptionalHelper<T, StreamTraitTypeStruct, OptionalTypeVSize>
{
    static const OptionalType optionalType = OptionalTypeVSize;

    template<class S> static inline void 
    write(S* stream, const T& v)
    {
        stream->writeSize(StreamTrait<T>::minWireSize);
        stream->write(v);
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        stream->skipSize();
        stream->read(v);
    }
};

// Helper to write variable size structs
template<typename T>
struct StreamOptionalHelper<T, StreamTraitTypeStruct, OptionalTypeFSize>
{
    static const OptionalType optionalType = OptionalTypeFSize;

    template<class S> static inline void 
    write(S* stream, const T& v)
    {
        stream->write((Int)0);
        typename S::size_type p = stream->pos();
        stream->write(v);
        stream->rewrite(static_cast<Int>(stream->pos() - p), p - 4);
    }

    template<class S> static inline void
    read(S* stream, T& v)
    {
        stream->skip(4);
        stream->read(v);
    }
};

// Class structs are encoded like structs 
template<typename T, OptionalType ot>
struct StreamOptionalHelper<T, StreamTraitTypeStructClass, ot> : StreamOptionalHelper<T, StreamTraitTypeStruct, ot>
{
};

// Optional proxies are encoded like variable size structs, using the FSize encoding
template<typename T>
struct StreamOptionalHelper<T, StreamTraitTypeProxy, OptionalTypeFSize> : 
    StreamOptionalHelper<T, StreamTraitTypeStruct, OptionalTypeFSize>
{
};

//
// Helpers to read/write optional sequences or dictionaries
//

template<typename T, bool isVariableLength, int sz>
struct StreamOptionalContainerHelper
{
    static const OptionalType optionalType = OptionalTypeFSize;

    template<class S> static inline void 
    write(S* stream, const T& v, Ice::Int n) 
    {
        assert(false); 
    }

    template<class S> static inline void 
    read(S* stream, T& v) 
    { 
        assert(false); 
    }
};

//
// Encode containers of variable size elements with the FSize optional
// type, since we can't easily figure out the size of the container
// before encoding. This is the same encoding as variable size structs
// so we just re-use its implementation.
//
template<typename T, int sz>
struct StreamOptionalContainerHelper<T, true, sz>
{
    static const OptionalType optionalType = OptionalTypeFSize;

    template<class S> static inline void 
    write(S* stream, const T& v, Ice::Int n) 
    {
        StreamOptionalHelper<T, StreamTraitTypeStruct, OptionalTypeFSize>::write(stream, v);
    }

    template<class S> static inline void 
    read(S* stream, T& v) 
    {
        StreamOptionalHelper<T, StreamTraitTypeStruct, OptionalTypeFSize>::read(stream, v);
    }
};

//
// Encode containers of fixed size elements with the VSize optional
// type since we can figure out the size of the container before
// encoding. 
//
template<typename T, int sz>
struct StreamOptionalContainerHelper<T, false, sz>
{ 
    static const OptionalType optionalType = OptionalTypeVSize;

    template<class S> static inline void 
    write(S* stream, const T& v, Ice::Int n)
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

//
// Optimization: containers of 1 byte elements are encoded with the
// VSize optional type. There's no need to encode an additional size
// for those, the number of elements of the container can be used to
// skip the optional.
//
template<typename T>
struct StreamOptionalContainerHelper<T, false, 1>
{ 
    static const OptionalType optionalType = OptionalTypeVSize;

    template<class S> static inline void 
    write(S* stream, const T& v, Ice::Int n) 
    {
        StreamOptionalHelper<T, StreamTraitTypeBuiltin, OptionalTypeVSize>::write(stream, v);
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        StreamOptionalHelper<T, StreamTraitTypeBuiltin, OptionalTypeVSize>::read(stream, v);
    }
};

//
// Helper to write sequences, delegates to the optional container
// helper template partial specializations.
//
template<typename T, OptionalType ot>
struct StreamOptionalHelper<T, StreamTraitTypeSequence, ot>
{
    typedef typename T::value_type E;
    static const int size = StreamTrait<E>::minWireSize;
    static const bool isVariableLength = StreamTrait<E>::isVariableLength;

    // The optional type of a sequence depends on wether or not elements are fixed
    // or variable size elements and their size.
    static const OptionalType optionalType = StreamOptionalContainerHelper<T, isVariableLength, size>::optionalType;

    template<class S> static inline void 
    write(S* stream, const T& v)
    {
        StreamOptionalContainerHelper<T, isVariableLength, size>::write(stream, v, v.size());
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        StreamOptionalContainerHelper<T, isVariableLength, size>::read(stream, v);
    }
};

template<typename T, OptionalType ot>
struct StreamOptionalHelper<std::pair<const T*, const T*>, StreamTraitTypeSequence, ot>
{
    typedef std::pair<const T*, const T*> P;
    static const int size = StreamTrait<T>::minWireSize;
    static const bool isVariableLength = StreamTrait<T>::isVariableLength;

    // The optional type of a sequence depends on wether or not elements are fixed
    // or variable size elements and their size.
    static const OptionalType optionalType = StreamOptionalContainerHelper<P, isVariableLength, size>::optionalType;

    template<class S> static inline void 
    write(S* stream, const P& v)
    {
        Ice::Int n = static_cast<Ice::Int>(v.second - v.first);
        StreamOptionalContainerHelper<P, isVariableLength, size>::write(stream, v, n);
    }

    template<class S> static inline void 
    read(S* stream, P& v)
    {
        StreamOptionalContainerHelper<P, isVariableLength, size>::read(stream, v);
    }
};

template<typename T, OptionalType ot>
struct StreamOptionalHelper<std::pair<T, T>, StreamTraitTypeSequence, ot>
{
    typedef std::pair<T, T> P;
    static const int size = StreamTrait<typename T::value_type>::minWireSize;
    static const bool isVariableLength = StreamTrait<typename T::value_type>::isVariableLength;

    // The optional type of a sequence depends on wether or not elements are fixed
    // or variable size elements and their size.
    static const OptionalType optionalType = StreamOptionalContainerHelper<P, isVariableLength, size>::optionalType;

    template<class S> static inline void 
    write(S* stream, const P& v)
    {
        Ice::Int n = static_cast<Ice::Int>(v.second - v.first);
        StreamOptionalContainerHelper<P, isVariableLength, size>::write(stream, v, n);
    }

    template<class S> static inline void 
    read(S* stream, P& v)
    {
        StreamOptionalContainerHelper<P, isVariableLength, size>::read(stream, v);
    }
};

template<typename T, OptionalType ot>
struct StreamOptionalHelper<std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> >,
                            StreamTraitTypeSequence, ot>
{
    typedef std::pair<IceUtil::ScopedArray<T>, std::pair<const T*, const T*> > P;
    static const int size = StreamTrait<T>::minWireSize;
    static const bool isVariableLength = StreamTrait<T>::isVariableLength;

    // The optional type of a sequence depends on wether or not elements are fixed
    // or variable size elements and their size.
    static const OptionalType optionalType = StreamOptionalContainerHelper<P, isVariableLength, size>::optionalType;

    template<class S> static inline void 
    write(S* stream, const P& v)
    {
        assert(false);
    }

    template<class S> static inline void 
    read(S* stream, P& v)
    {
        StreamOptionalContainerHelper<P, isVariableLength, size>::read(stream, v);
    }
};


//
// Helper to write dictionaries, delegates to the optional container
// helper template partial specializations.
//
template<typename T, OptionalType ot>
struct StreamOptionalHelper<T, StreamTraitTypeDictionary, ot>
{
    typedef typename T::key_type K;
    typedef typename T::mapped_type V;

    static const int size = StreamTrait<K>::minWireSize + StreamTrait<V>::minWireSize;
    static const bool isVariableLength = StreamTrait<K>::isVariableLength || StreamTrait<V>::isVariableLength;

    // The optional type of a dictionary depends on wether or not elements are fixed
    // or variable size elements.
    static const OptionalType optionalType = StreamOptionalContainerHelper<T, isVariableLength, size>::optionalType;

    template<class S> static inline void 
    write(S* stream, const T& v)
    {
        StreamOptionalContainerHelper<T, isVariableLength, size>::write(stream, v, v.size());
    }

    template<class S> static inline void 
    read(S* stream, T& v)
    {
        StreamOptionalContainerHelper<T, isVariableLength, size>::read(stream, v);
    }
};

}

#endif
