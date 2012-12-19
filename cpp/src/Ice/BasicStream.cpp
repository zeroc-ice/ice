// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/BasicStream.h>
#include <Ice/DefaultsAndOverrides.h>
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
#include <Ice/SlicedData.h>
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

const Byte BasicStream::FLAG_HAS_TYPE_ID_STRING    = (1<<0);
const Byte BasicStream::FLAG_HAS_TYPE_ID_INDEX     = (1<<1);
const Byte BasicStream::FLAG_HAS_TYPE_ID_COMPACT   = (1<<0) | (1<<1);
const Byte BasicStream::FLAG_HAS_OPTIONAL_MEMBERS  = (1<<2);
const Byte BasicStream::FLAG_HAS_INDIRECTION_TABLE = (1<<3);
const Byte BasicStream::FLAG_HAS_SLICE_SIZE        = (1<<4);
const Byte BasicStream::FLAG_IS_LAST_SLICE         = (1<<5);

IceInternal::BasicStream::BasicStream(Instance* instance, const EncodingVersion& encoding, bool unlimited) :
    IceInternal::Buffer(instance->messageSizeMax()),
    _instance(instance),
    _closure(0),
    _encoding(encoding),
    _currentReadEncaps(0),
    _currentWriteEncaps(0),
    _sliceObjects(true),
    _messageSizeMax(_instance->messageSizeMax()), // Cached for efficiency.
    _unlimited(unlimited),
    _stringConverter(instance->initializationData().stringConverter),
    _wstringConverter(instance->initializationData().wstringConverter),
    _startSeq(-1),
    _sizePos(-1)
{
    //
    // Initialize the encoding members of our pre-allocated encapsulations, in case
    // this stream is used without an explicit encapsulation.
    //
    _preAllocatedReadEncaps.encoding = encoding;
    _preAllocatedWriteEncaps.encoding = encoding;
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

    _sizePos = -1;

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
    // Swap is never called for BasicStreams that have encapsulations being read/write. However,
    // encapsulations might still be set in case marhsalling or un-marhsalling failed. We just
    // reset the encapsulations if there are still some set.
    //
    resetEncaps();
    other.resetEncaps();

    std::swap(_unlimited, other._unlimited);
    std::swap(_startSeq, other._startSeq);
    std::swap(_minSeqSize, other._minSeqSize);
    std::swap(_sizePos, other._sizePos);
}

void
IceInternal::BasicStream::resetEncaps()
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

    _preAllocatedReadEncaps.reset();
    _preAllocatedWriteEncaps.reset();
}

void
IceInternal::BasicStream::startWriteEncaps()
{
    //
    // If no encoding version is specified, use the current write
    // encapsulation encoding version if there's a current write
    // encapsulation, otherwise, use the stream encoding version.
    //

    if(_currentWriteEncaps)
    {
        startWriteEncaps(_currentWriteEncaps->encoding, _currentWriteEncaps->format);
    }
    else
    {
        startWriteEncaps(_encoding, Ice::DefaultFormat);
    }
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

EncodingVersion
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
    EncodingVersion encoding;
    read(encoding.major);
    read(encoding.minor);
    i += sz - sizeof(Int) - 2;
    return encoding;
}

void
IceInternal::BasicStream::readPendingObjects()
{
    if(_currentReadEncaps && _currentReadEncaps->decoder)
    {
        _currentReadEncaps->decoder->readPendingObjects();
        delete _currentReadEncaps->decoder;
        _currentReadEncaps->decoder = 0;
    }
    else if(getReadEncoding() == Ice::Encoding_1_0)
    {
        //
        // If using the 1.0 encoding and no objects were read, we
        // still read an empty sequence of pending objects if
        // requested (i.e.: if this is called).
        //
        // This is required by the 1.0 encoding, even if no objects
        // are written we do marshal an empty sequence if marshaled
        // data types use classes.
        //
        skipSize();
    }
}

void
IceInternal::BasicStream::writePendingObjects()
{
    if(_currentWriteEncaps && _currentWriteEncaps->encoder)
    {
        _currentWriteEncaps->encoder->writePendingObjects();
        delete _currentWriteEncaps->encoder;
        _currentWriteEncaps->encoder = 0;
    }
    else if(getWriteEncoding() == Ice::Encoding_1_0)
    {
        //
        // If using the 1.0 encoding and no objects were written, we
        // still write an empty sequence for pending objects if
        // requested (i.e.: if this is called).
        // 
        // This is required by the 1.0 encoding, even if no objects
        // are written we do marshal an empty sequence if marshaled
        // data types use classes.
        //
        writeSize(0);
    }
}


Int
IceInternal::BasicStream::readAndCheckSeqSize(int minSize)
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
IceInternal::BasicStream::read(std::vector<Ice::Byte>& v)
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
IceInternal::BasicStream::read(pair<const Byte*, const Byte*>& v)
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
           b[pos + idx] = static_cast<Byte>(*(begin + idx));
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
    Int sz = readAndCheckSeqSize(1);
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
IceInternal::BasicStream::read(vector<Short>& v)
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

