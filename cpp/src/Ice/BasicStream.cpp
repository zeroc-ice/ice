// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/BasicStream.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ObjectFactory.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/FactoryTableInit.h>
#include <Ice/TraceUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/StringConverter.h>
#include <IceUtil/Unicode.h>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

class StreamUTF8BufferI : public Ice::UTF8Buffer
{
public:
    
    StreamUTF8BufferI(BasicStream& stream) : 
        _stream(stream)
    {
    }
    
    Ice::Byte*
    getMoreBytes(size_t howMany, Ice::Byte* firstUnused)
    {
        assert(howMany > 0);

        if(firstUnused != 0)
        {
            //
            // Return unused bytes
            //
            _stream.b.resize(firstUnused - _stream.b.begin());
        }

        //
        // Index of first unused byte
        //
        Buffer::Container::size_type pos = _stream.b.size();

        //
        // Since resize may reallocate the buffer, when firstUnused != 0, the
        // return value can be != firstUnused
        //
        _stream.resize(pos + howMany);

        return &_stream.b[pos];
    }

private:

    BasicStream& _stream;
};

}

IceInternal::BasicStream::BasicStream(Instance* instance, bool unlimited) :
    IceInternal::Buffer(instance->messageSizeMax()),
    _instance(instance),
    _closure(0),
    _currentReadEncaps(0),
    _currentWriteEncaps(0),
    _traceSlicing(-1),
    _sliceObjects(true),
    _messageSizeMax(_instance->messageSizeMax()), // Cached for efficiency.
    _unlimited(unlimited),
    _stringConverter(instance->initializationData().stringConverter),
    _wstringConverter(instance->initializationData().wstringConverter),
    _startSeq(-1),
    _objectList(0)
{
}

void
IceInternal::BasicStream::clear()
{
    while(_currentReadEncaps && _currentReadEncaps != &_preAllocatedReadEncaps)
    {
        ReadEncaps* oldEncaps = _currentReadEncaps;
        _currentReadEncaps = _currentReadEncaps->previous;
        delete oldEncaps;
    }

    while(_currentWriteEncaps && _currentWriteEncaps != &_preAllocatedWriteEncaps)
    {
        WriteEncaps* oldEncaps = _currentWriteEncaps;
        _currentWriteEncaps = _currentWriteEncaps->previous;
        delete oldEncaps;
    }

    _startSeq = -1;

    delete _objectList;
    _objectList = 0;
    _sliceObjects = true;
}

void*
IceInternal::BasicStream::closure() const
{
    return _closure;
}

void*
IceInternal::BasicStream::closure(void* p)
{
    void* prev = _closure;
    _closure = p;
    return prev;
}

void
IceInternal::BasicStream::swap(BasicStream& other)
{
    assert(_instance == other._instance);

    swapBuffer(other);

    std::swap(_closure, other._closure);

    //
    // Swap is never called for BasicStreams that have more than one
    // encaps.
    //
    assert(!_currentReadEncaps || _currentReadEncaps == &_preAllocatedReadEncaps);
    assert(!_currentWriteEncaps || _currentWriteEncaps == &_preAllocatedWriteEncaps);
    assert(!other._currentReadEncaps || other._currentReadEncaps == &other._preAllocatedReadEncaps);
    assert(!other._currentWriteEncaps || other._currentWriteEncaps == &other._preAllocatedWriteEncaps);

    if(_currentReadEncaps || other._currentReadEncaps)
    {
        _preAllocatedReadEncaps.swap(other._preAllocatedReadEncaps);

        if(!_currentReadEncaps)
        {
            _currentReadEncaps = &_preAllocatedReadEncaps;
            other._currentReadEncaps = 0;
        }
        else if(!other._currentReadEncaps)
        {
            other._currentReadEncaps = &other._preAllocatedReadEncaps;
            _currentReadEncaps = 0;
        }
    }

    if(_currentWriteEncaps || other._currentWriteEncaps)
    {
        _preAllocatedWriteEncaps.swap(other._preAllocatedWriteEncaps);

        if(!_currentWriteEncaps)
        {
            _currentWriteEncaps = &_preAllocatedWriteEncaps;
            other._currentWriteEncaps = 0;
        }
        else if(!other._currentWriteEncaps)
        {
            other._currentWriteEncaps = &other._preAllocatedWriteEncaps;
            _currentWriteEncaps = 0;
        }
    }

    std::swap(_startSeq, other._startSeq);
    std::swap(_minSeqSize, other._minSeqSize);
    std::swap(_objectList, other._objectList);
    std::swap(_unlimited, other._unlimited);
}

