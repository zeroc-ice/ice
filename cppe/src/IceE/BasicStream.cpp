// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/BasicStream.h>
#include <IceE/Instance.h>
#include <IceE/ProxyFactory.h>
#include <IceE/UserExceptionFactory.h>
#include <IceE/LocalExceptions.h>
#include <IceE/Protocol.h>
#include <IceE/FactoryTable.h>
#include <IceE/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::BasicStream::BasicStream(Instance* instance) :
    _instance(instance),
    _currentReadEncaps(0),
    _currentWriteEncaps(0),
    _messageSizeMax(_instance->messageSizeMax()), // Cached for efficiency.
    _seqDataStack(0)
{
}

IceInternal::BasicStream::~BasicStream()
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

Instance*
IceInternal::BasicStream::instance() const
{
    return _instance;
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
	    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
    }
    else // Nested sequence
    {
	checkSeq(bytesLeft);
    }
}

//
// Check, given the number of elements requested for this sequence,
// that this sequence, plus the sum of the sizes of the remaining
// number of elements of all enclosing sequences, would still fit
// within the message.
//
void
IceInternal::BasicStream::checkSeq()
{
    checkSeq(static_cast<int>(b.end() - i));
}

void
IceInternal::BasicStream::checkSeq(int bytesLeft)
{
    int size = 0;
    SeqData* sd = _seqDataStack;
    do
    {
	size += (sd->numElements - 1) * sd->minSize;
	sd = sd->previous;
    }
    while(sd);

    if(size > bytesLeft)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
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
	    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
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

IceInternal::BasicStream::WriteEncaps::WriteEncaps()
    : writeIndex(0), previous(0)
{
}

IceInternal::BasicStream::WriteEncaps::~WriteEncaps()
{
}

void
IceInternal::BasicStream::WriteEncaps::reset()
{
    writeIndex = 0;
    previous = 0;
}

void
IceInternal::BasicStream::WriteEncaps::swap(WriteEncaps& other)
{
    std::swap(start, other.start);

    std::swap(writeIndex, other.writeIndex);
    std::swap(previous, other.previous);
}

void
IceInternal::BasicStream::startWriteEncaps()
{
    WriteEncaps* oldEncaps = _currentWriteEncaps;
    if(!oldEncaps) // First allocated encaps?
    {
	_currentWriteEncaps = &_preAllocatedWriteEncaps;
    }
    else
    {
	_currentWriteEncaps = new WriteEncaps();
	_currentWriteEncaps->previous = oldEncaps;
    }
    _currentWriteEncaps->start = b.size();

    write(Int(0)); // Placeholder for the encapsulation length.
    write(encodingMajor);
    write(encodingMinor);
}

void
IceInternal::BasicStream::endWriteEncaps()
{
    assert(_currentWriteEncaps);
    Container::size_type start = _currentWriteEncaps->start;
    Int sz = static_cast<Int>(b.size() - start); // Size includes size and version.
    Byte* dest = &(*(b.begin() + start));

#ifdef ICEE_BIG_ENDIAN
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

    WriteEncaps* oldEncaps = _currentWriteEncaps;
    _currentWriteEncaps = _currentWriteEncaps->previous;
    if(oldEncaps == &_preAllocatedWriteEncaps)
    {
	oldEncaps->reset();
    }
    else
    {
	delete oldEncaps;
    }
}

IceInternal::BasicStream::ReadEncaps::ReadEncaps()
    : previous(0)
{
}

IceInternal::BasicStream::ReadEncaps::~ReadEncaps()
{
}

void
IceInternal::BasicStream::ReadEncaps::reset()
{
    previous = 0;
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

void
IceInternal::BasicStream::startReadEncaps()
{
    ReadEncaps* oldEncaps = _currentReadEncaps;
    if(!oldEncaps) // First allocated encaps?
    {
	_currentReadEncaps = &_preAllocatedReadEncaps;
    }
    else
    {
	_currentReadEncaps = new ReadEncaps();
	_currentReadEncaps->previous = oldEncaps;
    }
    _currentReadEncaps->start = i - b.begin();

    //
    // I don't use readSize() and writeSize() for encapsulations,
    // because when creating an encapsulation, I must know in advance
    // how many bytes the size information will require in the data
    // stream. If I use an Int, it is always 4 bytes. For
    // readSize()/writeSize(), it could be 1 or 5 bytes.
    //
    Int sz;
    read(sz);
    if(sz < 0)
    {
	throw NegativeSizeException(__FILE__, __LINE__);
    }
    if(i - sizeof(Int) + sz > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    _currentReadEncaps->sz = sz;

    Byte eMajor;
    Byte eMinor;
    read(eMajor);
    read(eMinor);
    if(eMajor != encodingMajor
       || static_cast<unsigned char>(eMinor) > static_cast<unsigned char>(encodingMinor))
    {
	UnsupportedEncodingException ex(__FILE__, __LINE__);
	ex.badMajor = static_cast<unsigned char>(eMajor);
	ex.badMinor = static_cast<unsigned char>(eMinor);
	ex.major = static_cast<unsigned char>(encodingMajor);
	ex.minor = static_cast<unsigned char>(encodingMinor);
	throw ex;
    }
    _currentReadEncaps->encodingMajor = eMajor;
    _currentReadEncaps->encodingMinor = eMinor;
}

void
IceInternal::BasicStream::endReadEncaps()
{
    assert(_currentReadEncaps);
    Container::size_type start = _currentReadEncaps->start;
    Int sz = _currentReadEncaps->sz;
    i = b.begin() + start + sz;

    ReadEncaps* oldEncaps = _currentReadEncaps;
    _currentReadEncaps = _currentReadEncaps->previous;
    if(oldEncaps == &_preAllocatedReadEncaps)
    {
	oldEncaps->reset();
    }
    else
    {
	delete oldEncaps;
    }
}

void
IceInternal::BasicStream::checkReadEncaps()
{
    assert(_currentReadEncaps);
    Container::size_type start = _currentReadEncaps->start;
    Int sz = _currentReadEncaps->sz;
    if(i != b.begin() + start + sz)
    {
        throw EncapsulationException(__FILE__, __LINE__);
    }
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
	throw NegativeSizeException(__FILE__, __LINE__);
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
#ifdef ICEE_BIG_ENDIAN
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
	throw NegativeSizeException(__FILE__, __LINE__);
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
	throw NegativeSizeException(__FILE__, __LINE__);
    }
    i += sz - sizeof(Int);
    if(i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
}

void
IceInternal::BasicStream::writeSize(Int v)
{
    assert(v >= 0);
    if(v > 254)
    {
	write(Byte(255));
	write(v);
    }
    else
    {
	write(static_cast<Byte>(v));
    }
}

void
IceInternal::BasicStream::readSize(Ice::Int& v)
{
    Byte byte;
    read(byte);
    unsigned val = static_cast<unsigned char>(byte);
    if(val == 255)
    {
	read(v);
	if(v < 0)
	{
	    throw NegativeSizeException(__FILE__, __LINE__);
	}
    }
    else
    {
	v = static_cast<Int>(static_cast<unsigned char>(byte));
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

void
IceInternal::BasicStream::writeBlob(const Ice::Byte* v, Container::size_type sz)
{
    if(sz > 0)
    {
	Container::size_type pos = b.size();
	resize(pos + sz);
	memcpy(&b[pos], v, sz);
    }
}

void
IceInternal::BasicStream::readBlob(Ice::Byte* v, Container::size_type sz)
{
    if(sz > 0)
    {
	if(static_cast<Container::size_type>(b.end() - i) < sz)
	{
	    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	memcpy(v, &*i, sz);
	i += sz;
    }
}

void
IceInternal::BasicStream::read(vector<Byte>& v)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	checkFixedSeq(sz, 1);
	vector<Byte>(i, i + sz).swap(v);
	i += sz;
    }
    else
    {
	v.clear();
    }
}

void
IceInternal::BasicStream::write(const vector<Byte>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	Container::size_type pos = b.size();
	resize(pos + sz);
	memcpy(&b[pos], &v[0], sz);
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

void
IceInternal::BasicStream::write(Short v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Short));
    Byte* dest = &b[pos];
#ifdef ICEE_BIG_ENDIAN
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
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Short);
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::write(const vector<Short>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	Container::size_type pos = b.size();
	resize(pos + sz * sizeof(Short));
#ifdef ICEE_BIG_ENDIAN
	const Byte* src = reinterpret_cast<const Byte*>(&v[0]) + sizeof(Short) - 1;
	Byte* dest = &(*(b.begin() + pos));
	for(int j = 0 ; j < sz ; ++j)
	{
	    *dest++ = *src--;
	    *dest++ = *src--;
	    src += 2 * sizeof(Short);
	}
#else
	memcpy(&b[pos], reinterpret_cast<const Byte*>(&v[0]), sz * sizeof(Short));
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
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::write(Int v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Int));
    Byte* dest = &b[pos];
#ifdef ICEE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Int) - 1;
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
IceInternal::BasicStream::read(Int& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Int)))
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Int);
#ifdef ICEE_BIG_ENDIAN
    Byte* dest = reinterpret_cast<Byte*>(&v) + sizeof(Int) - 1;
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
IceInternal::BasicStream::write(const vector<Int>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	Container::size_type pos = b.size();
	resize(pos + sz * sizeof(Int));
#ifdef ICEE_BIG_ENDIAN
	const Byte* src = reinterpret_cast<const Byte*>(&v[0]) + sizeof(Int) - 1;
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
	memcpy(&b[pos], reinterpret_cast<const Byte*>(&v[0]), sz * sizeof(Int));
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
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::write(Long v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Long));
    Byte* dest = &b[pos];
