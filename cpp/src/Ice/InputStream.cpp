// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/InputStream.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ValueFactory.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/FactoryTableInit.h>
#include <Ice/TraceUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/SlicedData.h>
#include <Ice/StringConverter.h>
#include <iterator>

#ifndef ICE_UNALIGNED
#   if defined(__i386) || defined(_M_IX86) || defined(__x86_64) || defined(_M_X64)
#       define ICE_UNALIGNED
#   endif
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;


Ice::InputStream::InputStream()
{
    initialize(currentEncoding);
}

Ice::InputStream::InputStream(const vector<Byte>& v) :
    Buffer(v)
{
    initialize(currentEncoding);
}

Ice::InputStream::InputStream(const pair<const Byte*, const Byte*>& p) :
    Buffer(p.first, p.second)
{
    initialize(currentEncoding);
}

Ice::InputStream::InputStream(Buffer& buf, bool adopt) :
    Buffer(buf, adopt)
{
    initialize(currentEncoding);
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator)
{
    initialize(communicator);
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator, const vector<Byte>& v) :
    Buffer(v)
{
    initialize(communicator);
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator, const pair<const Byte*, const Byte*>& p) :
    Buffer(p.first, p.second)
{
    initialize(communicator);
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator, Buffer& buf, bool adopt) :
    Buffer(buf, adopt)
{
    initialize(communicator);
}

Ice::InputStream::InputStream(const EncodingVersion& encoding)
{
    initialize(encoding);
}

Ice::InputStream::InputStream(const EncodingVersion& encoding, const vector<Byte>& v) :
    Buffer(v)
{
    initialize(encoding);
}

Ice::InputStream::InputStream(const EncodingVersion& encoding, const pair<const Byte*, const Byte*>& p) :
    Buffer(p.first, p.second)
{
    initialize(encoding);
}

Ice::InputStream::InputStream(const EncodingVersion& encoding, Buffer& buf, bool adopt) :
    Buffer(buf, adopt)
{
    initialize(encoding);
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator, const EncodingVersion& encoding)
{
    initialize(communicator, encoding);
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator, const EncodingVersion& encoding,
                              const vector<Byte>& v) :
    Buffer(v)
{
    initialize(communicator, encoding);
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator, const EncodingVersion& encoding,
                              const pair<const Byte*, const Byte*>& p) :
    Buffer(p.first, p.second)
{
    initialize(communicator, encoding);
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator, const EncodingVersion& encoding,
                              Buffer& buf, bool adopt) :
    Buffer(buf, adopt)
{
    initialize(communicator, encoding);
}

Ice::InputStream::InputStream(Instance* instance, const EncodingVersion& encoding)
{
    initialize(instance, encoding);
}

Ice::InputStream::InputStream(Instance* instance, const EncodingVersion& encoding, Buffer& buf, bool adopt) :
    Buffer(buf, adopt)
{
    initialize(instance, encoding);
}

void
Ice::InputStream::initialize(const CommunicatorPtr& communicator)
{
    Instance* instance = getInstance(communicator).get();
    initialize(instance, instance->defaultsAndOverrides()->defaultEncoding);
}

void
Ice::InputStream::initialize(const CommunicatorPtr& communicator, const EncodingVersion& encoding)
{
    initialize(getInstance(communicator).get(), encoding);
}

void
Ice::InputStream::initialize(Instance* instance, const EncodingVersion& encoding)
{
    initialize(encoding);

    _instance = instance;

#ifndef ICE_CPP11_MAPPING
    _collectObjects = _instance->collectObjects();
#endif
    _traceSlicing = _instance->traceLevels()->slicing > 0;
}

void
Ice::InputStream::initialize(const EncodingVersion& encoding)
{
    _instance = 0;
    _encoding = encoding;
    _currentEncaps = 0;
#ifndef ICE_CPP11_MAPPING
    _collectObjects = false;
#endif
    _traceSlicing = false;
    _closure = 0;
    _sliceValues = true;
    _startSeq = -1;
    _minSeqSize = 0;
}

void
Ice::InputStream::clear()
{
    while(_currentEncaps && _currentEncaps != &_preAllocatedEncaps)
    {
        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        delete oldEncaps;
    }

    _startSeq = -1;
    _sliceValues = true;
}

void
Ice::InputStream::setValueFactoryManager(const ValueFactoryManagerPtr& vfm)
{
    _valueFactoryManager = vfm;
}

void
Ice::InputStream::setLogger(const LoggerPtr& logger)
{
    _logger = logger;
}

void
#ifdef ICE_CPP11_MAPPING
Ice::InputStream::setCompactIdResolver(std::function<std::string(int)> r)
#else
Ice::InputStream::setCompactIdResolver(const CompactIdResolverPtr& r)
#endif
{
    _compactIdResolver = r;
}

#ifndef ICE_CPP11_MAPPING
void
Ice::InputStream::setCollectObjects(bool b)
{
    _collectObjects = b;
}
#endif

void
Ice::InputStream::setSliceValues(bool b)
{
    _sliceValues = b;
}

void
Ice::InputStream::setTraceSlicing(bool b)
{
    _traceSlicing = b;
}

void*
Ice::InputStream::getClosure() const
{
    return _closure;
}

void*
Ice::InputStream::setClosure(void* p)
{
    void* prev = _closure;
    _closure = p;
    return prev;
}

void
Ice::InputStream::swap(InputStream& other)
{
    swapBuffer(other);

    std::swap(_instance, other._instance);
    std::swap(_encoding, other._encoding);
#ifndef ICE_CPP11_MAPPING
    std::swap(_collectObjects, other._collectObjects);
#endif
    std::swap(_traceSlicing, other._traceSlicing);
    std::swap(_closure, other._closure);
    std::swap(_sliceValues, other._sliceValues);

    //
    // Swap is never called for streams that have encapsulations being read. However,
    // encapsulations might still be set in case unmarshaling failed. We just
    // reset the encapsulations if there are still some set.
    //
    resetEncapsulation();
    other.resetEncapsulation();

    std::swap(_startSeq, other._startSeq);
    std::swap(_minSeqSize, other._minSeqSize);

    std::swap(_valueFactoryManager, other._valueFactoryManager);
    std::swap(_logger, other._logger);
    std::swap(_compactIdResolver, other._compactIdResolver);
}

void
Ice::InputStream::resetEncapsulation()
{
    while(_currentEncaps && _currentEncaps != &_preAllocatedEncaps)
    {
        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        delete oldEncaps;
    }

    _preAllocatedEncaps.reset();
}

Int
Ice::InputStream::getEncapsulationSize()
{
    assert(_currentEncaps);
    return _currentEncaps->sz - static_cast<Int>(sizeof(Int)) - 2;
}

EncodingVersion
Ice::InputStream::skipEncapsulation()
{
    Int sz;
    read(sz);
    if(sz < 6)
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    if(i - sizeof(Int) + sz > b.end())
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    EncodingVersion encoding;
    read(encoding.major);
    read(encoding.minor);
    i += sz - sizeof(Int) - 2;
    return encoding;
}