Short*
IceInternal::BasicStream::read(pair<const Short*, const Short*>& v)
{
    Short* result = 0;
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Short)));
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

Int*
IceInternal::BasicStream::read(pair<const Int*, const Int*>& v)
{
    Int* result = 0;
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Int)));
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

Long*
IceInternal::BasicStream::read(pair<const Long*, const Long*>& v)
{
    Long* result = 0;
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Long)));
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

Float*
IceInternal::BasicStream::read(pair<const Float*, const Float*>& v)
{
    Float* result = 0;
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Float)));
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
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Double)));
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
    Int sz = readSize();
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

Int
IceInternal::BasicStream::readEnum(Int maxValue)
{
    if(getReadEncoding() == Encoding_1_0)
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
IceInternal::BasicStream::writeEnum(Int v, Int maxValue)
{
    if(getWriteEncoding() == Encoding_1_0)
    {
        if(maxValue < 127)
        {
            write(static_cast<Byte>(v));
        }
        else if(maxValue < 32767)
        {
            write(static_cast<Short>(v));
        }
        else 
        {
            write(v);
        }
    }
    else
    {
        writeSize(v);
    }
}

void
IceInternal::BasicStream::writeException(const Ice::UserException& e)
{
    initWriteEncaps();
    _currentWriteEncaps->encoder->write(e);

    //
    // Reset the encoder, the writing of the exception wrote
    // pending objects if any.
    //
    delete _currentWriteEncaps->encoder;
    _currentWriteEncaps->encoder = 0;
}

void
IceInternal::BasicStream::throwException(const UserExceptionFactoryPtr& factory)
{
    initReadEncaps();
    try
    {
        _currentReadEncaps->decoder->throwException(factory);
    }
    catch(const Ice::UserException&)
    {
        //
        // Reset the decoder, the reading of the exception wrote
        // pending objects if any.
        //
        delete _currentReadEncaps->decoder;
        _currentReadEncaps->decoder = 0;
        throw;
    }
}

void
IceInternal::BasicStream::sliceObjects(bool doSlice)
{
    _sliceObjects = doSlice;
}

bool
IceInternal::BasicStream::readOptImpl(Int readTag, OptionalFormat expectedFormat)
{
    if(getReadEncoding() == Encoding_1_0)
    {
        return false; // Optional members aren't supported with the 1.0 encoding.
    }

    Int tag = 0;
    OptionalFormat format;
    do
    {
        if(i >= b.begin() + _currentReadEncaps->start + _currentReadEncaps->sz)
        {
            return false; // End of encapsulation also indicates end of optionals.
        }

        Byte v;
        read(v);
        format = static_cast<OptionalFormat>(v & 0x07); // First 3 bits.
        tag = static_cast<Int>(v >> 3);
        if(tag == 31)
        {
            tag = readSize();
        }
    }
    while(format != OptionalFormatEndMarker && tag < readTag && skipOpt(format)); // Skip optional data members
    
    if(format == OptionalFormatEndMarker || tag > readTag)
    {
        //
        // Rewind the stream to correctly read the next optional data
        // member tag & format next time.
        //
        i -= tag < 31 ? 1 : (tag < 255 ? 2 : 6);
        return false; // No optional data members with the requested tag.
    } 
    
    assert(readTag == tag);
    if(format != expectedFormat)
    {
        ostringstream os;
        os << "invalid optional data member `" << tag << "': unexpected format";
        throw MarshalException(__FILE__, __LINE__, os.str());
    }

    //
    // We have an optional data member with the requested tag and
    // format.
    // 
    return true;
}

bool
IceInternal::BasicStream::writeOptImpl(Int tag, OptionalFormat type)
{
    if(getWriteEncoding() == Encoding_1_0)
    {
        return false; // Optional members aren't supported with the 1.0 encoding.
    }

    Byte v = static_cast<Byte>(type);
    if(tag < 31)
    {
        v |= tag << 3;
        write(v);
    }
    else
    {
        v |= 0x0F8; // tag = 31
        write(v);
        writeSize(tag);
    }
    return true;
}

bool
IceInternal::BasicStream::skipOpt(OptionalFormat type)
{
    int sz;
    switch(type)
    {
    case Ice::OptionalFormatF1:
    {
        sz = 1;
        break;
    }
    case Ice::OptionalFormatF2:
    {
        sz = 2;
        break;
    }
    case Ice::OptionalFormatF4:
    {
        sz = 4;
        break;
    }
    case Ice::OptionalFormatF8:
    {
        sz = 8;
        break;
    }
    case Ice::OptionalFormatSize:
    {
        skipSize();
        return true;
    }
    case Ice::OptionalFormatVSize:
    {
        sz = readSize();
        break;
    }
    case Ice::OptionalFormatFSize:
    {
        read(sz);
        break;
    }
    default:
    {
        return false;
    }
    }
    skip(sz);
    return true;
}

bool
BasicStream::skipOpts()
{
    //
    // Skip remaining un-read optional members.
    // 
    OptionalFormat format;
    do
    {
        if(i >= b.begin() + _currentReadEncaps->start + _currentReadEncaps->sz)
        {
            return false; // End of encapsulation also indicates end of optionals.
        }

        Byte v;
        read(v);
        format = static_cast<OptionalFormat>(v & 0x07); // Read first 3 bits.
        if(static_cast<Int>(v >> 3) == 31)
        {
            skipSize();
        }
    }
    while(skipOpt(format));
    assert(format == OptionalFormatEndMarker);
    return true;
}

void
IceInternal::BasicStream::throwUnmarshalOutOfBoundsException(const char* file, int line)
{
    throw UnmarshalOutOfBoundsException(file, line);
}

void
IceInternal::BasicStream::throwEncapsulationException(const char* file, int line)
{
    throw EncapsulationException(file, line);
}

void
IceInternal::BasicStream::initReadEncaps()
{
    if(!_currentReadEncaps) // Lazy initialization.
    {
        _currentReadEncaps = &_preAllocatedReadEncaps;
        _currentReadEncaps->sz = static_cast<Ice::Int>(b.size());
    }

    if(!_currentReadEncaps->decoder) // Lazy initialization.
    {
        _currentReadEncaps->decoder = new EncapsDecoder(this, _currentReadEncaps, _sliceObjects);
    }
}

void
IceInternal::BasicStream::initWriteEncaps()
{
    if(!_currentWriteEncaps) // Lazy initialization.
    {
        _currentWriteEncaps = &_preAllocatedWriteEncaps;
        _currentWriteEncaps->start = b.size();
    }

    if(_currentWriteEncaps->format == Ice::DefaultFormat)
    {
        _currentWriteEncaps->format = _instance->defaultsAndOverrides()->defaultFormat;
    }

    if(!_currentWriteEncaps->encoder) // Lazy initialization.
    {
        _currentWriteEncaps->encoder = new EncapsEncoder(this, _currentWriteEncaps);
    }
}

void
IceInternal::BasicStream::EncapsEncoder::write(const ObjectPtr& v)
{
    if(v)
    {
        //
        // Register the object.
        //
        Int index = registerObject(v);

        if(_encaps->encoding == Encoding_1_0)
        {
            //
            // Object references are encoded as a negative integer in 1.0.
            //
            _stream->write(-index);
        }
        else if(_sliceType != NoSlice && _encaps->format == SlicedFormat)
        {
            //
            // An object reference that appears inside a slice of an
            // object or exception encoded as a positive non-zero
            // index into a per-slice indirection table.
            //
            // We use _indirectionMap to keep track of the object
            // references in the current slice; it maps the object
            // reference to the position in the indirection list. Note
            // that the position is offset by one (e.g., the first
            // position = 1).
            //
            IndirectionMap::iterator p = _indirectionMap.find(index);
            if(p == _indirectionMap.end())
            {
                _indirectionTable.push_back(index);
                const Int sz = static_cast<Int>(_indirectionTable.size()); // Position + 1
                _indirectionMap.insert(make_pair(index, sz));
                _stream->writeSize(sz);
            }
            else
            {
                _stream->writeSize(p->second);
            }
        }
        else
        {
            _stream->writeSize(index);
        }
    }
    else
    {
        //
        // Write nil reference.
        //
        if(_encaps->encoding == Encoding_1_0)
        {
            _stream->write(0);
        }
        else
        {
            _stream->writeSize(0);
        }
    }
}

void
IceInternal::BasicStream::EncapsEncoder::write(const UserException& v)
{
    //
    // User exception with the 1.0 encoding start with a boolean
    // flag that indicates whether or not the exception uses
    // classes. 
    //
    // This allows reading the pending objects even if some part of
    // the exception was sliced. With encoding > 1.0, we don't need
    // this, each slice indirect patch table indicates the presence of
    // objects.
    //
    bool usesClasses;
    if(_encaps->encoding == Encoding_1_0)
    {
        usesClasses = v.__usesClasses();
        _stream->write(usesClasses);
    }
    else
    {
        usesClasses = true; // Always call writePendingObjects
    }
    v.__write(_stream);
    if(usesClasses)
    {
        writePendingObjects();
    }
}

void
IceInternal::BasicStream::EncapsEncoder::startObject(const SlicedDataPtr& data)
{
    _sliceType = ObjectSlice;
    _firstSlice = true;
    if(data)
    {
        writeSlicedData(data);
    }
}

void
IceInternal::BasicStream::EncapsEncoder::endObject()
{
    if(_encaps->encoding == Encoding_1_0)
    {
        //
        // Write the Object slice.
        //
        startSlice(Object::ice_staticId(), -1, true);
        _stream->writeSize(0); // For compatibility with the old AFM.
        endSlice();
    }
    _sliceType = NoSlice;
}

void
IceInternal::BasicStream::EncapsEncoder::startException(const SlicedDataPtr& data)
{
    _sliceType = ExceptionSlice;
    _firstSlice = true;
    if(data)
    {
        writeSlicedData(data);
    }
}

void 
IceInternal::BasicStream::EncapsEncoder::endException()
{
    _sliceType = NoSlice;
}

void
IceInternal::BasicStream::EncapsEncoder::startSlice(const string& typeId, int compactId, bool last)
{
    assert(_indirectionTable.empty() && _indirectionMap.empty());
    _sliceFlags = 0;
    _sliceFlagsPos = _stream->b.size();

    //
    // Encode the slice size for the old encoding and if using the
    // sliced format.
    //
    if(_encaps->encoding == Encoding_1_0 || _encaps->format == SlicedFormat)
    {
        _sliceFlags |= FLAG_HAS_SLICE_SIZE;
    }

    //
    // This is the last slice.
    //
    if(last)
    {
        _sliceFlags |= FLAG_IS_LAST_SLICE;
    }

    //
    // For object slices, encode the flag and the type ID either as a
    // string or index. For exception slices, don't encode slice flags
    // for the old encoding and always encode the type ID a string.
    //
    if(_sliceType == ObjectSlice)
    {
        _stream->write(Byte(0)); // Placeholder for the slice flags

        //
        // Encode the type ID (only in the first slice for the compact
        // encoding).
        // 
        if(_encaps->format == SlicedFormat || _encaps->encoding == Encoding_1_0 || _firstSlice)
        {
            if(_encaps->encoding != Encoding_1_0 && compactId >= 0)
            {
                _sliceFlags |= FLAG_HAS_TYPE_ID_COMPACT;
                _stream->writeSize(compactId);
            }
            else
            {
                //
                // If the type ID has already been seen, write the index
                // of the type ID, otherwise allocate a new type ID and
                // write the string.
                //
                TypeIdWriteMap::const_iterator p = _typeIdMap.find(typeId);
                if(p != _typeIdMap.end())
                {
                    _sliceFlags |= FLAG_HAS_TYPE_ID_INDEX;
                    _stream->writeSize(p->second);
                }
                else
                {
                    _sliceFlags |= FLAG_HAS_TYPE_ID_STRING;
                    _typeIdMap.insert(make_pair(typeId, ++_typeIdIndex));
                    _stream->write(typeId, false);
                }
            }
        }
    }
    else
    {
        if(_encaps->encoding != Encoding_1_0)
        {
            _stream->write(Byte(0)); // Placeholder for the slice flags
        }
        _stream->write(typeId, false);
    }

    if(_sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        _stream->write(Int(0)); // Placeholder for the slice length.
    }

    _writeSlice = _stream->b.size();
    _firstSlice = false;
}

void
IceInternal::BasicStream::EncapsEncoder::endSlice()
{
    //
    // Write the optional member end marker if some optional members
    // were encoded. Note that the optional members are encoded before
    // the indirection table and are included in the slice size.
    //
    if(_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        assert(_encaps->encoding != Encoding_1_0);
        _stream->write(static_cast<Byte>(OptionalFormatEndMarker));
    }

    //
    // Write the slice length if necessary.
    //
    if(_sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        Int sz = static_cast<Int>(_stream->b.size() - _writeSlice + sizeof(Int));
        Byte* dest = &(*(_stream->b.begin() + _writeSlice - sizeof(Int)));
        _stream->write(sz, dest);
    }

    //
    // Only write the indirection table if it contains entries.
    //
    if(!_indirectionTable.empty())
    {
        assert(_encaps->encoding != Encoding_1_0);
        assert(_encaps->format == SlicedFormat);
        _sliceFlags |= FLAG_HAS_INDIRECTION_TABLE;

        //
        // Write the indirection table as a sequence<size> to conserve space.
        //
        _stream->writeSizeSeq(_indirectionTable);

        _indirectionTable.clear();
        _indirectionMap.clear();
    }

    //
    // Finally, update the slice flags (or the object slice has index
    // type ID boolean for the 1.0 encoding)
    //
    if(_encaps->encoding == Encoding_1_0)
    {
        if(_sliceType == ObjectSlice) // No flags for 1.0 exception slices.
        {
            Byte* dest = &(*(_stream->b.begin() + _sliceFlagsPos));
            *dest = static_cast<Byte>(_sliceFlags & FLAG_HAS_TYPE_ID_INDEX ? true : false);
        }
    }
    else
    {
        Byte* dest = &(*(_stream->b.begin() + _sliceFlagsPos));
        *dest = _sliceFlags;
    }
}

void
IceInternal::BasicStream::EncapsEncoder::writePendingObjects()
{
    //
    // With the 1.0 encoding, write pending objects if the marshalled
    // data uses classes. Otherwise with encoding > 1.0, only write
    // pending objects if some non-nil references were written.
    //
    if(_encaps->encoding != Encoding_1_0)
    {
        if(_toBeMarshaledMap.empty())
        {
            return;
        }
        else 
        {
            //
            // Write end marker for encapsulation optionals before encoding 
            // the pending objects.
            //
            _stream->write(static_cast<Byte>(OptionalFormatEndMarker));
        }
    }

    while(!_toBeMarshaledMap.empty())
    {
        //
        // Consider the to be marshalled objects as marshalled now,
        // this is necessary to avoid adding again the "to be
        // marshalled objects" into _toBeMarshaledMap while writing
        // objects.
        //
        _marshaledMap.insert(_toBeMarshaledMap.begin(), _toBeMarshaledMap.end());

        PtrToIndexMap savedMap;
        savedMap.swap(_toBeMarshaledMap);
        _stream->writeSize(static_cast<Int>(savedMap.size()));
        for(PtrToIndexMap::iterator p = savedMap.begin(); p != savedMap.end(); ++p)
        {
            //
            // Ask the instance to marshal itself. Any new class
            // instances that are triggered by the classes marshaled
            // are added to toBeMarshaledMap.
            //
            if(_encaps->encoding == Encoding_1_0)
            {
                _stream->write(p->second);
            }
            else
            {
                _stream->writeSize(p->second);
            }

            try
            {
                p->first->ice_preMarshal();
            }
            catch(const std::exception& ex)
            {
                Warning out(_stream->instance()->initializationData().logger);
                out << "std::exception raised by ice_preMarshal:\n" << ex;
            }
            catch(...)
            {
                Warning out(_stream->instance()->initializationData().logger);
                out << "unknown exception raised by ice_preMarshal";
            }

            p->first->__write(_stream);
        }
    }
    _stream->writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
}

void
IceInternal::BasicStream::EncapsEncoder::writeSlicedData(const SlicedDataPtr& slicedData)
{
    assert(slicedData);

    //
    // We only remarshal preserved slices if the target encoding is > 1.0 and we are
    // using the sliced format. Otherwise, we ignore the preserved slices, which
    // essentially "slices" the object into the most-derived type known by the sender.
    //
    if(_encaps->encoding == Encoding_1_0 || _encaps->format != SlicedFormat)
    {
        return;
    }

    for(SliceInfoSeq::const_iterator p = slicedData->slices.begin(); p != slicedData->slices.end(); ++p)
    {
        startSlice((*p)->typeId, (*p)->compactId, (*p)->isLastSlice);
 
        //
        // Write the bytes associated with this slice.
        //
        _stream->writeBlob((*p)->bytes);
        
        if((*p)->hasOptionalMembers)
        {
            _sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
        }

        //
        // Assemble and write the indirection table. The table must have the same order
        // as the list of objects.
        //
        for(vector<ObjectPtr>::const_iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            _indirectionTable.push_back(registerObject(*q));
        }

        endSlice();
    }
}

Int 
IceInternal::BasicStream::EncapsEncoder::registerObject(const ObjectPtr& v)
{
    assert(v);

    //
    // Look for this instance in the to-be-marshaled map.
    //
    PtrToIndexMap::const_iterator p = _toBeMarshaledMap.find(v);
    if(p != _toBeMarshaledMap.end())
    {
        return p->second;
    }

    //
    // Didn't find it, try the marshaled map next.
    //
    PtrToIndexMap::const_iterator q = _marshaledMap.find(v);
    if(q != _marshaledMap.end())
    {
        return q->second;
    }

    //
    // We haven't seen this instance previously, create a new
    // index, and insert it into the to-be-marshaled map.
    //
    _toBeMarshaledMap.insert(make_pair(v, ++_objectIdIndex));
    return _objectIdIndex;
}

void
IceInternal::BasicStream::EncapsDecoder::read(PatchFunc patchFunc, void* patchAddr)
{
    ObjectPtr v;

    Int index;
    if(_encaps->encoding == Encoding_1_0)
    {
        //
        // Object references are encoded as a negative integer in 1.0.
        //
        _stream->read(index);
        if(index > 0)
        {
            throw MarshalException(__FILE__, __LINE__, "invalid object id");
        }
        index = -index;
    }
    else
    {
        //
        // Later versions use a size.
        //
        index = _stream->readSize();
        if(index < 0)
        {
            throw MarshalException(__FILE__, __LINE__, "invalid object id");
        }
    }

    if(index == 0)
    {
        //
        // Calling the patch function for null instances is necessary for correct functioning of Ice for
        // Python and Ruby.
        //
        ObjectPtr nil;
        patchFunc(patchAddr, nil);
    }
    else if(_sliceType != NoSlice && _sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
    {
        //
        // Maintain a list of indirect references. Note that the indirect index
        // starts at 1, so we decrement it by one to derive an index into
        // the indirection table that we'll read at the end of the slice.
        //
        IndirectPatchEntry e;
        e.index = index - 1;
        e.patchFunc = patchFunc;
        e.patchAddr = patchAddr;
        _indirectPatchList.push_back(e);
    }
    else
    {
        addPatchEntry(index, patchFunc, patchAddr);
    }
}

void
IceInternal::BasicStream::EncapsDecoder::throwException(const UserExceptionFactoryPtr& factory)
{
    assert(_sliceType == NoSlice);

    //
    // User exception with the 1.0 encoding start with a boolean flag
    // that indicates whether or not the exception has classes.
    //
    // This allows reading the pending objects even if some part of
    // the exception was sliced. With encoding > 1.0, we don't need
    // this, each slice indirect patch table indicates the presence of
    // objects.
    //
    bool usesClasses;
    if(_encaps->encoding == Encoding_1_0)
    {
        _stream->read(usesClasses);
    }
    else
    {
        usesClasses = true; // Always call readPendingObjects.
    }

    _sliceType = ExceptionSlice;
    _skipFirstSlice = false;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _typeId;
    UserExceptionFactoryPtr exceptionFactory = factory;
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
                exceptionFactory->createAndThrow(_typeId);
            }
            catch(UserException& ex)
            {
                ex.__read(_stream);
                if(usesClasses)
                {
                    readPendingObjects();
                }
                throw;
        
                // Never reached.
            }
        }

        //
        // Performance sensitive, so we use lazy initialization for
        // tracing.
        //
        if(_traceSlicing == -1)
        {
            _traceSlicing = _stream->instance()->traceLevels()->slicing;
            _slicingCat = _stream->instance()->traceLevels()->slicingCat;
        }
        if(_traceSlicing > 0)
        {
            traceSlicing("exception", _typeId, _slicingCat, _stream->instance()->initializationData().logger);
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice(); 

        //
        // If this is the last slice, raise an exception and stop un-marshalling.
        //
        if(_sliceFlags & FLAG_IS_LAST_SLICE)
        {
            if(mostDerivedId.length() > 2 && mostDerivedId[0] == ':' && mostDerivedId[1] == ':')
            {
                throw UnknownUserException(__FILE__, __LINE__, mostDerivedId.substr(2));
            }
            else
            {
                throw UnknownUserException(__FILE__, __LINE__, mostDerivedId);
            }
        }

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
            if(_encaps->encoding == Encoding_1_0)
            {
                // Set the reason member to a more helpful message.
                ex.reason = "unknown exception type `" + mostDerivedId + "'";
            }
            throw;
        }
    }
}