void
IceInternal::BasicStream::WriteEncaps::swap(WriteEncaps& other)
{
    std::swap(start, other.start);

    std::swap(writeIndex, other.writeIndex);
    std::swap(toBeMarshaledMap, other.toBeMarshaledMap);
    std::swap(marshaledMap, other.marshaledMap);
    std::swap(typeIdMap, other.typeIdMap);
    std::swap(typeIdIndex, other.typeIdIndex);

    std::swap(previous, other.previous);
}

void
IceInternal::BasicStream::ReadEncaps::swap(ReadEncaps& other)
{
    std::swap(start, other.start);
    std::swap(sz, other.sz);

    std::swap(encodingMajor, other.encodingMajor);
    std::swap(encodingMinor, other.encodingMinor);

    std::swap(patchMap, other.patchMap);
    std::swap(unmarshaledMap, other.unmarshaledMap);
    std::swap(typeIdMap, other.typeIdMap);
    std::swap(typeIdIndex, other.typeIdIndex);

    std::swap(previous, other.previous);
}

void
IceInternal::BasicStream::endWriteEncapsChecked()
{
    if(!_currentWriteEncaps)
    {
        throw EncapsulationException(__FILE__, __LINE__, "not in an encapsulation");
    }
    endWriteEncaps();
}

void
IceInternal::BasicStream::endReadEncapsChecked()
{
    if(!_currentReadEncaps)
    {
        throw EncapsulationException(__FILE__, __LINE__, "not in an encapsulation");
    }
    endReadEncaps();
}

Int
IceInternal::BasicStream::getReadEncapsSize()
{
    assert(_currentReadEncaps);
    return _currentReadEncaps->sz - static_cast<Int>(sizeof(Int)) - 2;
}

void
IceInternal::BasicStream::skipEncaps()
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
    i += sz - sizeof(Int);
}

void
IceInternal::BasicStream::startWriteSlice()
{
    write(Int(0)); // Placeholder for the slice length.
    _writeSlice = b.size();
}

void
IceInternal::BasicStream::endWriteSlice()
{
    Int sz = static_cast<Int>(b.size() - _writeSlice + sizeof(Int));
    Byte* dest = &(*(b.begin() + _writeSlice - sizeof(Int)));
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&sz) + sizeof(Int) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&sz);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
IceInternal::BasicStream::startReadSlice()
{
    Int sz;
    read(sz);
    if(sz < 4)
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    _readSlice = i - b.begin();
}

void
IceInternal::BasicStream::endReadSlice()
{
}

void
IceInternal::BasicStream::skipSlice()
{
    Int sz;
    read(sz);
    if(sz < 4)
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    i += sz - sizeof(Int);
    if(i > b.end())
    {
        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
}

void
IceInternal::BasicStream::readAndCheckSeqSize(int minSize, Ice::Int& sz)
{
    readSize(sz);

    if(sz == 0)
    {
        return;
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
}

void
IceInternal::BasicStream::writeTypeId(const string& id)
{
    if(!_currentWriteEncaps || !_currentWriteEncaps->typeIdMap)
    {
        //
        // write(ObjectPtr) must be called first.
        //
        throw MarshalException(__FILE__, __LINE__, "type ids require an encapsulation");
    }

    TypeIdWriteMap::const_iterator k = _currentWriteEncaps->typeIdMap->find(id);
    if(k != _currentWriteEncaps->typeIdMap->end())
    {
        write(true);
        writeSize(k->second);
    }
    else
    {
        _currentWriteEncaps->typeIdMap->insert(make_pair(id, ++_currentWriteEncaps->typeIdIndex));
        write(false);
        write(id, false);
    }
}

void
IceInternal::BasicStream::readTypeId(string& id)
{
    if(!_currentReadEncaps || !_currentReadEncaps->typeIdMap)
    {
        //
        // read(PatchFunc, void*) must be called first.
        //
        throw MarshalException(__FILE__, __LINE__, "type ids require an encapsulation");
    }

    bool isIndex;
    read(isIndex);
    if(isIndex)
    {
        Ice::Int index;
        readSize(index);
        TypeIdReadMap::const_iterator k = _currentReadEncaps->typeIdMap->find(index);
        if(k == _currentReadEncaps->typeIdMap->end())
        {
            throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        id = k->second;
    }
    else
    {
        read(id, false);
        _currentReadEncaps->typeIdMap->insert(make_pair(++_currentReadEncaps->typeIdIndex, id));
    }
}

void
IceInternal::BasicStream::writeBlob(const vector<Byte>& v)
{
    if(!v.empty())
    {
        Container::size_type pos = b.size();
        resize(pos + v.size());
        memcpy(&b[pos], &v[0], v.size());
    }
}

void
IceInternal::BasicStream::readBlob(vector<Byte>& v, Int sz)
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

void IceInternal::BasicStream::write(Byte v, int end)
{
    if(v >= end)
    {
        throw MarshalException(__FILE__, __LINE__, "enumerator out of range");
    }
    write(v);
}

void
IceInternal::BasicStream::write(const Byte* begin, const Byte* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz);
        memcpy(&b[pos], begin, sz);
    }
}

void
IceInternal::BasicStream::read(Byte& b, int end)
{
    read(b);
    if(b >= end)
    {
        throw MarshalException(__FILE__, __LINE__, "enumerator out of range");
    }
}

void
IceInternal::BasicStream::read(pair<const Byte*, const Byte*>& v)
{
    Int sz;
    readAndCheckSeqSize(1, sz);
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
IceInternal::BasicStream::write(const vector<bool>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz);
        copy(v.begin(), v.end(), b.begin() + pos);
    }
}