void
Ice::InputStream::readPendingValues()
{
    if(_currentEncaps && _currentEncaps->decoder)
    {
        _currentEncaps->decoder->readPendingValues();
    }
    else if(getEncoding() == Ice::Encoding_1_0)
    {
        //
        // If using the 1.0 encoding and no instances were read, we
        // still read an empty sequence of pending instances if
        // requested (i.e.: if this is called).
        //
        // This is required by the 1.0 encoding, even if no instances
        // are written we do marshal an empty sequence if marshaled
        // data types use classes.
        //
        skipSize();
    }
}

Int
Ice::InputStream::readAndCheckSeqSize(int minSize)
{
    Int sz = readSize();

    if(sz == 0)
    {
        return sz;
    }

    //
    // The _startSeq variable points to the start of the sequence for which
    // we expect to read at least _minSeqSize bytes from the stream.
    //
    // If not initialized or if we already read more data than _minSeqSize,
    // we reset _startSeq and _minSeqSize for this sequence (possibly a
    // top-level sequence or enclosed sequence it doesn't really matter).
    //
    // Otherwise, we are reading an enclosed sequence and we have to bump
    // _minSeqSize by the minimum size that this sequence will  require on
    // the stream.
    //
    // The goal of this check is to ensure that when we start un-marshalling
    // a new sequence, we check the minimal size of this new sequence against
    // the estimated remaining buffer size. This estimatation is based on
    // the minimum size of the enclosing sequences, it's _minSeqSize.
    //
    if(_startSeq == -1 || i > (b.begin() + _startSeq + _minSeqSize))
    {
        _startSeq = static_cast<int>(i - b.begin());
        _minSeqSize = sz * minSize;
    }
    else
    {
        _minSeqSize += sz * minSize;
    }

    //
    // If there isn't enough data to read on the stream for the sequence (and
    // possibly enclosed sequences), something is wrong with the marshalled
    // data: it's claiming having more data that what is possible to read.
    //
    if(_startSeq + _minSeqSize > static_cast<int>(b.size()))
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }

    return sz;
}

void
Ice::InputStream::readBlob(vector<Byte>& v, Int sz)
{
    if(sz > 0)
    {
        if(b.end() - i < sz)
        {
            throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        vector<Byte>(i, i + sz).swap(v);
        i += sz;
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(std::vector<Ice::Byte>& v)
{
    std::pair<const Ice::Byte*, const Ice::Byte*> p;
    read(p);
    if(p.first != p.second)
    {
        v.resize(static_cast<Ice::Int>(p.second - p.first));
        copy(p.first, p.second, v.begin());
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(pair<const Byte*, const Byte*>& v)
{
    Int sz = readAndCheckSeqSize(1);
    if(sz > 0)
    {
        v.first = i;
        v.second = i + sz;
        i += sz;
    }
    else
    {
        v.first = v.second = i;
    }
}

void
Ice::InputStream::read(vector<bool>& v)
{
    Int sz = readAndCheckSeqSize(1);
    if(sz > 0)
    {
        v.resize(sz);
        copy(i, i + sz, v.begin());
        i += sz;
    }
    else
    {
        v.clear();
    }
}

namespace
{

template<size_t boolSize>
struct ReadBoolHelper
{
    static bool* read(pair<const bool*, const bool*>& v, Int sz, InputStream::Container::iterator& i)
    {
        bool* array = new bool[sz];
        for(int idx = 0; idx < sz; ++idx)
        {
            array[idx] = static_cast<bool>(*(i + idx));
        }
        v.first = array;
        v.second = array + sz;
        return array;
    }
};

template<>
struct ReadBoolHelper<1>
{
    static bool* read(pair<const bool*, const bool*>& v, Int sz, InputStream::Container::iterator& i)
    {
        v.first = reinterpret_cast<bool*>(i);
        v.second = reinterpret_cast<bool*>(i) + sz;
        return 0;
    }
};

}

#ifdef ICE_CPP11_MAPPING
void
Ice::InputStream::read(pair<const bool*, const bool*>& v)
{
    Int sz = readAndCheckSeqSize(1);
    if(sz > 0)
    {
        auto boolArray = ReadBoolHelper<sizeof(bool)>::read(v, sz, i);
        if(boolArray)
        {
            _deleters.push_back([boolArray] { delete[] boolArray; });
        }
        i += sz;
    }
    else
    {
        v.first = v.second = reinterpret_cast<bool*>(i);
    }
}

#else
void
Ice::InputStream::read(pair<const bool*, const bool*>& v, IceUtil::ScopedArray<bool>& result)
{
    Int sz = readAndCheckSeqSize(1);
    if(sz > 0)
    {
        result.reset(ReadBoolHelper<sizeof(bool)>::read(v, sz, i));
        i += sz;
    }
    else
    {
        result.reset();
        v.first = v.second = reinterpret_cast<bool*>(i);
    }
}
#endif

void
Ice::InputStream::read(Short& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Short)))
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Short);
#ifdef ICE_BIG_ENDIAN
    Byte* dest = reinterpret_cast<Byte*>(&v) + sizeof(Short) - 1;
    *dest-- = *src++;
    *dest = *src;
#else
    Byte* dest = reinterpret_cast<Byte*>(&v);
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::InputStream::read(vector<Short>& v)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Short)));
    if(sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Short));
        v.resize(sz);
#ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&v[0]) + sizeof(Short) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Short);
        }
#else
        copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
    else
    {
        v.clear();
    }
}

#ifdef ICE_CPP11_MAPPING
void
Ice::InputStream::read(pair<const short*, const short*>& v)
#else
void
Ice::InputStream::read(pair<const Short*, const Short*>& v, IceUtil::ScopedArray<Short>& result)
#endif
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Short)));
    if(sz > 0)
    {
#ifdef ICE_UNALIGNED
        v.first = reinterpret_cast<Short*>(i);
        i += sz * static_cast<int>(sizeof(Short));
        v.second = reinterpret_cast<Short*>(i);
#else
#  ifdef ICE_CPP11_MAPPING
        auto result = new short[sz];
        _deleters.push_back([result] { delete[] result; });
        v.first = result;
        v.second = result + sz;
#  else
        result.reset(new Short[sz]);
        v.first = result.get();
        v.second = result.get() + sz;
#   endif

        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Short));
#  ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&result[0]) + sizeof(Short) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Short);
        }
#  else
        copy(begin, i, reinterpret_cast<Byte*>(&result[0]));
#  endif
#endif
    }
    else
    {
#ifndef ICE_CPP11_MAPPING
        result.reset();
#endif
        v.first = v.second = 0;
    }
}

void
Ice::InputStream::read(vector<Int>& v)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Int)));
    if(sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Int));
        v.resize(sz);
#ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&v[0]) + sizeof(Int) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Int);
        }
#else
        copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
    else
    {
        v.clear();
    }
}

#ifdef ICE_CPP11_MAPPING
void
Ice::InputStream::read(pair<const Int*, const Int*>& v)
#else
void
Ice::InputStream::read(pair<const Int*, const Int*>& v, ::IceUtil::ScopedArray<Int>& result)
#endif
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Int)));
    if(sz > 0)
    {
#ifdef ICE_UNALIGNED
        v.first = reinterpret_cast<Int*>(i);
        i += sz * static_cast<int>(sizeof(Int));
        v.second = reinterpret_cast<Int*>(i);
#else

#  ifdef ICE_CPP11_MAPPING
        auto result = new int[sz];
        _deleters.push_back([result] { delete[] result; });
        v.first = result;
        v.second = result + sz;
#  else
        result.reset(new Int[sz]);
        v.first = result.get();
        v.second = result.get() + sz;
#  endif

        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Int));
