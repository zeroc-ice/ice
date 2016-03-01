// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/StringConverter.h>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

class StreamUTF8BufferI : public IceUtil::UTF8Buffer
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
            _stream.resize(firstUnused - _stream.b.begin());
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

const Byte OPTIONAL_END_MARKER        = 0xFF;

const Byte FLAG_HAS_TYPE_ID_STRING    = (1<<0);
const Byte FLAG_HAS_TYPE_ID_INDEX     = (1<<1);
const Byte FLAG_HAS_TYPE_ID_COMPACT   = (1<<0) | (1<<1);
const Byte FLAG_HAS_OPTIONAL_MEMBERS  = (1<<2);
const Byte FLAG_HAS_INDIRECTION_TABLE = (1<<3);
const Byte FLAG_HAS_SLICE_SIZE        = (1<<4);
const Byte FLAG_IS_LAST_SLICE         = (1<<5);

}


IceInternal::BasicStream::BasicStream(Instance* instance, const EncodingVersion& encoding) :
    _instance(instance),
    _closure(0),
    _encoding(encoding),
    _currentReadEncaps(0),
    _currentWriteEncaps(0),
    _sliceObjects(true),
    _stringConverter(instance->getStringConverter()),
    _wstringConverter(instance->getWstringConverter()),
    _startSeq(-1),
    _minSeqSize(0)
{
    //
    // Initialize the encoding members of our pre-allocated encapsulations, in case
    // this stream is used without an explicit encapsulation.
    //
    _preAllocatedReadEncaps.encoding = encoding;
    _preAllocatedWriteEncaps.encoding = encoding;
}

IceInternal::BasicStream::BasicStream(Instance* instance, const EncodingVersion& encoding, const Byte* begin,
                                      const Byte* end) :
    IceInternal::Buffer(begin, end),
    _instance(instance),
    _closure(0),
    _encoding(encoding),
    _currentReadEncaps(0),
    _currentWriteEncaps(0),
    _sliceObjects(true),
    _stringConverter(instance->getStringConverter()),
    _wstringConverter(instance->getWstringConverter()),
    _startSeq(-1),
    _minSeqSize(0)
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

    std::swap(_startSeq, other._startSeq);
    std::swap(_minSeqSize, other._minSeqSize);
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

void
IceInternal::BasicStream::read(pair<const bool*, const bool*>& v, IceUtil::ScopedArray<bool>& result)
{
    Int sz = readAndCheckSeqSize(1);
    if(sz > 0)
    {
        result.reset(BasicStreamReadBoolHelper<sizeof(bool)>::read(v, sz, i));
        i += sz;
    }
    else
    {
        result.reset();
        v.first = v.second = reinterpret_cast<bool*>(i);
    }
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

void
IceInternal::BasicStream::read(pair<const Short*, const Short*>& v, IceUtil::ScopedArray<Short>& result)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Short)));
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86) || defined(__x86_64) || defined(_M_X64)
        v.first = reinterpret_cast<Short*>(i);
        i += sz * static_cast<int>(sizeof(Short));
        v.second = reinterpret_cast<Short*>(i);
#else
        result.reset(new Short[sz]);
        v.first = result.get();
        v.second = result.get() + sz;

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
        result.reset();
        v.first = v.second = 0;
    }
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

void
IceInternal::BasicStream::read(pair<const Int*, const Int*>& v, ::IceUtil::ScopedArray<Int>& result)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Int)));
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86) || defined(__x86_64) || defined(_M_X64)
        v.first = reinterpret_cast<Int*>(i);
        i += sz * static_cast<int>(sizeof(Int));
        v.second = reinterpret_cast<Int*>(i);
#else
        result.reset(new Int[sz]);
        v.first = result.get();
        v.second = result.get() + sz;

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
        result.reset();
        v.first = v.second = 0;
    }
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

void
IceInternal::BasicStream::read(pair<const Long*, const Long*>& v, IceUtil::ScopedArray<Long>& result)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Long)));
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86) || defined(__x86_64) || defined(_M_X64)
        v.first = reinterpret_cast<Long*>(i);
        i += sz * static_cast<int>(sizeof(Long));
        v.second = reinterpret_cast<Long*>(i);
#else
        result.reset(new Long[sz]);
        v.first = result.get();
        v.second = result.get() + sz;

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
        result.reset();
        v.first = v.second = 0;
    }
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