void 
IceInternal::BasicStream::EncapsDecoder::startObject()
{
    assert(_sliceType == ObjectSlice);
    _skipFirstSlice = true;
}

SlicedDataPtr
IceInternal::BasicStream::EncapsDecoder::endObject(bool preserve)
{
    if(_encaps->encoding == Encoding_1_0)
    {
        //
        // Read the Ice::Object slice.
        //
        startSlice();

        //
        // For compatibility with the old AFM.
        //
        Int sz = _stream->readSize();
        if(sz != 0)
        {
            throw MarshalException(__FILE__, __LINE__, "invalid Object slice");
        }

        endSlice();
    }

    _sliceType = NoSlice;
    SlicedDataPtr slicedData;
    if(preserve)
    {
        slicedData = readSlicedData();
    }
    _slices.clear();
    _indirectionTables.clear();
    return slicedData;
}

void 
IceInternal::BasicStream::EncapsDecoder::startException()
{
    assert(_sliceType == ExceptionSlice);
    _skipFirstSlice = true;
}

SlicedDataPtr 
IceInternal::BasicStream::EncapsDecoder::endException(bool preserve)
{
    _sliceType = NoSlice;
    SlicedDataPtr slicedData;
    if(preserve)
    {
        slicedData = readSlicedData();
    }
    _slices.clear();
    _indirectionTables.clear();
    return slicedData;
}