#  ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&result[0]) + sizeof(Int) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Int);
        }
#  else
        copy(begin, i, reinterpret_cast<Byte*>(&result[0]));
#  endif
#endif
    }
    else
    {
#ifndef ICE_CPP11_MAPPING
        result.reset();
#endif
        v.first = v.second = 0;
    }
}

void
Ice::InputStream::read(Long& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Long)))
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Long);
#ifdef ICE_BIG_ENDIAN
    Byte* dest = reinterpret_cast<Byte*>(&v) + sizeof(Long) - 1;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest = *src;
#else
    Byte* dest = reinterpret_cast<Byte*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::InputStream::read(vector<Long>& v)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Long)));
    if(sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Long));
        v.resize(sz);
#ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&v[0]) + sizeof(Long) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Long);
        }
#else
        copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
    else
    {
        v.clear();
    }
}

#ifdef ICE_CPP11_MAPPING
void
Ice::InputStream::read(pair<const Long*, const Long*>& v)
#else
void
Ice::InputStream::read(pair<const Long*, const Long*>& v, IceUtil::ScopedArray<Long>& result)
#endif
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Long)));
    if(sz > 0)
    {
#ifdef ICE_UNALIGNED
        v.first = reinterpret_cast<Long*>(i);
        i += sz * static_cast<int>(sizeof(Long));
        v.second = reinterpret_cast<Long*>(i);
#else

#  ifdef ICE_CPP11_MAPPING
        auto result = new long long[sz];
        _deleters.push_back([result] { delete[] result; });
        v.first = result;
        v.second = result + sz;
#  else
        result.reset(new Long[sz]);
        v.first = result.get();
        v.second = result.get() + sz;
#  endif

        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Long));
#  ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&result[0]) + sizeof(Long) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Long);
        }
#  else
        copy(begin, i, reinterpret_cast<Byte*>(&result[0]));
#  endif
#endif
    }
    else
    {
#ifndef ICE_CPP11_MAPPING
        result.reset();
#endif
        v.first = v.second = 0;
    }
}

void
Ice::InputStream::read(Float& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Float)))
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Float);
#ifdef ICE_BIG_ENDIAN
    Byte* dest = reinterpret_cast<Byte*>(&v) + sizeof(Float) - 1;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest = *src;
#else
    Byte* dest = reinterpret_cast<Byte*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::InputStream::read(vector<Float>& v)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Float)));
    if(sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Float));
        v.resize(sz);
#ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&v[0]) + sizeof(Float) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Float);
        }
#else
        copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
    else
    {
        v.clear();
    }
}

#ifdef ICE_CPP11_MAPPING
void
Ice::InputStream::read(pair<const Float*, const Float*>& v)
#else
void
Ice::InputStream::read(pair<const Float*, const Float*>& v, IceUtil::ScopedArray<Float>& result)
#endif
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Float)));
    if(sz > 0)
    {
#ifdef ICE_UNALIGNED
        v.first = reinterpret_cast<Float*>(i);
        i += sz * static_cast<int>(sizeof(Float));
        v.second = reinterpret_cast<Float*>(i);
#else

#  ifdef ICE_CPP11_MAPPING
        auto result = new float[sz];
        _deleters.push_back([result] { delete[] result; });
        v.first = result;
        v.second = result + sz;
#  else
        result.reset(new Float[sz]);
        v.first = result.get();
        v.second = result.get() + sz;
#  endif

        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Float));
#  ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&result[0]) + sizeof(Float) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Float);
        }
#  else
        copy(begin, i, reinterpret_cast<Byte*>(&result[0]));
#  endif
#endif
    }
    else
    {
#ifndef ICE_CPP11_MAPPING
        result.reset();
#endif
        v.first = v.second = 0;
    }
}

void
Ice::InputStream::read(Double& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Double)))
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Double);
#ifdef ICE_BIG_ENDIAN
    Byte* dest = reinterpret_cast<Byte*>(&v) + sizeof(Double) - 1;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest-- = *src++;
    *dest = *src;
#else
    Byte* dest = reinterpret_cast<Byte*>(&v);
#  if defined(ICE_LITTLEBYTE_BIGWORD)
    dest[4] = *src++;
    dest[5] = *src++;
    dest[6] = *src++;
    dest[7] = *src++;
    dest[0] = *src++;
    dest[1] = *src++;
    dest[2] = *src++;
    dest[3] = *src;
#  else
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#  endif
#endif
}

void
Ice::InputStream::read(vector<Double>& v)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Double)));
    if(sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Double));
        v.resize(sz);
#ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&v[0]) + sizeof(Double) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Double);
        }
#elif defined(ICE_LITTLEBYTE_BIGWORD)
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&v[0]);
        for(int j = 0 ; j < sz ; ++j)
        {
            dest[4] = *src++;
            dest[5] = *src++;
            dest[6] = *src++;
            dest[7] = *src++;
            dest[0] = *src++;
            dest[1] = *src++;
            dest[2] = *src++;
            dest[3] = *src++;
            dest += sizeof(Double);
        }
#else
        copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
    else
    {
        v.clear();
    }
}


#ifdef ICE_CPP11_MAPPING
void
Ice::InputStream::read(pair<const Double*, const Double*>& v)
#else
void
Ice::InputStream::read(pair<const Double*, const Double*>& v, IceUtil::ScopedArray<Double>& result)
#endif
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Double)));
    if(sz > 0)
    {
#ifdef ICE_UNALIGNED
        v.first = reinterpret_cast<Double*>(i);
        i += sz * static_cast<int>(sizeof(Double));
        v.second = reinterpret_cast<Double*>(i);
#else

#  ifdef ICE_CPP11_MAPPING
        auto result = new double[sz];
        _deleters.push_back([result] { delete[] result; });
        v.first = result;
        v.second = result + sz;
#  else
        result.reset(new Double[sz]);
        v.first = result.get();
        v.second = result.get() + sz;
#  endif

        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(Double));
#  ifdef ICE_BIG_ENDIAN
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&result[0]) + sizeof(Double) - 1;
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            dest += 2 * sizeof(Double);
        }
#  elif defined(ICE_LITTLEBYTE_BIGWORD)
        const Byte* src = &(*begin);
        Byte* dest = reinterpret_cast<Byte*>(&result[0]);
        for(int j = 0 ; j < sz ; ++j)
        {
            dest[4] = *src++;
            dest[5] = *src++;
            dest[6] = *src++;
            dest[7] = *src++;
            dest[0] = *src++;
            dest[1] = *src++;
            dest[2] = *src++;
            dest[3] = *src++;
            dest += sizeof(Double);
        }

#  else
        copy(begin, i, reinterpret_cast<Byte*>(&result[0]));
#  endif
#endif
    }
    else
    {
#ifndef ICE_CPP11_MAPPING
        result.reset();
#endif
        v.first = v.second = 0;
    }
}