void
IceInternal::BasicStream::read(pair<const Float*, const Float*>& v, IceUtil::ScopedArray<Float>& result)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Float)));
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86) || defined(__x86_64) || defined(_M_X64)
        v.first = reinterpret_cast<Float*>(i);
        i += sz * static_cast<int>(sizeof(Float));
        v.second = reinterpret_cast<Float*>(i);
#else
        result.reset(new Float[sz]);
        v.first = result.get();
        v.second = result.get() + sz;

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
        result.reset();
        v.first = v.second = 0;
    }
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
#elif defined(ICE_LITTLEBYTE_BIGWORD)
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

void
IceInternal::BasicStream::read(pair<const Double*, const Double*>& v, IceUtil::ScopedArray<Double>& result)
{
    Int sz = readAndCheckSeqSize(static_cast<int>(sizeof(Double)));
    if(sz > 0)
    {
#if defined(__i386) || defined(_M_IX86) || defined(__x86_64) || defined(_M_X64)
        v.first = reinterpret_cast<Double*>(i);
        i += sz * static_cast<int>(sizeof(Double));
        v.second = reinterpret_cast<Double*>(i);
#else
        result.reset(new Double[sz]);
        v.first = result.get();
        v.second = result.get() + sz;

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
        result.reset();
        v.first = v.second = 0;
    }
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
IceInternal::BasicStream::writeConverted(const char* vdata, size_t vsize)
{
    //
    // What is the size of the resulting UTF-8 encoded string?
    // Impossible to tell, so we guess. If we don't guess correctly,
    // we'll have to fix the mistake afterwards
    //
    try
    {
        Int guessedSize = static_cast<Int>(vsize);
        writeSize(guessedSize); // writeSize() only writes the size; it does not reserve any buffer space.

        size_t firstIndex = b.size();
        StreamUTF8BufferI buffer(*this);

        Byte* lastByte = _stringConverter->toUTF8(vdata, vdata + vsize, buffer);
        if(lastByte != b.end())
        {
            resize(lastByte - b.begin());
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
    catch(const IceUtil::IllegalConversionException& ex)
    {
        throw StringConversionException(__FILE__, __LINE__, ex.reason());
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
    try
    {
        _stringConverter->fromUTF8(i, i + sz, v);
    }
    catch(const IceUtil::IllegalConversionException& ex)
    {
        throw StringConversionException(__FILE__, __LINE__, ex.reason());
    }
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
    try
    {
        Int guessedSize = static_cast<Int>(v.size());
        writeSize(guessedSize); // writeSize() only writes the size; it does not reserve any buffer space.

        size_t firstIndex = b.size();
        StreamUTF8BufferI buffer(*this);

        Byte* lastByte = _wstringConverter->toUTF8(v.data(), v.data() + v.size(), buffer);
        if(lastByte != b.end())
        {
            resize(lastByte - b.begin());
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
    catch(const IceUtil::IllegalConversionException& ex)
    {
        throw StringConversionException(__FILE__, __LINE__, ex.reason());
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

        try
        {
            _wstringConverter->fromUTF8(i, i + sz, v);
            i += sz;
        }
        catch(const IceUtil::IllegalConversionException& ex)
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
IceInternal::BasicStream::writeException(const UserException& e)
{
    initWriteEncaps();
    _currentWriteEncaps->encoder->write(e);
}

void
IceInternal::BasicStream::throwException(const UserExceptionFactoryPtr& factory)
{
    initReadEncaps();
    _currentReadEncaps->decoder->throwException(factory);
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

    while(true)
    {
        if(i >= b.begin() + _currentReadEncaps->start + _currentReadEncaps->sz)
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
            skipOpt(format); // Skip optional data members
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

bool
IceInternal::BasicStream::writeOptImpl(Int tag, OptionalFormat type)
{
    if(getWriteEncoding() == Encoding_1_0)
    {
        return false; // Optional members aren't supported with the 1.0 encoding.
    }

    Byte v = static_cast<Byte>(type);
    if(tag < 30)
    {
        v |= static_cast<Byte>(tag << 3);
        write(v);
    }
    else
    {
        v |= 0xF0; // tag = 30
        write(v);
        writeSize(tag);
    }
    return true;
}

void
IceInternal::BasicStream::skipOpt(OptionalFormat type)
{
    switch(type)
    {
    case Ice::OptionalFormatF1:
    {
        skip(1);
        break;
    }
    case Ice::OptionalFormatF2:
    {
        skip(2);
        break;
    }
    case Ice::OptionalFormatF4:
    {
        skip(4);
        break;
    }
    case Ice::OptionalFormatF8:
    {
        skip(8);
        break;
    }
    case Ice::OptionalFormatSize:
    {
        skipSize();
        break;
    }
    case Ice::OptionalFormatVSize:
    {
        skip(readSize());
        break;
    }
    case Ice::OptionalFormatFSize:
    {
        Int sz;
        read(sz);
        skip(sz);
        break;
    }
    case Ice::OptionalFormatClass:
    {
        read(0, 0);
        break;
    }
    }
}

void
BasicStream::skipOpts()
{
    //
    // Skip remaining un-read optional members.
    //
    while(true)
    {
        if(i >= b.begin() + _currentReadEncaps->start + _currentReadEncaps->sz)
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
        skipOpt(format);
    }
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
        ObjectFactoryManagerPtr factoryManager = _instance->servantFactoryManager();
        if(_currentReadEncaps->encoding == Encoding_1_0)
        {
            _currentReadEncaps->decoder = new EncapsDecoder10(this, _currentReadEncaps, _sliceObjects, factoryManager);
        }
        else
        {
            _currentReadEncaps->decoder = new EncapsDecoder11(this, _currentReadEncaps, _sliceObjects, factoryManager);
        }
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
        if(_currentWriteEncaps->encoding == Encoding_1_0)
        {
            _currentWriteEncaps->encoder = new EncapsEncoder10(this, _currentWriteEncaps);
        }
        else
        {
            _currentWriteEncaps->encoder = new EncapsEncoder11(this, _currentWriteEncaps);
        }
    }
}

string
IceInternal::BasicStream::EncapsDecoder::readTypeId(bool isIndex)
{
    if(isIndex)
    {
        Int index = _stream->readSize();
        TypeIdReadMap::const_iterator k = _typeIdMap.find(index);
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

Ice::ObjectPtr
IceInternal::BasicStream::EncapsDecoder::newInstance(const string& typeId)
{
    Ice::ObjectPtr v;

    //
    // Try to find a factory registered for the specific type.
    //
    ObjectFactoryPtr userFactory = _servantFactoryManager->find(typeId);
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
        userFactory = _servantFactoryManager->find("");
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
IceInternal::BasicStream::EncapsDecoder::unmarshal(Int index, const Ice::ObjectPtr& v)
{
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

    if(_objectList.empty() && _patchMap.empty())
    {
        try
        {
            if(_stream->instance()->collectObjects())
            {
                v->ice_collectable(true);
            }
            v->ice_postUnmarshal();
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
    else
    {
        _objectList.push_back(v);

        if(_patchMap.empty())
        {
            //
            // Iterate over the object list and invoke ice_postUnmarshal on
            // each object.  We must do this after all objects have been
            // unmarshaled in order to ensure that any object data members
            // have been properly patched.
            //
            for(ObjectList::iterator p = _objectList.begin(); p != _objectList.end(); ++p)
            {
                try
                {
                    if(_stream->instance()->collectObjects())
                    {
                        (*p)->ice_collectable(true);
                    }
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
            _objectList.clear();
        }
    }
}

void
IceInternal::BasicStream::EncapsDecoder10::read(PatchFunc patchFunc, void* patchAddr)
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
        ObjectPtr nil;
        patchFunc(patchAddr, nil);
    }
    else
    {
        addPatchEntry(index, patchFunc, patchAddr);
    }
}

void
IceInternal::BasicStream::EncapsDecoder10::throwException(const UserExceptionFactoryPtr& factory)
{
    assert(_sliceType == NoSlice);

    //
    // User exception with the 1.0 encoding start with a boolean flag
    // that indicates whether or not the exception has classes.
    //
    // This allows reading the pending objects even if some part of
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
IceInternal::BasicStream::EncapsDecoder10::startInstance(SliceType sliceType)
#else
IceInternal::BasicStream::EncapsDecoder10::startInstance(SliceType)
#endif
{
    assert(_sliceType == sliceType);
    _skipFirstSlice = true;
}

SlicedDataPtr
IceInternal::BasicStream::EncapsDecoder10::endInstance(bool)
{
    //
    // Read the Ice::Object slice.
    //
    if(_sliceType == ObjectSlice)
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
IceInternal::BasicStream::EncapsDecoder10::startSlice()
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
    // For objects, first read the type ID boolean which indicates
    // whether or not the type ID is encoded as a string or as an
    // index. For exceptions, the type ID is always encoded as a
    // string.
    //
    if(_sliceType == ObjectSlice)
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
IceInternal::BasicStream::EncapsDecoder10::endSlice()
{
}

void
IceInternal::BasicStream::EncapsDecoder10::skipSlice()
{
    if(_stream->instance()->traceLevels()->slicing > 0)
    {
        const Ice::LoggerPtr& logger = _stream->instance()->initializationData().logger;
        if(_sliceType == ExceptionSlice)
        {
            traceSlicing("exception", _typeId, _stream->instance()->traceLevels()->slicingCat, logger);
        }
        else
        {
            traceSlicing("object", _typeId, _stream->instance()->traceLevels()->slicingCat, logger);
        }
    }
    assert(_sliceSize >= 4);
    _stream->skip(_sliceSize - sizeof(Int));
}

void
IceInternal::BasicStream::EncapsDecoder10::readPendingObjects()
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
IceInternal::BasicStream::EncapsDecoder10::readInstance()
{
    Int index;
    _stream->read(index);

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
    ObjectPtr v;
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

        v = newInstance(_typeId);

        //
        // We found a factory, we get out of this loop.
        //
        if(v)
        {
            break;
        }

        //
        // If object slicing is disabled, stop un-marshalling.
        //
        if(!_sliceObjects)
        {
            throw NoObjectFactoryException(__FILE__, __LINE__, "no object factory found and object slicing is disabled",
                                           _typeId);
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();
        startSlice(); // Read next Slice header for next iteration.
    }

    //
    // Un-marshal the object and add-it to the map of un-marshaled objects.
    //
    unmarshal(index, v);
}

void
IceInternal::BasicStream::EncapsDecoder11::read(PatchFunc patchFunc, void* patchAddr)
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
            ObjectPtr nil;
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
IceInternal::BasicStream::EncapsDecoder11::throwException(const UserExceptionFactoryPtr& factory)
{
    assert(!_current);

    push(ExceptionSlice);

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _current->typeId;
    UserExceptionFactoryPtr exceptionFactory = factory;
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
                exceptionFactory->createAndThrow(_current->typeId);
            }
            catch(UserException& ex)
            {
                ex.__read(_stream);
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
            if(mostDerivedId.length() > 2 && mostDerivedId[0] == ':' && mostDerivedId[1] == ':')
            {
                throw UnknownUserException(__FILE__, __LINE__, mostDerivedId.substr(2));
            }
            else
            {
                throw UnknownUserException(__FILE__, __LINE__, mostDerivedId);
            }
        }

        startSlice();
    }
}

void
#ifndef NDEBUG
IceInternal::BasicStream::EncapsDecoder11::startInstance(SliceType sliceType)
#else
IceInternal::BasicStream::EncapsDecoder11::startInstance(SliceType)
#endif
{
    assert(_current->sliceType == sliceType);
    _current->skipFirstSlice = true;
}

SlicedDataPtr
IceInternal::BasicStream::EncapsDecoder11::endInstance(bool preserve)
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
IceInternal::BasicStream::EncapsDecoder11::startSlice()
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
    // Read the type ID, for object slices the type ID is encoded as a
    // string or as an index, for exceptions it's always encoded as a
    // string.
    //
    if(_current->sliceType == ObjectSlice)
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
IceInternal::BasicStream::EncapsDecoder11::endSlice()
{
    if(_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        _stream->skipOpts();
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
IceInternal::BasicStream::EncapsDecoder11::skipSlice()
{
    if(_stream->instance()->traceLevels()->slicing > 0)
    {
        const Ice::LoggerPtr& logger = _stream->instance()->initializationData().logger;
        if(_current->sliceType == ExceptionSlice)
        {
            traceSlicing("exception", _current->typeId, _stream->instance()->traceLevels()->slicingCat, logger);
        }
        else
        {
            traceSlicing("object", _current->typeId, _stream->instance()->traceLevels()->slicingCat, logger);
        }
    }

    Container::iterator start = _stream->i;

    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        assert(_current->sliceSize >= 4);
        _stream->skip(_current->sliceSize - sizeof(Int));
    }
    else
    {
        if(_current->sliceType == ObjectSlice)
        {
            throw NoObjectFactoryException(__FILE__, __LINE__,
                                           "no object factory found and compact format prevents "
                                           "slicing (the sender should use the sliced format instead)",
                                           _current->typeId);
        }
        else
        {
            if(_current->typeId.length() > 2 && _current->typeId[0] == ':' && _current->typeId[1] == ':')
            {
                throw UnknownUserException(__FILE__, __LINE__, _current->typeId.substr(2));
            }
            else
            {
                throw UnknownUserException(__FILE__, __LINE__, _current->typeId);
            }
        }
    }

    //
    // Preserve this slice.
    //
    SliceInfoPtr info = new SliceInfo;
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
IceInternal::BasicStream::EncapsDecoder11::readOpt(Ice::Int readTag, Ice::OptionalFormat expectedFormat)
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
IceInternal::BasicStream::EncapsDecoder11::readInstance(Int index, PatchFunc patchFunc, void* patchAddr)
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

    push(ObjectSlice);

    //
    // Get the object ID before we start reading slices. If some
    // slices are skiped, the indirect object table are still read and
    // might read other objects.
    //
    index = ++_objectIdIndex;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _current->typeId;
    Ice::ObjectPtr v;
    const CompactIdResolverPtr& compactIdResolver = _stream->instance()->initializationData().compactIdResolver;
    while(true)
    {
        if(_current->compactId >= 0)
        {
            //
            // Translate a compact (numeric) type ID into a string type ID.
            //
            _current->typeId.clear();
            if(compactIdResolver)
            {
                try
                {
                    _current->typeId = compactIdResolver->resolve(_current->compactId);
                }
                catch(const LocalException&)
                {
                    throw;
                }
                catch(const std::exception& ex)
                {
                    ostringstream ostr;
                    ostr << "exception in CompactIdResolver for ID " << _current->compactId;
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
                    ostr << "unknown exception in CompactIdResolver for ID " << _current->compactId;
                    throw MarshalException(__FILE__, __LINE__, ostr.str());
                }
            }
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
        // If object slicing is disabled, stop un-marshalling.
        //
        if(!_sliceObjects)
        {
            throw NoObjectFactoryException(__FILE__, __LINE__, "no object factory found and object slicing is disabled",
                                           _current->typeId);
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();

        //
        // If this is the last slice, keep the object as an opaque UnknownSlicedObject.
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
                v = new UnknownSlicedObject(mostDerivedId);
            }

            break;
        }

        startSlice(); // Read next Slice header for next iteration.
    }

    //
    // Un-marshal the object
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
IceInternal::BasicStream::EncapsDecoder11::readSlicedData()
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
        // We use the "objects" list in SliceInfo to hold references
        // to the target objects. Note that the objects might not have
        // been read yet in the case of a circular reference to an
        // enclosing object.
        //
        const IndexList& table = _current->indirectionTables[n];
        vector<ObjectPtr>& objects = _current->slices[n]->objects;
        objects.resize(table.size());
        IndexList::size_type j = 0;
        for(IndexList::const_iterator p = table.begin(); p != table.end(); ++p)
        {
            addPatchEntry(*p, &patchHandle<Object>, &objects[j++]);
        }
    }
    return new SlicedData(_current->slices);
}

Int
IceInternal::BasicStream::EncapsEncoder::registerTypeId(const string& typeId)
{
    TypeIdWriteMap::const_iterator p = _typeIdMap.find(typeId);
    if(p != _typeIdMap.end())
    {
        return p->second;
    }
    else
    {
        _typeIdMap.insert(make_pair(typeId, ++_typeIdIndex));
        return -1;
    }
}

void
IceInternal::BasicStream::EncapsEncoder10::write(const ObjectPtr& v)
{
    //
    // Object references are encoded as a negative integer in 1.0.
    //
    if(v)
    {
        _stream->write(-registerObject(v));
    }
    else
    {
        _stream->write(0);
    }
}

void
IceInternal::BasicStream::EncapsEncoder10::write(const UserException& v)
{
    //
    // User exception with the 1.0 encoding start with a boolean
    // flag that indicates whether or not the exception uses
    // classes.
    //
    // This allows reading the pending objects even if some part of
    // the exception was sliced.
    //
    bool usesClasses = v.__usesClasses();
    _stream->write(usesClasses);
    v.__write(_stream);
    if(usesClasses)
    {
        writePendingObjects();
    }
}

void
IceInternal::BasicStream::EncapsEncoder10::startInstance(SliceType sliceType, const SlicedDataPtr&)
{
    _sliceType = sliceType;
}

void
IceInternal::BasicStream::EncapsEncoder10::endInstance()
{
    if(_sliceType == ObjectSlice)
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
IceInternal::BasicStream::EncapsEncoder10::startSlice(const string& typeId, int, bool /*last*/)
{
    //
    // For object slices, encode a boolean to indicate how the type ID
    // is encoded and the type ID either as a string or index. For
    // exception slices, always encode the type ID as a string.
    //
    if(_sliceType == ObjectSlice)
    {
        Int index = registerTypeId(typeId);
        if(index < 0)
        {
            _stream->write(false);
            _stream->write(typeId, false);
        }
        else
        {
            _stream->write(true);
            _stream->writeSize(index);
        }
    }
    else
    {
        _stream->write(typeId, false);
    }

    _stream->write(Int(0)); // Placeholder for the slice length.

    _writeSlice = _stream->b.size();
}

void
IceInternal::BasicStream::EncapsEncoder10::endSlice()
{
    //
    // Write the slice length.
    //
    Int sz = static_cast<Int>(_stream->b.size() - _writeSlice + sizeof(Int));
    Byte* dest = &(*(_stream->b.begin() + _writeSlice - sizeof(Int)));
    _stream->write(sz, dest);
}

void
IceInternal::BasicStream::EncapsEncoder10::writePendingObjects()
{
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
            _stream->write(p->second);

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

Int
IceInternal::BasicStream::EncapsEncoder10::registerObject(const ObjectPtr& v)
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
IceInternal::BasicStream::EncapsEncoder11::write(const ObjectPtr& v)
{
    if(!v)
    {
        _stream->writeSize(0); // Nil reference.
    }
    else if(_current && _encaps->format == SlicedFormat)
    {
        //
        // If writting an object within a slice and using the sliced
        // format, write an index from the object indirection
        // table. The indirect object table is encoded at the end of
        // each slice and is always read (even if the Slice is
        // unknown).
        //
        PtrToIndexMap::const_iterator p = _current->indirectionMap.find(v);
        if(p == _current->indirectionMap.end())
        {
            _current->indirectionTable.push_back(v);
            Int idx = static_cast<Int>(_current->indirectionTable.size()); // Position + 1 (0 is reserved for nil)
            _current->indirectionMap.insert(make_pair(v, idx));
            _stream->writeSize(idx);
        }
        else
        {
            _stream->writeSize(p->second);
        }
    }
    else
    {
        writeInstance(v); // Write the instance or a reference if already marshaled.
    }
}

void
IceInternal::BasicStream::EncapsEncoder11::write(const UserException& v)
{
    v.__write(_stream);
}

void
IceInternal::BasicStream::EncapsEncoder11::startInstance(SliceType sliceType, const SlicedDataPtr& data)
{
    if(!_current)
    {
        _current = &_preAllocatedInstanceData;
    }
    else
    {
        _current = _current->next ? _current->next : new InstanceData(_current);
    }
    _current->sliceType = sliceType;
    _current->firstSlice = true;

    if(data)
    {
        writeSlicedData(data);
    }
}

void
IceInternal::BasicStream::EncapsEncoder11::endInstance()
{
    _current = _current->previous;
}

void
IceInternal::BasicStream::EncapsEncoder11::startSlice(const string& typeId, int compactId, bool last)
{
    assert(_current->indirectionTable.empty() && _current->indirectionMap.empty());

    _current->sliceFlagsPos = _stream->b.size();

    _current->sliceFlags = 0;
    if(_encaps->format == SlicedFormat)
    {
        _current->sliceFlags |= FLAG_HAS_SLICE_SIZE; // Encode the slice size if using the sliced format.
    }
    if(last)
    {
        _current->sliceFlags |= FLAG_IS_LAST_SLICE; // This is the last slice.
    }

    _stream->write(Byte(0)); // Placeholder for the slice flags

    //
    // For object slices, encode the flag and the type ID either as a
    // string or index. For exception slices, always encode the type
    // ID a string.
    //
    if(_current->sliceType == ObjectSlice)
    {
        //
        // Encode the type ID (only in the first slice for the compact
        // encoding).
        //
        if(_encaps->format == SlicedFormat || _current->firstSlice)
        {
            if(compactId >= 0)
            {
                _current->sliceFlags |= FLAG_HAS_TYPE_ID_COMPACT;
                _stream->writeSize(compactId);
            }
            else
            {
                Int index = registerTypeId(typeId);
                if(index < 0)
                {
                    _current->sliceFlags |= FLAG_HAS_TYPE_ID_STRING;
                    _stream->write(typeId, false);
                }
                else
                {
                    _current->sliceFlags |= FLAG_HAS_TYPE_ID_INDEX;
                    _stream->writeSize(index);
                }
            }
        }
    }
    else
    {
        _stream->write(typeId, false);
    }

    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        _stream->write(Int(0)); // Placeholder for the slice length.
    }

    _current->writeSlice = _stream->b.size();
    _current->firstSlice = false;
}

void
IceInternal::BasicStream::EncapsEncoder11::endSlice()
{
    //
    // Write the optional member end marker if some optional members
    // were encoded. Note that the optional members are encoded before
    // the indirection table and are included in the slice size.
    //
    if(_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        _stream->write(OPTIONAL_END_MARKER);
    }

    //
    // Write the slice length if necessary.
    //
    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        Int sz = static_cast<Int>(_stream->b.size() - _current->writeSlice + sizeof(Int));
        Byte* dest = &(*(_stream->b.begin() + _current->writeSlice - sizeof(Int)));
        _stream->write(sz, dest);
    }

    //
    // Only write the indirection table if it contains entries.
    //
    if(!_current->indirectionTable.empty())
    {
        assert(_encaps->format == SlicedFormat);
        _current->sliceFlags |= FLAG_HAS_INDIRECTION_TABLE;

        //
        // Write the indirection object table.
        //
        _stream->writeSize(static_cast<Int>(_current->indirectionTable.size()));
        ObjectList::const_iterator p;
        for(p = _current->indirectionTable.begin(); p != _current->indirectionTable.end(); ++p)
        {
            writeInstance(*p);
        }
        _current->indirectionTable.clear();
        _current->indirectionMap.clear();
    }

    //
    // Finally, update the slice flags.
    //
    Byte* dest = &(*(_stream->b.begin() + _current->sliceFlagsPos));
    *dest = _current->sliceFlags;
}

bool
IceInternal::BasicStream::EncapsEncoder11::writeOpt(Ice::Int tag, Ice::OptionalFormat format)
{
    if(!_current)
    {
        return _stream->writeOptImpl(tag, format);
    }
    else
    {
        if(_stream->writeOptImpl(tag, format))
        {
            _current->sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
            return true;
        }
        else
        {
            return false;
        }
    }
}

void
IceInternal::BasicStream::EncapsEncoder11::writeSlicedData(const SlicedDataPtr& slicedData)
{
    assert(slicedData);

    //
    // We only remarshal preserved slices if we are using the sliced
    // format. Otherwise, we ignore the preserved slices, which
    // essentially "slices" the object into the most-derived type
    // known by the sender.
    //
    if(_encaps->format != SlicedFormat)
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
            _current->sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
        }

        //
        // Make sure to also re-write the object indirection table.
        //
        _current->indirectionTable = (*p)->objects;

        endSlice();
    }
}

void
IceInternal::BasicStream::EncapsEncoder11::writeInstance(const ObjectPtr& v)
{
    assert(v);

    //
    // If the instance was already marshaled, just write it's ID.
    //
    PtrToIndexMap::const_iterator q = _marshaledMap.find(v);
    if(q != _marshaledMap.end())
    {
        _stream->writeSize(q->second);
        return;
    }

    //
    // We haven't seen this instance previously, create a new ID,
    // insert it into the marshaled map, and write the instance.
    //
    _marshaledMap.insert(make_pair(v, ++_objectIdIndex));

    try
    {
        v->ice_preMarshal();
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

    _stream->writeSize(1); // Object instance marker.
    v->__write(_stream);
}