namespace
{

template<size_t boolSize>
struct BasicStreamWriteBoolHelper
{
    static void write(const bool* begin, BasicStream::Container::size_type pos, BasicStream::Container& b, Int sz)
    {
        for(int idx = 0; idx < sz; ++idx)
        {
           b[pos + idx] = static_cast<Ice::Byte>(*(begin + idx));
        }
    }
};

template<>
struct BasicStreamWriteBoolHelper<1>
{
    static void write(const bool* begin, BasicStream::Container::size_type pos, BasicStream::Container& b, Int sz)
    {
        memcpy(&b[pos], begin, sz);
    }
};

}

void
IceInternal::BasicStream::write(const bool* begin, const bool* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz);
        BasicStreamWriteBoolHelper<sizeof(bool)>::write(begin, pos, b, sz);
    }
}

void
IceInternal::BasicStream::read(vector<bool>& v)
{
    Int sz;
    readAndCheckSeqSize(1, sz);
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
struct BasicStreamReadBoolHelper
{
    static bool* read(pair<const bool*, const bool*>& v, Int sz, BasicStream::Container::iterator& i)
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
struct BasicStreamReadBoolHelper<1>
{
    static bool* read(pair<const bool*, const bool*>& v, Int sz, BasicStream::Container::iterator& i)
    {
        v.first = reinterpret_cast<bool*>(i);
        v.second = reinterpret_cast<bool*>(i) + sz;
        return 0;
    }
};

}

bool*
IceInternal::BasicStream::read(pair<const bool*, const bool*>& v)
{
    bool* result = 0;
    Int sz;
    readAndCheckSeqSize(1, sz);
    if(sz > 0)
    {
        result = BasicStreamReadBoolHelper<sizeof(bool)>::read(v, sz, i);
        i += sz;
    }
    else
    {
        v.first = v.second = reinterpret_cast<bool*>(i);
    }
    return result;
}

void
IceInternal::BasicStream::write(Short v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Short));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Short) - 1;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
IceInternal::BasicStream::write(Short v, int end)
{
    if(v < 0 || v >= end)
    {
        throw MarshalException(__FILE__, __LINE__, "enumerator out of range");
    }
    write(v);
}

void
IceInternal::BasicStream::write(const Short* begin, const Short* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Short));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Short) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Short);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Short));
#endif
    }
}

void
IceInternal::BasicStream::read(Short& v)
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
IceInternal::BasicStream::read(Short& v, int end)
{
    read(v);
    if(v < 0 || v >= end)
    {
        throw MarshalException(__FILE__, __LINE__, "enumerator out of range");
    }
}

void
IceInternal::BasicStream::read(vector<Short>& v)
{
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Short)), sz);
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

Short*
IceInternal::BasicStream::read(pair<const Short*, const Short*>& v)
{
    Short* result = 0;
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Short)), sz);
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86)
        v.first = reinterpret_cast<Short*>(i);
        i += sz * static_cast<int>(sizeof(Short));
        v.second = reinterpret_cast<Short*>(i);
#else
        result = new Short[sz];
        v.first = result;
        v.second = result + sz;

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
        v.first = v.second = 0;
    }
    return result;
}

void
IceInternal::BasicStream::read(Int& v, int end)
{
    read(v);
    if(v < 0 || v >= end)
    {
        throw MarshalException(__FILE__, __LINE__, "enumerator out of range");
    }
}

void
IceInternal::BasicStream::write(Int v, int end)
{
    if(v < 0 || v >= end)
    {
        throw MarshalException(__FILE__, __LINE__, "enumerator out of range");
    }
    write(v);
}

