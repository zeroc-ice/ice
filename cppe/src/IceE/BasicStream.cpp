// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/Properties.h>
#include <IceE/BasicStream.h>
#include <IceE/Instance.h>
#include <IceE/ProxyFactory.h>
#include <IceE/UserExceptionFactory.h>
#include <IceE/LocalException.h>
#include <IceE/Protocol.h>
#include <IceE/FactoryTable.h>
#include <IceE/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

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

    while(_seqDataStack)
    {
	SeqData* oldSeqData = _seqDataStack;
	_seqDataStack = _seqDataStack->previous;
	delete oldSeqData;
    }
}

void
IceInternal::BasicStream::swap(BasicStream& other)
{
    assert(_instance == other._instance);

    Buffer::swap(other);

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

    std::swap(_seqDataStack, other._seqDataStack);
    std::swap(_unlimited, other._unlimited);
}

//
// startSeq() and endSeq() sanity-check sequence sizes during
// unmarshaling and prevent malicious messages with incorrect sequence
// sizes from causing the receiver to use up all available memory by
// allocating sequences with an impossibly large number of elements.
//
// The code generator inserts calls to startSeq() and endSeq() around
// the code to unmarshal a sequence of a variable-length type. startSeq()
// is called immediately after reading the sequence size, and endSeq() is
// called after reading the final element of a sequence.
//
// For a sequence of a fixed-length type, the code generator inserts a
// call to checkFixedSeq(), which does not cause any memory allocations.
//
// For sequences that contain constructed types that, in turn, contain
// sequences, the code generator also inserts a call to endElement()
// (inlined in BasicStream.h) after unmarshaling each element.
//
// startSeq() is passed the unmarshaled element count, plus the
// minimum size (in bytes) occupied by the sequence's element
// type. numElements * minSize is the smallest possible number of
// bytes that the sequence will occupy on the wire.
//
// Every time startSeq() is called, it pushes the element count and
// the minimum size on a stack. Every time endSeq() is called, it pops
// the stack.
//
// For an ordinary sequence (one that does not (recursively) contain
// nested sequences), numElements * minSize must be less than the
// number of bytes remaining in the stream.
//
// For a sequence that is nested within some other sequence, there
// must be enough bytes remaining in the stream for this sequence
// (numElements + minSize), plus the sum of the bytes required by the
// remaining elements of all the enclosing sequences.
//
// For the enclosing sequences, numElements - 1 is the number of
// elements for which unmarshaling has not started yet. (The call to
// endElement() in the generated code decrements that number whenever
// a sequence element is unmarshaled.)
//
// For sequences that have variable-length elements, checkSeq() is called
// whenever an element is unmarshaled. checkSeq() also checks whether
// the stream has a sufficient number of bytes remaining.  This means
// that, for messages with bogus sequence sizes, unmarshaling is
// aborted at the earliest possible point.
//