#ifdef ICEE_BIG_ENDIAN
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
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Long);
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::write(const vector<Long>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	Container::size_type pos = b.size();
	resize(pos + sz * sizeof(Long));
#ifdef ICEE_BIG_ENDIAN
	const Byte* src = reinterpret_cast<const Byte*>(&v[0]) + sizeof(Long) - 1;
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
	memcpy(&b[pos], reinterpret_cast<const Byte*>(&v[0]), sz * sizeof(Long));
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
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::write(Float v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Float));
    Byte* dest = &b[pos];
#ifdef ICEE_BIG_ENDIAN
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
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Float);
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::write(const vector<Float>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	Container::size_type pos = b.size();
	resize(pos + sz * sizeof(Float));
#ifdef ICEE_BIG_ENDIAN
	const Byte* src = reinterpret_cast<const Byte*>(&v[0]) + sizeof(Float) - 1;
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
	memcpy(&b[pos], reinterpret_cast<const Byte*>(&v[0]), sz * sizeof(Float));
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
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::write(Double v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Double));
    Byte* dest = &b[pos];
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::read(Double& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Double)))
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const Byte* src = &(*i);
    i += sizeof(Double);
#ifdef ICEE_BIG_ENDIAN
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
IceInternal::BasicStream::write(const vector<Double>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	Container::size_type pos = b.size();
	resize(pos + sz * sizeof(Double));
#ifdef ICEE_BIG_ENDIAN
	const Byte* src = reinterpret_cast<const Byte*>(&v[0]) + sizeof(Double) - 1;
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
#else
	memcpy(&b[pos], reinterpret_cast<const Byte*>(&v[0]), sz * sizeof(Double));
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
#ifdef ICEE_BIG_ENDIAN
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
#else
	copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
    else
    {
	v.clear();
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
IceInternal::BasicStream::write(const string& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	Container::size_type pos = b.size();
	resize(pos + sz);
	memcpy(&b[pos], v.c_str(), sz);
    }
}

void
IceInternal::BasicStream::write(const vector<string>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	vector<string>::const_iterator p;
	for(p = v.begin(); p != v.end(); ++p)
	{
	    write(*p);
	}
    }
}

void
IceInternal::BasicStream::read(string& v)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	if(b.end() - i < sz)
	{
	    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	string(reinterpret_cast<const char*>(&*i), reinterpret_cast<const char*>(&*i) + sz).swap(v);
//	v.assign(reinterpret_cast<const char*>(&(*i)), sz);
	i += sz;
    }
    else
    {
        v.clear();
    }
}

void
IceInternal::BasicStream::read(vector<string>& v)
{
    Int sz;
    readSize(sz);
    if(sz > 0)
    {
	startSeq(sz, 1);
	v.resize(sz);
	for(int i = 0; i < sz; ++i)
	{
	    read(v[i]);
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
    read(id);
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
	    read(id); // Read type id for next slice.
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
IceInternal::BasicStream::throwUnmarshalOutOfBoundsException(const char* file, int line)
{
    throw UnmarshalOutOfBoundsException(file, line);
}

void
IceInternal::BasicStream::throwMemoryLimitException(const char* file, int line)
{
    throw MemoryLimitException(file, line);
}

IceInternal::BasicStream::SeqData::SeqData(int num, int sz) : numElements(num), minSize(sz)
{
}