void
IceInternal::BasicStream::write(const Int* begin, const Int* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Int));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Int) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Int);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Int));
#endif
    }
}

void
IceInternal::BasicStream::read(vector<Int>& v)
{
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Int)), sz);
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

Int*
IceInternal::BasicStream::read(pair<const Int*, const Int*>& v)
{
    Int* result = 0;
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Int)), sz);
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86)
        v.first = reinterpret_cast<Int*>(i);
        i += sz * static_cast<int>(sizeof(Int));
        v.second = reinterpret_cast<Int*>(i);
#else
        result = new Int[sz];
        v.first = result;
        v.second = result + sz;

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
        v.first = v.second = 0;
    }
    return result;
}

void
IceInternal::BasicStream::write(Long v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Long));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Long) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
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
IceInternal::BasicStream::write(const Long* begin, const Long* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Long));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Long) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Long);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Long));
#endif
    }
}

void
IceInternal::BasicStream::read(Long& v)
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
IceInternal::BasicStream::read(vector<Long>& v)
{
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Long)), sz);
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

Long*
IceInternal::BasicStream::read(pair<const Long*, const Long*>& v)
{
    Long* result = 0;
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Long)), sz);
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86)
        v.first = reinterpret_cast<Long*>(i);
        i += sz * static_cast<int>(sizeof(Long));
        v.second = reinterpret_cast<Long*>(i);
#else
        result = new Long[sz];
        v.first = result;
        v.second = result + sz;

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
        v.first = v.second = 0;
    }
    return result;
}

void
IceInternal::BasicStream::write(Float v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Float));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Float) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
IceInternal::BasicStream::write(const Float* begin, const Float* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Float));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Float) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Float);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Float));
#endif
    }
}

void
IceInternal::BasicStream::read(Float& v)
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
IceInternal::BasicStream::read(vector<Float>& v)
{
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Float)), sz);
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

Float*
IceInternal::BasicStream::read(pair<const Float*, const Float*>& v)
{
    Float* result = 0;
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Float)), sz);
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86)
        v.first = reinterpret_cast<Float*>(i);
        i += sz * static_cast<int>(sizeof(Float));
        v.second = reinterpret_cast<Float*>(i);
#else
        result = new Float[sz];
        v.first = result;
        v.second = result + sz;

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
        v.first = v.second = 0;
    }
    return result;
}

void
IceInternal::BasicStream::write(Double v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Double));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Double) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
#  if defined(__arm__) && defined(__linux)
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
IceInternal::BasicStream::write(const Double* begin, const Double* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Double));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Double) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Double);
        }
#elif defined(__arm__) && defined(__linux)
        const Byte* src = reinterpret_cast<const Byte*>(begin);
        Byte* dest = &(*(b.begin() + pos));
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
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Double));
#endif
    }
}

void
IceInternal::BasicStream::read(Double& v)
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
#  if defined(__arm__) && defined(__linux)
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
IceInternal::BasicStream::read(vector<Double>& v)
{
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Double)), sz);
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
#elif defined(__arm__) && defined(__linux)
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

Double*
IceInternal::BasicStream::read(pair<const Double*, const Double*>& v)
{
    Double* result = 0;
    Int sz;
    readAndCheckSeqSize(static_cast<int>(sizeof(Double)), sz);
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86)
        v.first = reinterpret_cast<Double*>(i);
        i += sz * static_cast<int>(sizeof(Double));
        v.second = reinterpret_cast<Double*>(i);
#else
        result = new Double[sz];
        v.first = result;
        v.second = result + sz;

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
#  elif defined(__arm__) && defined(__linux)
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
        v.first = v.second = 0;
    }
    return result;
}

//
// NOTE: This member function is intentionally omitted in order to
// cause a link error if it is used. This is for efficiency reasons:
// writing a const char * requires a traversal of the string to get
// the string length first, which takes O(n) time, whereas getting the
// string length from a std::string takes constant time.
//
/*
void
IceInternal::BasicStream::write(const char*)
{
}
*/