void
Ice::InputStream::read(std::string& v, bool convert)
{
    Int sz = readSize();
    if(sz > 0)
    {
        if(b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        if(!convert || !readConverted(v, sz))
        {
            string(reinterpret_cast<const char*>(&*i), reinterpret_cast<const char*>(&*i) + sz).swap(v);
        }
        i += sz;
    }
    else
    {
        v.clear();
    }
}

#ifdef ICE_CPP11_MAPPING
void
Ice::InputStream::read(const char*& vdata, size_t& vsize, bool convert)
{
    int sz = readSize();
    if(sz > 0)
    {
        if(b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        if(convert == false)
        {
            vdata = reinterpret_cast<const char*>(&*i);
            vsize = static_cast<size_t>(sz);
            i += sz;
        }
        else
        {
            string converted;
            if(readConverted(converted, sz))
            {
                if(converted.size() <= static_cast<size_t>(sz))
                {
                    //
                    // Write converted string directly into buffer
                    //
                    std::memcpy(i, converted.data(), converted.size());
                    vdata = reinterpret_cast<const char*>(&*i);
                    vsize = converted.size();
                }
                else
                {
                    auto holder = new string(std::move(converted));
                    _deleters.push_back([holder] { delete holder; });
                    vdata = holder->data();
                    vsize = holder->size();
                }
            }
            else
            {
                vdata = reinterpret_cast<const char*>(&*i);
                vsize = static_cast<size_t>(sz);
            }
            i += sz;
        }
    }
    else
    {
        vdata = 0;
        vsize = 0;
    }
}

#else

void
Ice::InputStream::read(const char*& vdata, size_t& vsize)
{
    Int sz = readSize();
    if(sz > 0)
    {
        if(b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        vdata = reinterpret_cast<const char*>(&*i);
        vsize = static_cast<size_t>(sz);
        i += sz;
    }
    else
    {
        vdata = 0;
        vsize = 0;
    }
}

void
Ice::InputStream::read(const char*& vdata, size_t& vsize, string& holder)
{
    Int sz = readSize();
    if(sz > 0)
    {
        if(b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        if(readConverted(holder, sz))
        {
            vdata = holder.data();
            vsize = holder.size();
        }
        else
        {
            vdata = reinterpret_cast<const char*>(&*i);
            vsize = static_cast<size_t>(sz);
        }
        i += sz;
    }
    else
    {
        holder.clear();
        vdata = 0;
        vsize = 0;
    }
}
#endif

bool
Ice::InputStream::readConverted(string& v, int sz)
{
    try
    {
        bool converted = false;

        //
        // NOTE: When using an _instance, we get a const& on the string reference to
        // not have to increment unecessarily its reference count.
        //

        if(_instance)
        {
            const StringConverterPtr& stringConverter = _instance->getStringConverter();
            if(stringConverter)
            {
                stringConverter->fromUTF8(i, i + sz, v);
                converted = true;
            }
        }
        else
        {
            StringConverterPtr stringConverter = getProcessStringConverter();
            if(stringConverter)
            {
                stringConverter->fromUTF8(i, i + sz, v);
                converted = true;
            }
        }

        return converted;
    }
    catch(const IllegalConversionException& ex)
    {
        throw StringConversionException(__FILE__, __LINE__, ex.reason());
    }
}

void
Ice::InputStream::read(vector<string>& v, bool convert)
{
    Int sz = readAndCheckSeqSize(1);
    if(sz > 0)
    {
        v.resize(sz);
        for(int j = 0; j < sz; ++j)
        {
            read(v[j], convert);
        }
    }
    else
    {
       v.clear();
    }
}

void
Ice::InputStream::read(wstring& v)
{
    Int sz = readSize();
    if(sz > 0)
    {
        if(b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        try
        {
            if(_instance)
            {
                const WstringConverterPtr& wstringConverter = _instance->getWstringConverter();
                wstringConverter->fromUTF8(i, i + sz, v);
            }
            else
            {
                WstringConverterPtr wstringConverter = getProcessWstringConverter();
                wstringConverter->fromUTF8(i, i + sz, v);
            }

            i += sz;
        }
        catch(const IllegalConversionException& ex)
        {
            throw StringConversionException(__FILE__, __LINE__, ex.reason());
        }
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(vector<wstring>& v)
{
    Int sz = readAndCheckSeqSize(1);
    if(sz > 0)
    {
        v.resize(sz);
        for(int j = 0; j < sz; ++j)
        {
            read(v[j]);
        }
    }
    else
    {
       v.clear();
    }
}

#ifdef ICE_CPP11_MAPPING
shared_ptr<ObjectPrx>
Ice::InputStream::readProxy()
{
    if(!_instance)
    {
        throw MarshalException(__FILE__, __LINE__, "cannot unmarshal a proxy without a communicator");
    }

    return _instance->proxyFactory()->streamToProxy(this);
}
#else
void
Ice::InputStream::read(ObjectPrx& v)
{
    if(!_instance)
    {
        throw MarshalException(__FILE__, __LINE__, "cannot unmarshal a proxy without a communicator");
    }

    v = _instance->proxyFactory()->streamToProxy(this);
}
#endif

Int
Ice::InputStream::readEnum(Int maxValue)
{
    if(getEncoding() == Encoding_1_0)
    {
        if(maxValue < 127)
        {
            Byte value;
            read(value);
            return value;
        }
        else if(maxValue < 32767)
        {
            Short value;
            read(value);
            return value;
        }
        else
        {
            Int value;
            read(value);
            return value;
        }
    }
    else
    {
        return readSize();
    }
}

void
Ice::InputStream::throwException(ICE_IN(ICE_USER_EXCEPTION_FACTORY) factory)
{
    initEncaps();
    _currentEncaps->decoder->throwException(factory);
}

bool
Ice::InputStream::readOptImpl(Int readTag, OptionalFormat expectedFormat)
{
    if(getEncoding() == Encoding_1_0)
    {
        return false; // Optional members aren't supported with the 1.0 encoding.
    }

    while(true)
    {
        if(i >= b.begin() + _currentEncaps->start + _currentEncaps->sz)
        {
            return false; // End of encapsulation also indicates end of optionals.
        }

        Byte v;
        read(v);
        if(v == OPTIONAL_END_MARKER)
        {
            --i; // Rewind
            return false;
        }

        OptionalFormat format = static_cast<OptionalFormat>(v & 0x07); // First 3 bits.
        Int tag = static_cast<Int>(v >> 3);
        if(tag == 30)
        {
            tag = readSize();
        }

        if(tag > readTag)
        {
            i -= tag < 30 ? 1 : (tag < 255 ? 2 : 6); // Rewind
            return false; // No optional data members with the requested tag.
        }
        else if(tag < readTag)
        {
            skipOptional(format); // Skip optional data members
        }
        else
        {
            if(format != expectedFormat)
            {
                ostringstream os;
                os << "invalid optional data member `" << tag << "': unexpected format";
                throw MarshalException(__FILE__, __LINE__, os.str());
            }
            return true;
        }
    }
    return true; // Keep the compiler happy.
}

void
Ice::InputStream::skipOptional(OptionalFormat type)
{
    switch(type)
    {
        case ICE_SCOPED_ENUM(OptionalFormat, F1):
        {
            skip(1);
            break;
        }
        case ICE_SCOPED_ENUM(OptionalFormat, F2):
        {
            skip(2);
            break;
        }
        case ICE_SCOPED_ENUM(OptionalFormat, F4):
        {
            skip(4);
            break;
        }
        case ICE_SCOPED_ENUM(OptionalFormat, F8):
        {
            skip(8);
            break;
        }
        case ICE_SCOPED_ENUM(OptionalFormat, Size):
        {
            skipSize();
            break;
        }
        case ICE_SCOPED_ENUM(OptionalFormat, VSize):
        {
            skip(readSize());
            break;
        }
        case ICE_SCOPED_ENUM(OptionalFormat, FSize):
        {
            Int sz;
            read(sz);
            if(sz < 0)
            {
                throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            skip(sz);
            break;
        }
        case ICE_SCOPED_ENUM(OptionalFormat, Class):
        {
            read(0, 0);
            break;
        }
    }
}

void
Ice::InputStream::skipOptionals()
{
    //
    // Skip remaining un-read optional members.
    //
    while(true)
    {
        if(i >= b.begin() + _currentEncaps->start + _currentEncaps->sz)
        {
            return; // End of encapsulation also indicates end of optionals.
        }

        Byte v;
        read(v);
        if(v == OPTIONAL_END_MARKER)
        {
            return;
        }

        OptionalFormat format = static_cast<OptionalFormat>(v & 0x07); // Read first 3 bits.
        if(static_cast<Int>(v >> 3) == 30)
        {
            skipSize();
        }
        skipOptional(format);
    }
}

void
Ice::InputStream::throwUnmarshalOutOfBoundsException(const char* file, int line)
{
    throw UnmarshalOutOfBoundsException(file, line);
}

void
Ice::InputStream::throwEncapsulationException(const char* file, int line)
{
    throw EncapsulationException(file, line);
}

string
Ice::InputStream::resolveCompactId(int id) const
{
    string type;

#ifdef ICE_CPP11_MAPPING
    function<string(int)> resolver = compactIdResolver();
#else
    CompactIdResolverPtr resolver = compactIdResolver();
#endif

    if(resolver)
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            type = resolver(id);
#else
            type = resolver->resolve(id);
#endif
        }
        catch(const LocalException&)
        {
            throw;
        }
        catch(const std::exception& ex)
        {
            ostringstream ostr;
            ostr << "exception in CompactIdResolver for ID " << id;
            string msg = ostr.str();
            string what = ex.what();
            if(!what.empty())
            {
                msg += ":\n" + what;
            }
            throw MarshalException(__FILE__, __LINE__, msg);
        }
        catch(...)
        {
            ostringstream ostr;
            ostr << "unknown exception in CompactIdResolver for ID " << id;
            throw MarshalException(__FILE__, __LINE__, ostr.str());
        }
    }

    return type;
}

void
Ice::InputStream::postUnmarshal(const ValuePtr& v) const
{
    try
    {
#ifndef ICE_CPP11_MAPPING
        if(_collectObjects)
        {
            v->ice_collectable(true);
        }
#endif
        v->ice_postUnmarshal();
    }
    catch(const std::exception& ex)
    {
        if(logger())
        {
            Warning out(logger());
            out << "std::exception raised by ice_postUnmarshal:\n" << ex;
        }
    }
    catch(...)
    {
        if(logger())
        {
            Warning out(logger());
            out << "unknown exception raised by ice_postUnmarshal";
        }
    }
}

void
Ice::InputStream::traceSkipSlice(const string& typeId, SliceType sliceType) const
{
    if(_traceSlicing && logger())
    {
        traceSlicing(sliceType == ExceptionSlice ? "exception" : "object", typeId, "Slicing", logger());
    }
}

ValueFactoryManagerPtr
Ice::InputStream::valueFactoryManager() const
{
    if(_valueFactoryManager)
    {
        return _valueFactoryManager;
    }
    else if(_instance)
    {
        return _instance->initializationData().valueFactoryManager;
    }

    return 0;
}

LoggerPtr
Ice::InputStream::logger() const
{
    if(_logger)
    {
        return _logger;
    }
    else if(_instance)
    {
        return _instance->initializationData().logger;
    }

    return 0;
}

#ifdef ICE_CPP11_MAPPING
function<string(int)>
Ice::InputStream::compactIdResolver() const
{
    if(_compactIdResolver)
    {
        return _compactIdResolver;
    }
    else if(_instance)
    {
        return _instance->initializationData().compactIdResolver;
    }

    return nullptr;
}
#else
CompactIdResolverPtr
Ice::InputStream::compactIdResolver() const
{
    if(_compactIdResolver)
    {
        return _compactIdResolver;
    }
    else if(_instance)
    {
        return _instance->initializationData().compactIdResolver;
    }

    return 0;
}
#endif

void
Ice::InputStream::initEncaps()
{
    if(!_currentEncaps) // Lazy initialization.
    {
        _currentEncaps = &_preAllocatedEncaps;
        _currentEncaps->encoding = _encoding;
        _currentEncaps->sz = static_cast<Ice::Int>(b.size());
    }

    if(!_currentEncaps->decoder) // Lazy initialization.
    {
        ValueFactoryManagerPtr vfm = valueFactoryManager();
        if(_currentEncaps->encoding == Encoding_1_0)
        {
            _currentEncaps->decoder = new EncapsDecoder10(this, _currentEncaps, _sliceValues, vfm);
        }
        else
        {
            _currentEncaps->decoder = new EncapsDecoder11(this, _currentEncaps, _sliceValues, vfm);
        }
    }
}

Ice::InputStream::EncapsDecoder::~EncapsDecoder()
{
    // Out of line to avoid weak vtable
}

string
Ice::InputStream::EncapsDecoder::readTypeId(bool isIndex)
{
    if(isIndex)
    {
        Int index = _stream->readSize();
        TypeIdMap::const_iterator k = _typeIdMap.find(index);
        if(k == _typeIdMap.end())
        {
            throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        return k->second;
    }
    else
    {
        string typeId;
        _stream->read(typeId, false);
        _typeIdMap.insert(make_pair(++_typeIdIndex, typeId));
        return typeId;
    }
}

Ice::ValuePtr
Ice::InputStream::EncapsDecoder::newInstance(const string& typeId)
{
    Ice::ValuePtr v;

    //
    // Try to find a factory registered for the specific type.
    //
#ifdef ICE_CPP11_MAPPING
    function<ValuePtr(const string&)> userFactory;
    if(_valueFactoryManager)
    {
        userFactory = _valueFactoryManager->find(typeId);
        if(userFactory)
        {
            v = userFactory(typeId);
        }
    }
#else
    ValueFactoryPtr userFactory;
    if(_valueFactoryManager)
    {
        userFactory = _valueFactoryManager->find(typeId);
        if(userFactory)
        {
            v = userFactory->create(typeId);
        }
    }
#endif
    //
    // If that fails, invoke the default factory if one has been registered.
    //
    if(!v && _valueFactoryManager)
    {
        userFactory = _valueFactoryManager->find("");
        if(userFactory)
        {
#ifdef ICE_CPP11_MAPPING
            v = userFactory(typeId);
#else
            v = userFactory->create(typeId);
#endif
        }
    }

    //
    // Last chance: check the table of static factories (i.e.,
    // automatically generated factories for concrete classes).
    //
    if(!v)
    {
#ifdef ICE_CPP11_MAPPING
        function<ValuePtr(const string&)> of = IceInternal::factoryTable->getValueFactory(typeId);
        if(of)
        {
            v = of(typeId);
            assert(v);
        }
#else
        ValueFactoryPtr of = IceInternal::factoryTable->getValueFactory(typeId);
        if(of)
        {
            v = of->create(typeId);
            assert(v);
        }
#endif
    }
    return v;
}

void
Ice::InputStream::EncapsDecoder::addPatchEntry(Int index, PatchFunc patchFunc, void* patchAddr)
{
    assert(index > 0);

    //
    // Check if we already unmarshaled the object. If that's the case,
    // just patch the object smart pointer and we're done.
    //
    IndexToPtrMap::iterator p = _unmarshaledMap.find(index);
    if(p != _unmarshaledMap.end())
    {
        (*patchFunc)(patchAddr, p->second);
        return;
    }

    //
    // Add a patch entry if the object isn't unmarshaled yet, the
    // smart pointer will be patched when the instance is
    // unmarshaled.
    //

    PatchMap::iterator q = _patchMap.find(index);
    if(q == _patchMap.end())
    {
        //
        // We have no outstanding instances to be patched for this
        // index, so make a new entry in the patch map.
        //
        q = _patchMap.insert(make_pair(index, PatchList())).first;
    }

    //
    // Append a patch entry for this instance.
    //
    PatchEntry e;
    e.patchFunc = patchFunc;
    e.patchAddr = patchAddr;
    q->second.push_back(e);
}

void
Ice::InputStream::EncapsDecoder::unmarshal(Int index, const Ice::ValuePtr& v)
{
    //
    // Add the object to the map of unmarshaled instances, this must
    // be done before reading the instances (for circular references).
    //
    _unmarshaledMap.insert(make_pair(index, v));

    //
    // Read the object.
    //
    v->_iceRead(_stream);

    //
    // Patch all instances now that the object is unmarshaled.
    //
    PatchMap::iterator patchPos = _patchMap.find(index);
    if(patchPos != _patchMap.end())
    {
        assert(patchPos->second.size() > 0);

        //
        // Patch all pointers that refer to the instance.
        //
        for(PatchList::iterator k = patchPos->second.begin(); k != patchPos->second.end(); ++k)
        {
            (*k->patchFunc)(k->patchAddr, v);
        }

        //
        // Clear out the patch map for that index -- there is nothing left
        // to patch for that index for the time being.
        //
        _patchMap.erase(patchPos);
    }

    if(_valueList.empty() && _patchMap.empty())
    {
        _stream->postUnmarshal(v);
    }
    else
    {
        _valueList.push_back(v);

        if(_patchMap.empty())
        {
            //
            // Iterate over the value list and invoke ice_postUnmarshal on
            // each value. We must do this after all values have been
            // unmarshaled in order to ensure that any value data members
            // have been properly patched.
            //
            for(ValueList::iterator p = _valueList.begin(); p != _valueList.end(); ++p)
            {
                _stream->postUnmarshal(*p);
            }
            _valueList.clear();
        }
    }
}

void
Ice::InputStream::EncapsDecoder10::read(PatchFunc patchFunc, void* patchAddr)
{
    assert(patchFunc && patchAddr);

    //
    // Object references are encoded as a negative integer in 1.0.
    //
    Int index;
    _stream->read(index);
    if(index > 0)
    {
        throw MarshalException(__FILE__, __LINE__, "invalid object id");
    }
    index = -index;

    if(index == 0)
    {
        //
        // Calling the patch function for null instances is necessary for correct functioning of Ice for
        // Python and Ruby.
        //
        ValuePtr nil;
        patchFunc(patchAddr, nil);
    }
    else
    {
        addPatchEntry(index, patchFunc, patchAddr);
    }
}

void
Ice::InputStream::EncapsDecoder10::throwException(ICE_IN(ICE_USER_EXCEPTION_FACTORY) factory)
{
    assert(_sliceType == NoSlice);

    //
    // User exception with the 1.0 encoding start with a boolean flag
    // that indicates whether or not the exception has classes.
    //
    // This allows reading the pending values even if some part of
    // the exception was sliced.
    //
    bool usesClasses;
    _stream->read(usesClasses);

    _sliceType = ExceptionSlice;
    _skipFirstSlice = false;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _typeId;
    ICE_USER_EXCEPTION_FACTORY exceptionFactory = factory;
    while(true)
    {
        //
        // Look for a statically-generated factory for this ID.
        //
        if(!exceptionFactory)
        {
            exceptionFactory = factoryTable->getExceptionFactory(_typeId);
        }

        //
        // We found a factory, we get out of this loop.
        //
        if(exceptionFactory)
        {
            //
            // Got factory -- ask the factory to instantiate the
            // exception, initialize the exception members, and throw
            // the exception.
            //
            try
            {
#ifdef ICE_CPP11_MAPPING
                exceptionFactory(_typeId);
#else
                exceptionFactory->createAndThrow(_typeId);
#endif
            }
            catch(UserException& ex)
            {
                ex._read(_stream);
                if(usesClasses)
                {
                    readPendingValues();
                }
                throw;

                // Never reached.
            }
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();
        try
        {
            startSlice();
        }
        catch(UnmarshalOutOfBoundsException& ex)
        {
            //
            // An oversight in the 1.0 encoding means there is no marker to indicate
            // the last slice of an exception. As a result, we just try to read the
            // next type ID, which raises UnmarshalOutOfBoundsException when the
            // input buffer underflows.
            //
            // Set the reason member to a more helpful message.
            //
            ex.reason = "unknown exception type `" + mostDerivedId + "'";
            throw;
        }
    }
}

void
#ifndef NDEBUG
Ice::InputStream::EncapsDecoder10::startInstance(SliceType sliceType)
#else
Ice::InputStream::EncapsDecoder10::startInstance(SliceType)
#endif
{
    assert(_sliceType == sliceType);
    _skipFirstSlice = true;
}

SlicedDataPtr
Ice::InputStream::EncapsDecoder10::endInstance(bool)
{
    //
    // Read the Ice::Value slice.
    //
    if(_sliceType == ValueSlice)
    {
        startSlice();
        Int sz = _stream->readSize(); // For compatibility with the old AFM.
        if(sz != 0)
        {
            throw MarshalException(__FILE__, __LINE__, "invalid Object slice");
        }
        endSlice();
    }
    _sliceType = NoSlice;
    return 0;
}

const std::string&
Ice::InputStream::EncapsDecoder10::startSlice()
{
    //
    // If first slice, don't read the header, it was already read in
    // readInstance or throwException to find the factory.
    //
    if(_skipFirstSlice)
    {
        _skipFirstSlice = false;
        return _typeId;
    }

    //
    // For values, first read the type ID boolean which indicates
    // whether or not the type ID is encoded as a string or as an
    // index. For exceptions, the type ID is always encoded as a
    // string.
    //
    if(_sliceType == ValueSlice)
    {
        bool isIndex;
        _stream->read(isIndex);
        _typeId = readTypeId(isIndex);
    }
    else
    {
        _stream->read(_typeId, false);
    }

    _stream->read(_sliceSize);
    if(_sliceSize < 4)
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    return _typeId;
}

void
Ice::InputStream::EncapsDecoder10::endSlice()
{
}

void
Ice::InputStream::EncapsDecoder10::skipSlice()
{
    _stream->traceSkipSlice(_typeId, _sliceType);
    assert(_sliceSize >= 4);
    _stream->skip(_sliceSize - sizeof(Int));
}

void
Ice::InputStream::EncapsDecoder10::readPendingValues()
{
    Int num;
    do
    {
        num = _stream->readSize();
        for(Int k = num; k > 0; --k)
        {
            readInstance();
        }
    }
    while(num);

    if(!_patchMap.empty())
    {
        //
        // If any entries remain in the patch map, the sender has sent an index for an object, but failed
        // to supply the object.
        //
        throw MarshalException(__FILE__, __LINE__, "index for class received, but no instance");
    }
}

void
Ice::InputStream::EncapsDecoder10::readInstance()
{
    Int index;
    _stream->read(index);

    if(index <= 0)
    {
        throw MarshalException(__FILE__, __LINE__, "invalid object id");
    }

    _sliceType = ValueSlice;
    _skipFirstSlice = false;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _typeId;
    ValuePtr v;
    while(true)
    {
        //
        // For the 1.0 encoding, the type ID for the base Object class
        // marks the last slice.
        //
        if(_typeId == Object::ice_staticId())
        {
            throw NoValueFactoryException(__FILE__, __LINE__, "", mostDerivedId);
        }

        v = newInstance(_typeId);

        //
        // We found a factory, we get out of this loop.
        //
        if(v)
        {
            break;
        }

        //
        // If value slicing is disabled, stop unmarshaling.
        //
        if(!_sliceValues)
        {
            throw NoValueFactoryException(__FILE__, __LINE__, "no value factory found and value slicing is disabled",
                                           _typeId);
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();
        startSlice(); // Read next Slice header for next iteration.
    }

    //
    // Unmarshal the instance and add it to the map of unmarshaled instances.
    //
    unmarshal(index, v);
}

void
Ice::InputStream::EncapsDecoder11::read(PatchFunc patchFunc, void* patchAddr)
{
    Int index = _stream->readSize();
    if(index < 0)
    {
        throw MarshalException(__FILE__, __LINE__, "invalid object id");
    }
    else if(index == 0)
    {
        //
        // Calling the patch function for null instances is necessary for correct functioning of Ice for
        // Python and Ruby.
        //
        if(patchFunc)
        {
            ValuePtr nil;
            patchFunc(patchAddr, nil);
        }
    }
    else if(_current && _current->sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
    {
        //
        // When reading an object within a slice and there's an
        // indirect object table, always read an indirect reference
        // that points to an object from the indirect object table
        // marshaled at the end of the Slice.
        //
        // Maintain a list of indirect references. Note that the
        // indirect index starts at 1, so we decrement it by one to
        // derive an index into the indirection table that we'll read
        // at the end of the slice.
        //
        if(patchFunc)
        {
            IndirectPatchEntry e;
            e.index = index - 1;
            e.patchFunc = patchFunc;
            e.patchAddr = patchAddr;
            _current->indirectPatchList.push_back(e);
        }
    }
    else
    {
        readInstance(index, patchFunc, patchAddr);
    }
}

void
Ice::InputStream::EncapsDecoder11::throwException(ICE_IN(ICE_USER_EXCEPTION_FACTORY) factory)
{
    assert(!_current);

    push(ExceptionSlice);

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _current->typeId;
    ICE_USER_EXCEPTION_FACTORY exceptionFactory = factory;
    while(true)
    {
        //
        // Look for a statically-generated factory for this ID.
        //
        if(!exceptionFactory)
        {
            exceptionFactory = factoryTable->getExceptionFactory(_current->typeId);
        }

        //
        // We found a factory, we get out of this loop.
        //
        if(exceptionFactory)
        {
            //
            // Got factory -- ask the factory to instantiate the
            // exception, initialize the exception members, and throw
            // the exception.
            //
            try
            {
#ifdef ICE_CPP11_MAPPING
                exceptionFactory(_current->typeId);
#else
                exceptionFactory->createAndThrow(_current->typeId);
#endif
            }
            catch(UserException& ex)
            {
                ex._read(_stream);
                throw;

                // Never reached.
            }
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();

        //
        // If this is the last slice, raise an exception and stop un-marshalling.
        //
        if(_current->sliceFlags & FLAG_IS_LAST_SLICE)
        {
            throw UnknownUserException(__FILE__, __LINE__, mostDerivedId);
        }

        startSlice();
    }
}

void
#ifndef NDEBUG
Ice::InputStream::EncapsDecoder11::startInstance(SliceType sliceType)
#else
Ice::InputStream::EncapsDecoder11::startInstance(SliceType)
#endif
{
    assert(_current->sliceType == sliceType);
    _current->skipFirstSlice = true;
}

SlicedDataPtr
Ice::InputStream::EncapsDecoder11::endInstance(bool preserve)
{
    SlicedDataPtr slicedData;
    if(preserve)
    {
        slicedData = readSlicedData();
    }
    _current->slices.clear();
    _current->indirectionTables.clear();
    _current = _current->previous;
    return slicedData;
}

const std::string&
Ice::InputStream::EncapsDecoder11::startSlice()
{
    //
    // If first slice, don't read the header, it was already read in
    // readInstance or throwException to find the factory.
    //
    if(_current->skipFirstSlice)
    {
        _current->skipFirstSlice = false;
        return _current->typeId;
    }

    _stream->read(_current->sliceFlags);

    //
    // Read the type ID, for value slices the type ID is encoded as a
    // string or as an index, for exceptions it's always encoded as a
    // string.
    //
    if(_current->sliceType == ValueSlice)
    {
        if((_current->sliceFlags & FLAG_HAS_TYPE_ID_COMPACT) == FLAG_HAS_TYPE_ID_COMPACT) // Must be checked first!
        {
            _current->typeId.clear();
            _current->compactId = _stream->readSize();
        }
        else if(_current->sliceFlags & (FLAG_HAS_TYPE_ID_STRING | FLAG_HAS_TYPE_ID_INDEX))
        {
            _current->typeId = readTypeId(_current->sliceFlags & FLAG_HAS_TYPE_ID_INDEX);
            _current->compactId = -1;
        }
        else
        {
            // Only the most derived slice encodes the type ID for the compact format.
            _current->typeId.clear();
            _current->compactId = -1;
        }
    }
    else
    {
        _stream->read(_current->typeId, false);
    }

    //
    // Read the slice size if necessary.
    //
    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        _stream->read(_current->sliceSize);
        if(_current->sliceSize < 4)
        {
            throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
    }
    else
    {
        _current->sliceSize = 0;
    }

    return _current->typeId;
}

void
Ice::InputStream::EncapsDecoder11::endSlice()
{
    if(_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        _stream->skipOptionals();
    }

    //
    // Read the indirect object table if one is present.
    //
    if(_current->sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
    {
        IndexList indirectionTable(_stream->readAndCheckSeqSize(1));
        for(IndexList::iterator p = indirectionTable.begin(); p != indirectionTable.end(); ++p)
        {
            *p = readInstance(_stream->readSize(), 0, 0);
        }

        //
        // Sanity checks. If there are optional members, it's possible
        // that not all object references were read if they are from
        // unknown optional data members.
        //
        if(indirectionTable.empty())
        {
            throw MarshalException(__FILE__, __LINE__, "empty indirection table");
        }
        if(_current->indirectPatchList.empty() && !(_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS))
        {
            throw MarshalException(__FILE__, __LINE__, "no references to indirection table");
        }

        //
        // Convert indirect references into direct references.
        //
        IndirectPatchList::iterator p;
        for(p = _current->indirectPatchList.begin(); p != _current->indirectPatchList.end(); ++p)
        {
            assert(p->index >= 0);
            if(p->index >= static_cast<Int>(indirectionTable.size()))
            {
                throw MarshalException(__FILE__, __LINE__, "indirection out of range");
            }
            addPatchEntry(indirectionTable[p->index], p->patchFunc, p->patchAddr);
        }
        _current->indirectPatchList.clear();
    }
}

void
Ice::InputStream::EncapsDecoder11::skipSlice()
{
    _stream->traceSkipSlice(_current->typeId, _current->sliceType);

    Container::iterator start = _stream->i;

    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        assert(_current->sliceSize >= 4);
        _stream->skip(_current->sliceSize - sizeof(Int));
    }
    else
    {
        if(_current->sliceType == ValueSlice)
        {
            throw NoValueFactoryException(__FILE__, __LINE__,
                                          "no value factory found and compact format prevents "
                                          "slicing (the sender should use the sliced format instead)",
                                          _current->typeId);
        }
        else
        {
            throw UnknownUserException(__FILE__, __LINE__, _current->typeId);
        }
    }

    //
    // Preserve this slice.
    //
    SliceInfoPtr info = ICE_MAKE_SHARED(SliceInfo);
    info->typeId = _current->typeId;
    info->compactId = _current->compactId;
    info->hasOptionalMembers = _current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS;
    info->isLastSlice = _current->sliceFlags & FLAG_IS_LAST_SLICE;
    if(info->hasOptionalMembers)
    {
        //
        // Don't include the optional member end marker. It will be re-written by
        // endSlice when the sliced data is re-written.
        //
        vector<Byte>(start, _stream->i - 1).swap(info->bytes);
    }
    else
    {
        vector<Byte>(start, _stream->i).swap(info->bytes);
    }

    _current->indirectionTables.push_back(IndexList());

    //
    // Read the indirect object table. We read the instances or their
    // IDs if the instance is a reference to an already un-marhsaled
    // object.
    //
    // The SliceInfo object sequence is initialized only if
    // readSlicedData is called.
    //
    if(_current->sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
    {
        IndexList& table = _current->indirectionTables.back();
        table.resize(_stream->readAndCheckSeqSize(1));
        for(IndexList::iterator p = table.begin(); p != table.end(); ++p)
        {
            *p = readInstance(_stream->readSize(), 0, 0);
        }
    }

    _current->slices.push_back(info);
}

bool
Ice::InputStream::EncapsDecoder11::readOptional(Ice::Int readTag, Ice::OptionalFormat expectedFormat)
{
    if(!_current)
    {
        return _stream->readOptImpl(readTag, expectedFormat);
    }
    else if(_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        return _stream->readOptImpl(readTag, expectedFormat);
    }
    return false;
}

Int
Ice::InputStream::EncapsDecoder11::readInstance(Int index, PatchFunc patchFunc, void* patchAddr)
{
    assert(index > 0);

    if(index > 1)
    {
        if(patchFunc)
        {
            addPatchEntry(index, patchFunc, patchAddr);
        }
        return index;
    }

    push(ValueSlice);

    //
    // Get the object ID before we start reading slices. If some
    // slices are skiped, the indirect object table are still read and
    // might read other instances.
    //
    index = ++_valueIdIndex;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _current->typeId;
    Ice::ValuePtr v;
    while(true)
    {
        if(_current->compactId >= 0)
        {
            //
            // Translate a compact (numeric) type ID into a string type ID.
            //
            _current->typeId = _stream->resolveCompactId(_current->compactId);
            if(_current->typeId.empty())
            {
                _current->typeId = IceInternal::factoryTable->getTypeId(_current->compactId);
            }
        }

        if(!_current->typeId.empty())
        {
            v = newInstance(_current->typeId);

            //
            // We found a factory, we get out of this loop.
            //
            if(v)
            {
                break;
            }
        }

        //
        // If value slicing is disabled, stop unmarshaling.
        //
        if(!_sliceValues)
        {
            throw NoValueFactoryException(__FILE__, __LINE__, "no value factory found and value slicing is disabled",
                                          _current->typeId);
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();

        //
        // If this is the last slice, keep the object as an opaque UnknownSlicedValue.
        //
        if(_current->sliceFlags & FLAG_IS_LAST_SLICE)
        {
            //
            // Provide a factory with an opportunity to supply the object.
            // We pass the "::Ice::Object" ID to indicate that this is the
            // last chance to preserve the object.
            //
            v = newInstance(Object::ice_staticId());
            if(!v)
            {
                v = ICE_MAKE_SHARED(UnknownSlicedValue, mostDerivedId);
            }

            break;
        }

        startSlice(); // Read next Slice header for next iteration.
    }

    //
    // Unmarshal the object.
    //
    unmarshal(index, v);

    if(!_current && !_patchMap.empty())
    {
        //
        // If any entries remain in the patch map, the sender has sent an index for an object, but failed
        // to supply the object.
        //
        throw MarshalException(__FILE__, __LINE__, "index for class received, but no instance");
    }

    if(patchFunc)
    {
        patchFunc(patchAddr, v);
    }
    return index;
}

SlicedDataPtr
Ice::InputStream::EncapsDecoder11::readSlicedData()
{
    if(_current->slices.empty()) // No preserved slices.
    {
        return 0;
    }

    //
    // The indirectionTables member holds the indirection table for
    // each slice in slices.
    //
    assert(_current->slices.size() == _current->indirectionTables.size());
    for(SliceInfoSeq::size_type n = 0; n < _current->slices.size(); ++n)
    {
        //
        // We use the "instances" list in SliceInfo to hold references
        // to the target instances. Note that the instances might not have
        // been read yet in the case of a circular reference to an
        // enclosing instance.
        //
        const IndexList& table = _current->indirectionTables[n];
        vector<ValuePtr>& instances = _current->slices[n]->instances;
        instances.resize(table.size());
        IndexList::size_type j = 0;
        for(IndexList::const_iterator p = table.begin(); p != table.end(); ++p)
        {
#ifdef ICE_CPP11_MAPPING
            addPatchEntry(*p, &patchHandle<Value>, &instances[j++]);
#else
            addPatchEntry(*p, &patchHandle<Object>, &instances[j++]);
#endif
        }
    }
    return ICE_MAKE_SHARED(SlicedData, _current->slices);
}