void
IceInternal::BasicStream::startSeq(int numElements, int minSize)
{
    if(numElements == 0) // Optimization to avoid pushing a useless stack frame.
    {
	return;
    }

    //
    // Push the current sequence details on the stack.
    //
    SeqData* sd = new SeqData(numElements, minSize);
    sd->previous = _seqDataStack;
    _seqDataStack = sd;

    int bytesLeft = static_cast<int>(b.end() - i);
    if(_seqDataStack->previous == 0) // Outermost sequence
    {
	//
	// The sequence must fit within the message.
	//
	if(numElements * minSize > bytesLeft) 
	{
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
    }
    else // Nested sequence
    {
	checkSeq(bytesLeft);
    }
}

void
IceInternal::BasicStream::checkFixedSeq(int numElements, int elemSize)
{
    int bytesLeft = static_cast<int>(b.end() - i);
    if(_seqDataStack == 0) // Outermost sequence
    {
	//
	// The sequence must fit within the message.
	//
	if(numElements * elemSize > bytesLeft) 
	{
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
    }
    else // Nested sequence
    {
	checkSeq(bytesLeft - numElements * elemSize);
    }
}

void
IceInternal::BasicStream::endSeq(int sz)
{
    if(sz == 0) // Pop only if something was pushed previously.
    {
	return;
    }

    //
    // Pop the sequence stack.
    //
    SeqData* oldSeqData = _seqDataStack;
    assert(oldSeqData);
    _seqDataStack = oldSeqData->previous;
    delete oldSeqData;
}

void
IceInternal::BasicStream::WriteEncaps::swap(WriteEncaps& other)
{
    std::swap(start, other.start);

    std::swap(writeIndex, other.writeIndex);
    std::swap(previous, other.previous);
}

void
IceInternal::BasicStream::ReadEncaps::swap(ReadEncaps& other)
{
    std::swap(start, other.start);
    std::swap(sz, other.sz);

    std::swap(encodingMajor, other.encodingMajor);
    std::swap(encodingMinor, other.encodingMinor);

    std::swap(previous, other.previous);
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
    if(sz < 0)
    {
	throwNegativeSizeException(__FILE__, __LINE__);
    }
    if(i - sizeof(Int) + sz > b.end())
    {
	throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
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
    if(sz < 0)
    {
	throwNegativeSizeException(__FILE__, __LINE__);
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
    if(sz < 0)
    {
	throwNegativeSizeException(__FILE__, __LINE__);
    }
    i += sz - sizeof(Int);
    if(i > b.end())
    {
	throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
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
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
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
IceInternal::BasicStream::read(pair<const Byte*, const Byte*>& v)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	checkFixedSeq(sz, 1);
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
    readSize(sz);
    if(sz > 0)
    {
        checkFixedSeq(sz, 1);
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
    readSize(sz);
    if(sz > 0)
    {
        checkFixedSeq(sz, 1);
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
IceInternal::BasicStream::read(Short& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Short)))
    {
	throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
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
IceInternal::BasicStream::read(vector<Short>& v)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	checkFixedSeq(sz, static_cast<int>(sizeof(Short)));
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
    readSize(sz);
    if(sz > 0)
    {
        checkFixedSeq(sz, static_cast<int>(sizeof(Short)));
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
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	checkFixedSeq(sz, static_cast<int>(sizeof(Int)));
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
    readSize(sz);
    if(sz > 0)
    {
        checkFixedSeq(sz, static_cast<int>(sizeof(Int)));
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
IceInternal::BasicStream::read(Long& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Long)))
    {
	throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
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
IceInternal::BasicStream::read(vector<Long>& v)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	checkFixedSeq(sz, static_cast<int>(sizeof(Long)));
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
    readSize(sz);
    if(sz > 0)
    {
        checkFixedSeq(sz, static_cast<int>(sizeof(Long)));
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
IceInternal::BasicStream::read(Float& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Float)))
    {
	throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
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
IceInternal::BasicStream::read(vector<Float>& v)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	checkFixedSeq(sz, static_cast<int>(sizeof(Float)));
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
    readSize(sz);
    if(sz > 0)
    {
        checkFixedSeq(sz, static_cast<int>(sizeof(Float)));
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
IceInternal::BasicStream::read(Double& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Double)))
    {
	throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
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
IceInternal::BasicStream::read(vector<Double>& v)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	checkFixedSeq(sz, static_cast<int>(sizeof(Double)));
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
    readSize(sz);
    if(sz > 0)
    {
        checkFixedSeq(sz, static_cast<int>(sizeof(Double)));
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

#ifdef ICEE_HAS_WSTRING

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

#endif

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
IceInternal::BasicStream::read(vector<string>& v, bool convert)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	startSeq(sz, 1);
	v.resize(sz);
	for(int i = 0; i < sz; ++i)
	{
	    read(v[i], convert);
	    checkSeq();
	    endElement();
	}
	endSeq(sz);
    }
    else
    {
       v.clear();
    }
}

#ifdef ICEE_HAS_WSTRING

void
IceInternal::BasicStream::write(const wstring& v)
{
    if(v.size() == 0)
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
IceInternal::BasicStream::read(vector<wstring>& v)
{   
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
        startSeq(sz, 1);
        v.resize(sz);
        for(int j = 0; j < sz; ++j)
        {
            read(v[j]);
            checkSeq();
            endElement();
        }
        endSeq(sz);
    }
    else
    {
       v.clear();
    }
}

#endif

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
IceInternal::BasicStream::write(const UserException& v)
{
    write(false);
    v.__write(this);
}

void
IceInternal::BasicStream::throwException()
{
    bool usesClasses;
    read(usesClasses);

    string id;
    read(id, false);
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
		assert(!usesClasses);
		ex.ice_throw();
	    }
	}
	else
	{
	    skipSlice(); // Slice off what we don't understand.
	    read(id, false); // Read type id for next slice.
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

IceInternal::BasicStream::SeqData::SeqData(int num, int sz) : numElements(num), minSize(sz)
{
}