void
IceInternal::BasicStream::writeConverted(const string& v)
{
    //
    // What is the size of the resulting UTF-8 encoded string?
    // Impossible to tell, so we guess. If we don't guess correctly,
    // we'll have to fix the mistake afterwards
    //
        
    Int guessedSize = static_cast<Int>(v.size());
    writeSize(guessedSize); // writeSize() only writes the size; it does not reserve any buffer space.
        
    size_t firstIndex = b.size();
    StreamUTF8BufferI buffer(*this);
            
    Byte* lastByte = _stringConverter->toUTF8(v.data(), v.data() + v.size(), buffer);
    if(lastByte != b.end())
    {
        b.resize(lastByte - b.begin());
    }
    size_t lastIndex = b.size();
        
    Int actualSize = static_cast<Int>(lastIndex - firstIndex);
        
    //
    // Check against the guess
    //
    if(guessedSize != actualSize)
    {
        if(guessedSize <= 254 && actualSize > 254)
        {
            //
            // Move the UTF-8 sequence 4 bytes further
            // Use memmove instead of memcpy since the source and destination typically overlap.
            //
            resize(b.size() + 4);
            memmove(b.begin() + firstIndex + 4, b.begin() + firstIndex, actualSize);
        }
        else if(guessedSize > 254 && actualSize <= 254)
        {
            //
            // Move the UTF-8 sequence 4 bytes back
            //
            memmove(b.begin() + firstIndex - 4, b.begin() + firstIndex, actualSize);
            resize(b.size() - 4);
        }
    
        if(guessedSize <= 254)
        {
            rewriteSize(actualSize, b.begin() + firstIndex - 1);
        }
        else
        {
            rewriteSize(actualSize, b.begin() + firstIndex - 1 - 4);
        }
    }
}

void
IceInternal::BasicStream::write(const string* begin, const string* end, bool convert)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        for(int i = 0; i < sz; ++i)
        {
            write(begin[i], convert);
        }
    }
}

void
IceInternal::BasicStream::readConverted(string& v, int sz)
{
    _stringConverter->fromUTF8(i, i + sz, v);
}

void
IceInternal::BasicStream::read(vector<string>& v, bool convert)
{
    Int sz;
    readAndCheckSeqSize(1, sz);
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
IceInternal::BasicStream::write(const wstring& v)
{
    if(v.empty())
    {
        writeSize(0);
        return;
    }

    //
    // What is the size of the resulting UTF-8 encoded string?
    // Impossible to tell, so we guess. If we don't guess correctly,
    // we'll have to fix the mistake afterwards
    //
        
    Int guessedSize = static_cast<Int>(v.size());
    writeSize(guessedSize); // writeSize() only writes the size; it does not reserve any buffer space.
        
    size_t firstIndex = b.size();
    StreamUTF8BufferI buffer(*this);
            
    Byte* lastByte = _wstringConverter->toUTF8(v.data(), v.data() + v.size(), buffer);
    if(lastByte != b.end())
    {
        b.resize(lastByte - b.begin());
    }
    size_t lastIndex = b.size();
        
    Int actualSize = static_cast<Int>(lastIndex - firstIndex);
        
    //
    // Check against the guess
    //
    if(guessedSize != actualSize)
    {
        if(guessedSize <= 254 && actualSize > 254)
        {
            //
            // Move the UTF-8 sequence 4 bytes further
            // Use memmove instead of memcpy since the source and destination typically overlap.
            //
            resize(b.size() + 4);
            memmove(b.begin() + firstIndex + 4, b.begin() + firstIndex, actualSize);
        }
        else if(guessedSize > 254 && actualSize <= 254)
        {
            //
            // Move the UTF-8 sequence 4 bytes back
            //
            memmove(b.begin() + firstIndex - 4, b.begin() + firstIndex, actualSize);
            resize(b.size() - 4);
        }
    
        if(guessedSize <= 254)
        {
            rewriteSize(actualSize, b.begin() + firstIndex - 1);
        }
        else
        {
            rewriteSize(actualSize, b.begin() + firstIndex - 1 - 4);
        }
    }
}

void
IceInternal::BasicStream::write(const wstring* begin, const wstring* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        for(int i = 0; i < sz; ++i)
        {
            write(begin[i]);
        }
    }
}