const string&
IceInternal::BasicStream::EncapsDecoder::startSlice()
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
    // Read the slice flags. For the 1.0 encoding there's no flag but
    // just a boolean for object slices. The boolean indicates whether
    // or not the type ID is encoded as a string or as an index.
    //
    if(_encaps->encoding == Encoding_1_0)
    {
        _sliceFlags = FLAG_HAS_SLICE_SIZE;
        if(_sliceType == ObjectSlice) // For exceptions, the type ID is always encoded as a string
        {
            bool isIndex;
            _stream->read(isIndex);
            _sliceFlags |= isIndex ? FLAG_HAS_TYPE_ID_INDEX : FLAG_HAS_TYPE_ID_STRING;
        }
    }
    else
    {
        _stream->read(_sliceFlags);
    }

    //
    // Read the type ID, for object slices the type ID is encoded as a
    // string or as an index, for exceptions it's always encoded as a
    // string.
    //
    if(_sliceType == ObjectSlice)
    {
        if((_sliceFlags & FLAG_HAS_TYPE_ID_COMPACT) == FLAG_HAS_TYPE_ID_COMPACT) // Must be checked first!
        {
            _typeId.clear();
            _compactId = _stream->readSize();
        }
        else if(_sliceFlags & FLAG_HAS_TYPE_ID_INDEX)
        {
            Int index = _stream->readSize();
            TypeIdReadMap::const_iterator k = _typeIdMap.find(index);
            if(k == _typeIdMap.end())
            {
                throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            _typeId = k->second;
            _compactId = -1;
        }
        else if(_sliceFlags & FLAG_HAS_TYPE_ID_STRING)
        {
            _stream->read(_typeId, false);
            _typeIdMap.insert(make_pair(++_typeIdIndex, _typeId));
            _compactId = -1;
        }
        else
        {
            // Only the most derived slice encodes the type ID for the
            // compact format.
            _typeId.clear();
            _compactId = -1;
        }
    } 
    else
    {
        _stream->read(_typeId, false);
    }

    //
    // Read the slice size if necessary.
    //
    if(_sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        _stream->read(_sliceSize);
        if(_sliceSize < 4)
        {
            throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
    }
    else
    {
        _sliceSize = 0;
    }

    //
    // Reset the indirect patch list for this new slice.
    //
    _indirectPatchList.clear();
    return _typeId;
}

void
IceInternal::BasicStream::EncapsDecoder::endSlice()
{
    if(_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        _stream->skipOpts();
    }

    //
    // Read the indirection table if one is present and transform the
    // indirect patch list into patch entries with direct references.
    //
    if(_sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
    {
        //
        // The table is written as a sequence<size> to conserve space.
        //
        IndexList indirectionTable;
        _stream->readSizeSeq(indirectionTable);

        //
        // Sanity checks. If there are optional members, it's possible
        // that not all object references were read if they are from
        // unknown optional data members.
        //
        if(indirectionTable.empty() && !_indirectPatchList.empty())
        {
            throw MarshalException(__FILE__, __LINE__, "empty indirection table");
        }
        else if(!indirectionTable.empty() && _indirectPatchList.empty() && !(_sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS))
        {
            throw MarshalException(__FILE__, __LINE__, "no references to indirection table");
        }

        //
        // Convert indirect references into direct references.
        //
        for(IndirectPatchList::iterator p = _indirectPatchList.begin(); p != _indirectPatchList.end(); ++p)
        {
            assert(p->index >= 0);
            if(p->index >= static_cast<Int>(indirectionTable.size()))
            {
                throw MarshalException(__FILE__, __LINE__, "indirection out of range");
            }
            const Int id = indirectionTable[p->index];
            if(id <= 0)
            {
                //
                // Entries in the table must be positive, just like a regular object reference.
                //
                throw MarshalException(__FILE__, __LINE__, "invalid id in object indirection table");
            }
            addPatchEntry(id, p->patchFunc, p->patchAddr);
        }
    }
}            

void
IceInternal::BasicStream::EncapsDecoder::readPendingObjects()
{
    //
    // With the 1.0 encoding, we read pending objects if the marshaled
    // data uses classes. Otherwise, only read pending objects if some
    // non-nil references were read.
    //
    if(_encaps->encoding != Encoding_1_0)
    {        
        if(_patchMap.empty())
        {
            return;
        }
        else
        {
            //
            // Read unread encapsulation optionals before reading the
            // pending objects.
            //
            _stream->skipOpts();
        }
    }

    Int num;
    ObjectList objectList;
    do
    {
        num = _stream->readSize();
        for(Int k = num; k > 0; --k)
        {
            objectList.push_back(readInstance());
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

    //
    // Iterate over the object list and invoke ice_postUnmarshal on
    // each object.  We must do this after all objects have been
    // unmarshaled in order to ensure that any object data members
    // have been properly patched.
    //
    for(ObjectList::iterator p = objectList.begin(); p != objectList.end(); ++p)
    {
        try
        {
            (*p)->ice_postUnmarshal();
        }
        catch(const std::exception& ex)
        {
            Warning out(_stream->instance()->initializationData().logger);
            out << "std::exception raised by ice_postUnmarshal:\n" << ex;
        }
        catch(...)
        {
            Warning out(_stream->instance()->initializationData().logger);
            out << "unknown exception raised by ice_postUnmarshal";
        }
    }
}

ObjectPtr
IceInternal::BasicStream::EncapsDecoder::readInstance()
{
    Int index;
    if(_encaps->encoding == Encoding_1_0)
    {
        _stream->read(index);
    }
    else
    {
        index = _stream->readSize();
    }

    ObjectPtr v;
    if(index <= 0)
    {
        throw MarshalException(__FILE__, __LINE__, "invalid object id");
    }

    _sliceType = ObjectSlice;
    _skipFirstSlice = false;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _typeId;
    ObjectFactoryManagerPtr servantFactoryManager = _stream->instance()->servantFactoryManager();
    while(true)
    {
        //
        // For the 1.0 encoding, the type ID for the base Object class
        // marks the last slice.
        //
        if(_typeId == Object::ice_staticId())
        {
            throw NoObjectFactoryException(__FILE__, __LINE__, "", mostDerivedId);
        }
        
        if(_compactId >= 0)
        {
            _typeId = IceInternal::factoryTable->getTypeId(_compactId);
        }

        if(!_typeId.empty())
        {
            v = newInstance(servantFactoryManager, _typeId);

            //
            // We found a factory, we get out of this loop.
            //
            if(v)
            {
                break;
            }
        }

        //
        // Performance sensitive, so we use lazy initialization for tracing.
        //
        if(_traceSlicing == -1)
        {
            _traceSlicing = _stream->instance()->traceLevels()->slicing;
            _slicingCat = _stream->instance()->traceLevels()->slicingCat;
        }
        if(_traceSlicing > 0)
        {
            traceSlicing("class", _typeId, _slicingCat, _stream->instance()->initializationData().logger);
        }

        //
        // If object slicing is disabled, stop un-marshalling.
        //
        if(!_sliceObjects)
        {
            throw NoObjectFactoryException(__FILE__, __LINE__, "object slicing is disabled", _typeId);
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();

        //
        // If this is the last slice, keep the object as an opaque UnknownSlicedObject.
        //
        if(_sliceFlags & FLAG_IS_LAST_SLICE)
        {
            //
            // Provide a factory with an opportunity to supply the object.
            // We pass the "::Ice::Object" ID to indicate that this is the
            // last chance to preserve the object.
            //
            v = newInstance(servantFactoryManager, Object::ice_staticId());
            if(!v)
            {
                v = new UnknownSlicedObject(mostDerivedId);
            }

            break;
        }
        
        startSlice(); // Read next Slice header for next iteration.
    }
     
    //
    // Add the object to the map of un-marshalled objects, this must
    // be done before reading the objects (for circular references).
    // 
    _unmarshaledMap.insert(make_pair(index, v));

    //
    // Read the object.
    //
    v->__read(_stream);

    //
    // Patch all instances now that the object is un-marshalled.
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

    return v;
}

void
IceInternal::BasicStream::EncapsDecoder::addPatchEntry(Int index, PatchFunc patchFunc, void* patchAddr)
{
    assert(index > 0);

    //
    // Check if already un-marshalled the object. If that's the case,
    // just patch the object smart pointer and we're done.
    //
    IndexToPtrMap::iterator p = _unmarshaledMap.find(index);
    if(p != _unmarshaledMap.end())
    {
        (*patchFunc)(patchAddr, p->second);
        return;
    }

    //
    // Add patch entry if the object isn't un-marshalled yet, the
    // smart pointer will be patched when the instance is
    // un-marshalled.
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
IceInternal::BasicStream::EncapsDecoder::skipSlice()
{
    Container::iterator start = _stream->i;

    if(_sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        assert(_sliceSize >= 4);
        _stream->skip(_sliceSize - sizeof(Int));
    }
    else
    {
        if(_sliceType == ObjectSlice)
        {
            throw NoObjectFactoryException(
                __FILE__, __LINE__, 
                "compact format prevents slicing (the sender should use the sliced format instead)", 
                _typeId);
        }
        else
        {
            throw UnknownUserException(__FILE__, __LINE__, _typeId.substr(2));
        }
    }

    if(_encaps->encoding != Encoding_1_0)
    {
        //
        // Preserve this slice.
        //
        SliceInfoPtr info = new SliceInfo;
        info->typeId = _typeId;
        info->compactId = _compactId;
        info->hasOptionalMembers = _sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS;
        info->isLastSlice = _sliceFlags & FLAG_IS_LAST_SLICE;
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
        _slices.push_back(info);

        _indirectionTables.push_back(IndexList());

        if(_sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
        {
            //
            // Read the indirection table, which is written as a sequence<size> to conserve space.
            //
            _stream->readSizeSeq(_indirectionTables.back());
        }
    }
}

SlicedDataPtr 
IceInternal::BasicStream::EncapsDecoder::readSlicedData()
{
    if(_slices.empty()) // No preserved slices.
    {
        return 0;
    }

    //
    // The _indirectionTables member holds the indirection table for each slice
    // in _slices.
    //
    assert(_slices.size() == _indirectionTables.size());

    for(SliceInfoSeq::size_type n = 0; n < _slices.size(); ++n)
    {
        //
        // We use the "objects" list in SliceInfo to hold references to the target
        // objects. Note however that we may not have actually read these objects
        // yet, so they need to be treated just like we had read the object references
        // directly (i.e., we add them to the patch list).
        //
        // Another important note: the SlicedData object that we return here must
        // not be destroyed before readPendingObjects is called, otherwise the
        // patch references will refer to invalid addresses.
        //
        const IndexList& table = _indirectionTables[n];
        _slices[n]->objects.resize(table.size());
        IndexList::size_type j = 0;
        for(IndexList::const_iterator p = table.begin(); p != table.end(); ++p)
        {
            const Int id = *p;
            if(id <= 0)
            {
                throw MarshalException(__FILE__, __LINE__, "invalid id in object indirection table");
            }
            addPatchEntry(id, &patchHandle<Object>, &_slices[n]->objects[j++]);
        }
    }

    return new SlicedData(_slices);
}

Ice::ObjectPtr
IceInternal::BasicStream::EncapsDecoder::newInstance(const ObjectFactoryManagerPtr& servantFactoryManager,
                                                     const string& typeId)
{
    Ice::ObjectPtr v;

    //
    // Try to find a factory registered for the specific type.
    //
    ObjectFactoryPtr userFactory = servantFactoryManager->find(typeId);
    if(userFactory)
    {
        v = userFactory->create(typeId);
    }

    //
    // If that fails, invoke the default factory if one has been
    // registered.
    //
    if(!v)
    {
        userFactory = servantFactoryManager->find("");
        if(userFactory)
        {
            v = userFactory->create(typeId);
        }
    }

    //
    // Last chance: check the table of static factories (i.e.,
    // automatically generated factories for concrete classes).
    //
    if(!v)
    {
        ObjectFactoryPtr of = IceInternal::factoryTable->getObjectFactory(typeId);
        if(of)
        {
            v = of->create(typeId);
            assert(v);
        }
    }

    return v;
}