void
IceInternal::BasicStream::read(wstring& v)
{
    Ice::Int sz;
    readSize(sz);
    if(sz > 0)
    {
        if(b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        _wstringConverter->fromUTF8(i, i + sz, v);
        i += sz;
    }
    else
    {
        v.clear();
    }
}

void
IceInternal::BasicStream::read(vector<wstring>& v)
{
    Int sz;
    readAndCheckSeqSize(1, sz);
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

void
IceInternal::BasicStream::write(const ObjectPrx& v)
{
    _instance->proxyFactory()->proxyToStream(v, this);
}

void
IceInternal::BasicStream::read(ObjectPrx& v)
{
    v = _instance->proxyFactory()->streamToProxy(this);
}

void
IceInternal::BasicStream::write(const ObjectPtr& v)
{
    if(!_currentWriteEncaps) // Lazy initialization.
    {
        _currentWriteEncaps = &_preAllocatedWriteEncaps;
        _currentWriteEncaps->start = b.size();
    }

    if(!_currentWriteEncaps->toBeMarshaledMap) // Lazy initialization.
    {
        _currentWriteEncaps->toBeMarshaledMap = new PtrToIndexMap;
        _currentWriteEncaps->marshaledMap = new PtrToIndexMap;
        _currentWriteEncaps->typeIdMap = new TypeIdWriteMap;
    }

    if(v)
    {
        //
        // Look for this instance in the to-be-marshaled map.
        //
        PtrToIndexMap::iterator p = _currentWriteEncaps->toBeMarshaledMap->find(v);
        if(p == _currentWriteEncaps->toBeMarshaledMap->end())
        {
            //
            // Didn't find it, try the marshaled map next.
            //
            PtrToIndexMap::iterator q = _currentWriteEncaps->marshaledMap->find(v);
            if(q == _currentWriteEncaps->marshaledMap->end())
            {
                //
                // We haven't seen this instance previously, create a
                // new index, and insert it into the to-be-marshaled
                // map.
                //
                q = _currentWriteEncaps->toBeMarshaledMap->insert(
                    _currentWriteEncaps->toBeMarshaledMap->end(),
                    pair<const ObjectPtr, Int>(v, ++_currentWriteEncaps->writeIndex));
            }
            p = q;
        }
        //
        // Write the index for the instance.
        //
        write(-(p->second));
    }
    else
    {
        write(0); // Write null pointer.
    }
}

void
IceInternal::BasicStream::read(PatchFunc patchFunc, void* patchAddr)
{
    if(!_currentReadEncaps) // Lazy initialization.
    {
        _currentReadEncaps = &_preAllocatedReadEncaps;
    }

    if(!_currentReadEncaps->patchMap) // Lazy initialization.
    {
        _currentReadEncaps->patchMap = new PatchMap;
        _currentReadEncaps->unmarshaledMap = new IndexToPtrMap;
        _currentReadEncaps->typeIdMap = new TypeIdReadMap;
    }

    ObjectPtr v;

    Int index;
    read(index);

    if(patchAddr)
    {
        if(index == 0)
        {
	    // Calling the patch function for null instances is necessary for correct functioning of Ice for
	    // Python and Ruby.
            patchFunc(patchAddr, v); // Null Ptr.
            return;
        }

        if(index < 0)
        {
            PatchMap::iterator p = _currentReadEncaps->patchMap->find(-index);
            if(p == _currentReadEncaps->patchMap->end())
            {
                //
                // We have no outstanding instances to be patched for this
                // index, so make a new entry in the patch map.
                //
                p = _currentReadEncaps->patchMap->insert(make_pair(-index, PatchList())).first;
            }
            //
            // Append a patch entry for this instance.
            //
            PatchEntry e;
            e.patchFunc = patchFunc;
            e.patchAddr = patchAddr;
            p->second.push_back(e);
            patchPointers(-index, _currentReadEncaps->unmarshaledMap->end(), p);
            return;
        }
    }
    if(index <= 0)
    {
        throw MarshalException(__FILE__, __LINE__, "Invalid class instance index");
    }

    string mostDerivedId;
    readTypeId(mostDerivedId);
    string id = mostDerivedId;
    while(true)
    {
        //
        // If we slice all the way down to Ice::Object, we throw
        // because Ice::Object is abstract.
        //
        if(id == Ice::Object::ice_staticId())
        {
            throw NoObjectFactoryException(__FILE__, __LINE__, "", mostDerivedId);
        }

        //
        // Try to find a factory registered for the specific type.
        //
        ObjectFactoryPtr userFactory = _instance->servantFactoryManager()->find(id);
        if(userFactory)
        {
            v = userFactory->create(id);
        }

        //
        // If that fails, invoke the default factory if one has been
        // registered.
        //
        if(!v)
        {
            userFactory = _instance->servantFactoryManager()->find("");
            if(userFactory)
            {
                v = userFactory->create(id);
            }
        }

        //
        // Last chance: check the table of static factories (i.e.,
        // automatically generated factories for concrete classes).
        //
        if(!v)
        {
            ObjectFactoryPtr of = IceInternal::factoryTable->getObjectFactory(id);
            if(of)
            {
                v = of->create(id);
                assert(v);
            }
        }

        if(!v)
        {
            if(_sliceObjects)
            {
                //
                // Performance sensitive, so we use lazy initialization for tracing.
                //
                if(_traceSlicing == -1)
                {
                    _traceSlicing = _instance->traceLevels()->slicing;
                    _slicingCat = _instance->traceLevels()->slicingCat;
                }
                if(_traceSlicing > 0)
                {
                    traceSlicing("class", id, _slicingCat, _instance->initializationData().logger);
                }
                skipSlice(); // Slice off this derived part -- we don't understand it.
                readTypeId(id); // Read next id for next iteration.
                continue;
            }
            else
            {
                NoObjectFactoryException ex(__FILE__, __LINE__);
                ex.type = id;
                throw ex;
            }
        }

        IndexToPtrMap::const_iterator unmarshaledPos =
                            _currentReadEncaps->unmarshaledMap->insert(make_pair(index, v)).first;

        //
        // Record each object instance so that readPendingObjects can
        // invoke ice_postUnmarshal after all objects have been
        // unmarshaled.
        //
        if(!_objectList)
        {
            _objectList = new ObjectList;
        }
        _objectList->push_back(v);

        v->__read(this, false);
        patchPointers(index, unmarshaledPos, _currentReadEncaps->patchMap->end());
        return;
    }

    //
    // We can't possibly end up here: at the very least, the type ID
    // "::Ice::Object" must be recognized, or client and server were
    // compiled with mismatched Slice definitions.
    //
    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
}

void
IceInternal::BasicStream::write(const UserException& v)
{
    write(v.__usesClasses());
    v.__write(this);
    if(v.__usesClasses())
    {
        writePendingObjects();
    }
}

void
IceInternal::BasicStream::throwException()
{
    bool usesClasses;
    read(usesClasses);

    string id;
    read(id, false);
    const string origId = id;

    for(;;)
    {
        //
        // Look for a factory for this ID.
        //
        UserExceptionFactoryPtr factory = factoryTable->getExceptionFactory(id);
        if(factory)
        {
            //
            // Got factory -- get the factory to instantiate the
            // exception, initialize the exception members, and throw
            // the exception.
            //
            try
            {
                factory->createAndThrow();
            }
            catch(UserException& ex)
            {
                ex.__read(this, false);
                if(usesClasses)
                {
                    readPendingObjects();
                }
                ex.ice_throw();
            }
        }
        else
        {
            //
            // Performance sensitive, so we use lazy initialization
            // for tracing.
            //
            if(_traceSlicing == -1)
            {
                _traceSlicing = _instance->traceLevels()->slicing;
                _slicingCat = _instance->traceLevels()->slicingCat;
            }
            if(_traceSlicing > 0)
            {
                traceSlicing("exception", id, _slicingCat, _instance->initializationData().logger);
            }

            skipSlice(); // Slice off what we don't understand.

            try
            {
                read(id, false); // Read type id for next slice.
            }
            catch(UnmarshalOutOfBoundsException& ex)
            {
                //
                // When read() raises this exception it means we've seen the last slice,
                // so we set the reason member to a more helpful message.
                //
                ex.reason = "unknown exception type `" + origId + "'";
                throw;
            }
        }
    }

    //
    // The only way out of the loop above is to find an exception for
    // which the receiver has a factory. If this does not happen,
    // sender and receiver disagree about the Slice definitions they
    // use. In that case, the receiver will eventually fail to read
    // another type ID and throw a MarshalException.
    //
}

void
IceInternal::BasicStream::writePendingObjects()
{
    if(_currentWriteEncaps && _currentWriteEncaps->toBeMarshaledMap)
    {
        while(_currentWriteEncaps->toBeMarshaledMap->size())
        {
            PtrToIndexMap savedMap = *_currentWriteEncaps->toBeMarshaledMap;
            writeSize(static_cast<Int>(savedMap.size()));
            for(PtrToIndexMap::iterator p = savedMap.begin(); p != savedMap.end(); ++p)
            {
                //
                // Add an instance from the old to-be-marshaled map to
                // the marshaled map and then ask the instance to
                // marshal itself. Any new class instances that are
                // triggered by the classes marshaled are added to
                // toBeMarshaledMap.
                //
                _currentWriteEncaps->marshaledMap->insert(*p);
                writeInstance(p->first, p->second);
            }

            //
            // We have marshaled all the instances for this pass,
            // substract what we have marshaled from the
            // toBeMarshaledMap.
            //
            PtrToIndexMap newMap;
            set_difference(_currentWriteEncaps->toBeMarshaledMap->begin(),
                           _currentWriteEncaps->toBeMarshaledMap->end(),
                           savedMap.begin(), savedMap.end(),
                           insert_iterator<PtrToIndexMap>(newMap, newMap.begin()));
            *_currentWriteEncaps->toBeMarshaledMap = newMap;
        }
    }
    writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
}

void
IceInternal::BasicStream::readPendingObjects()
{
    Int num;
    do
    {
        readSize(num);
        for(Int k = num; k > 0; --k)
        {
            read(0, 0);
        }
    }
    while(num);

    if(_currentReadEncaps && _currentReadEncaps->patchMap && _currentReadEncaps->patchMap->size() != 0)
    {
        //
        // If any entries remain in the patch map, the sender has sent an index for an object, but failed
        // to supply the object.
        //
        throw MarshalException(__FILE__, __LINE__, "Index for class received, but no instance");
    }

    //
    // Iterate over the object list and invoke ice_postUnmarshal on
    // each object.  We must do this after all objects have been
    // unmarshaled in order to ensure that any object data members
    // have been properly patched.
    //
    if(_objectList)
    {
        for(ObjectList::iterator p = _objectList->begin(); p != _objectList->end(); ++p)
        {
            try
            {
                (*p)->ice_postUnmarshal();
            }
            catch(const std::exception& ex)
            {
                Ice::Warning out(_instance->initializationData().logger);
                out << "std::exception raised by ice_postUnmarshal:\n" << ex;
            }
            catch(...)
            {
                Ice::Warning out(_instance->initializationData().logger);
                out << "unknown exception raised by ice_postUnmarshal";
            }
        }
    }
}

void
IceInternal::BasicStream::sliceObjects(bool doSlice)
{
    _sliceObjects = doSlice;
}

void
IceInternal::BasicStream::throwUnmarshalOutOfBoundsException(const char* file, int line)
{
    throw UnmarshalOutOfBoundsException(file, line);
}

void
IceInternal::BasicStream::throwUnsupportedEncodingException(const char* file, int line, Byte eMajor, Byte eMinor)
{   
    UnsupportedEncodingException ex(file, line);
    ex.badMajor = static_cast<unsigned char>(eMajor);
    ex.badMinor = static_cast<unsigned char>(eMinor);
    ex.major = static_cast<unsigned char>(encodingMajor);
    ex.minor = static_cast<unsigned char>(encodingMinor);
    throw ex;
}

void
IceInternal::BasicStream::throwEncapsulationException(const char* file, int line)
{
    throw EncapsulationException(file, line);
}

void
IceInternal::BasicStream::writeInstance(const ObjectPtr& v, Int index)
{
    write(index);
    try
    {
        v->ice_preMarshal();
    }
    catch(const std::exception& ex)
    {
        Ice::Warning out(_instance->initializationData().logger);
        out << "std::exception raised by ice_preMarshal:\n" << ex;
    }
    catch(...)
    {
        Ice::Warning out(_instance->initializationData().logger);
        out << "unknown exception raised by ice_preMarshal";
    }
    v->__write(this);
}

void
IceInternal::BasicStream::patchPointers(Int index, IndexToPtrMap::const_iterator unmarshaledPos,
                                        PatchMap::iterator patchPos)
{
    //
    // Called whenever we have unmarshaled a new instance. The index
    // is the index of the instance.  UnmarshaledPos denotes the
    // instance just unmarshaled and patchPos denotes the patch map
    // entry for the index just unmarshaled. (Exactly one of these two
    // iterators must be end().)  Patch any pointers in the patch map
    // with the new address.
    //
    assert(   (unmarshaledPos != _currentReadEncaps->unmarshaledMap->end()
               && patchPos == _currentReadEncaps->patchMap->end())
           || (unmarshaledPos == _currentReadEncaps->unmarshaledMap->end()
               && patchPos != _currentReadEncaps->patchMap->end())
          );

    if(unmarshaledPos != _currentReadEncaps->unmarshaledMap->end())
    {
        //
        // We have just unmarshaled an instance -- check if something
        // needs patching for that instance.
        //
        patchPos = _currentReadEncaps->patchMap->find(index);
        if(patchPos == _currentReadEncaps->patchMap->end())
        {
            return; // We don't have anything to patch for the instance just unmarshaled.
        }
    }
    else
    {
        //
        // We have just unmarshaled an index -- check if we have
        // unmarshaled the instance for that index yet.
        //
        unmarshaledPos = _currentReadEncaps->unmarshaledMap->find(index);
        if(unmarshaledPos == _currentReadEncaps->unmarshaledMap->end())
        {
            return; // We haven't unmarshaled the instance yet.
        }
    }
    assert(patchPos->second.size() > 0);

    ObjectPtr v = unmarshaledPos->second;
    assert(v);

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
    _currentReadEncaps->patchMap->erase(patchPos);
}

