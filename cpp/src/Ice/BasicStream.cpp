// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/BasicStream.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ObjectFactory.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/FactoryTable.h>
#include <Ice/TraceUtil.h>

template<typename InputIter, typename OutputIter>
void
ice_copy(InputIter first, InputIter last, OutputIter result)
{
    std::copy(first, last, result);
}

template<>
void
ice_copy(std::vector<Ice::Byte>::const_iterator first, std::vector<Ice::Byte>::const_iterator last,
	 std::vector<Ice::Byte>::iterator result)
{
    if(last != first)
    {
	memcpy(&*result, &*first, last - first);
    }
}

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::BasicStream::BasicStream(Instance* instance) :
    _instance(instance),
    _currentReadEncaps(0),
    _currentWriteEncaps(0)
{
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

    b.swap(other.b);
    std::swap(i, other.i);
    _readEncapsStack.swap(other._readEncapsStack);
    _writeEncapsStack.swap(other._writeEncapsStack);
    std::swap(_currentReadEncaps, other._currentReadEncaps);
    std::swap(_currentWriteEncaps, other._currentWriteEncaps);
}

void inline
inlineResize(Buffer* buffer, size_t total)
{
    if(total > 1024 * 1024) // TODO: configurable.
    {
	throw MemoryLimitException(__FILE__, __LINE__);
    }

    size_t capacity = buffer->b.capacity();
    if(capacity < total)
    {
	buffer->b.reserve(max(total, 2 * capacity));
    }

    buffer->b.resize(total);
}

void
IceInternal::BasicStream::resize(int total)
{
    inlineResize(this, total);
}

void
IceInternal::BasicStream::reserve(int total)
{
    if(total > 1024 * 1024) // TODO: configurable.
    {
	throw MemoryLimitException(__FILE__, __LINE__);
    }

    b.reserve(total);
}

IceInternal::BasicStream::WriteEncaps::WriteEncaps()
    : writeIndex(0), toBeMarshaledMap(0), marshaledMap(0), typeIdIndex(0), typeIdMap(0)
{
}

IceInternal::BasicStream::WriteEncaps::~WriteEncaps()
{
    if(toBeMarshaledMap)
    {
	delete toBeMarshaledMap;
	delete marshaledMap;
	delete typeIdMap;
    }
}

void
IceInternal::BasicStream::startWriteEncaps()
{
    {
	_writeEncapsStack.push_back(WriteEncaps());
	_currentWriteEncaps = &_writeEncapsStack.back();
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
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + start);
#else
    ice_copy(p, p + sizeof(Int), b.begin() + start);
#endif

    {
	_writeEncapsStack.pop_back();
	if(_writeEncapsStack.empty())
	{
	    _currentWriteEncaps = 0;
	}
	else
	{
	    _currentWriteEncaps = &_writeEncapsStack.back();
	}
    }
}

IceInternal::BasicStream::ReadEncaps::ReadEncaps()
    : patchMap(0), unmarshaledMap(0), typeIdIndex(0), typeIdMap(0)
{
}

IceInternal::BasicStream::ReadEncaps::~ReadEncaps()
{
    if(patchMap)
    {
	delete patchMap;
	delete unmarshaledMap;
	delete typeIdMap;
    }
}

void
IceInternal::BasicStream::startReadEncaps()
{
    {
	_readEncapsStack.push_back(ReadEncaps());
	_currentReadEncaps = &_readEncapsStack.back();
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

    _readEncapsStack.pop_back();
    if(_readEncapsStack.empty())
    {
	_currentReadEncaps = 0;
    }
    else
    {
	_currentReadEncaps = &_readEncapsStack.back();
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
    return _currentReadEncaps->sz - sizeof(Int) - 2;
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
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + _writeSlice - sizeof(Int));
#else
    copy(p, p + sizeof(Int), b.begin() + _writeSlice - sizeof(Int));
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
BasicStream::writeTypeId(const string& id)
{
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
	write(id);
    }
}

void
BasicStream::readTypeId(string& id)
{
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
	read(id);
	_currentReadEncaps->typeIdMap->insert(make_pair(++_currentReadEncaps->typeIdIndex, id));
    }
}

void
IceInternal::BasicStream::writeBlob(const vector<Byte>& v)
{
    size_t pos = b.size();
    inlineResize(this, pos + v.size());
    ice_copy(v.begin(), v.end(), b.begin() + pos);
}

void
IceInternal::BasicStream::readBlob(vector<Byte>& v, Int sz)
{
    if(b.end() - i < sz)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += sz;
    v.resize(sz);
    ice_copy(begin, i, v.begin());
}

void
IceInternal::BasicStream::writeBlob(const Ice::Byte* v, size_t len)
{
    size_t pos = b.size();
    inlineResize(this, pos + len);
    ice_copy(&v[0], &v[0 + len], b.begin() + pos);
}

void
IceInternal::BasicStream::readBlob(Ice::Byte* v, size_t len)
{
    if(static_cast<size_t>(b.end() - i) < len)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += len;
    ice_copy(begin, i, &v[0]);
}

void
IceInternal::BasicStream::write(const vector<Byte>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    size_t pos = b.size();
    inlineResize(this, pos + sz);
    ice_copy(v.begin(), v.end(), b.begin() + pos);
}

void
IceInternal::BasicStream::read(Byte& v)
{
    if(i >= b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v = *i++;
}

void
IceInternal::BasicStream::read(vector<Byte>& v)
{
    Int sz;
    readSize(sz);
    if(b.end() - i < sz)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += sz;
    v.resize(sz);
    ice_copy(begin, i, v.begin());
}

void
IceInternal::BasicStream::write(const vector<bool>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    size_t pos = b.size();
    inlineResize(this, pos + sz);
    ice_copy(v.begin(), v.end(), b.begin() + pos);
}

void
IceInternal::BasicStream::read(bool& v)
{
    if(i >= b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v = *i++;
}

void
IceInternal::BasicStream::read(vector<bool>& v)
{
    Int sz;
    readSize(sz);
    if(b.end() - i < sz)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += sz;
    v.resize(sz);
    ice_copy(begin, i, v.begin());
}

void
IceInternal::BasicStream::write(Short v)
{
    size_t pos = b.size();
    inlineResize(this, pos + sizeof(Short));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Short), b.begin() + pos);
#else
    ice_copy(p, p + sizeof(Short), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Short>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	size_t pos = b.size();
	inlineResize(this, pos + sz * sizeof(Short));
	const Byte* p = reinterpret_cast<const Byte*>(&v[0]);
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(p, p + sizeof(Short), b.begin() + pos);
	    p += sizeof(Short);
	    pos += sizeof(Short);
	}
#else
	ice_copy(p, p + sz * sizeof(Short), b.begin() + pos);
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
    Container::iterator begin = i;
    i += sizeof(Short);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    ice_copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Short>& v)
{
    Int sz;
    readSize(sz);
    const int length = sz * static_cast<int>(sizeof(Short));
    if(b.end() - i < length)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += length;
    v.resize(sz);
    if(sz > 0)
    {
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(begin, begin + sizeof(Short), reinterpret_cast<Byte*>(&v[j]));
	    begin += sizeof(Short);
	}
#else
	ice_copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
}

void
IceInternal::BasicStream::write(Int v)
{
    size_t pos = b.size();
    inlineResize(this, pos + sizeof(Int));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
#else
    ice_copy(p, p + sizeof(Int), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Int>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	size_t pos = b.size();
	inlineResize(this, pos + sz * sizeof(Int));
	const Byte* p = reinterpret_cast<const Byte*>(&v[0]);
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
	    p += sizeof(Int);
	    pos += sizeof(Int);
	}
#else
	ice_copy(p, p + sz * sizeof(Int), b.begin() + pos);
#endif
    }
}

void
IceInternal::BasicStream::read(Int& v)
{
    if(b.end() - i < static_cast<int>(sizeof(Int)))
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += sizeof(Int);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    ice_copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Int>& v)
{
    Int sz;
    readSize(sz);
    const int length = sz * static_cast<int>(sizeof(Int));
    if(b.end() - i < length)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += length;
    v.resize(sz);
    if(sz > 0)
    {
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(begin, begin + sizeof(Int), reinterpret_cast<Byte*>(&v[j]));
	    begin += sizeof(Int);
	}
#else
	ice_copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
}

void
IceInternal::BasicStream::write(Long v)
{
    size_t pos = b.size();
    inlineResize(this, pos + sizeof(Long));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Long), b.begin() + pos);
#else
    ice_copy(p, p + sizeof(Long), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Long>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	size_t pos = b.size();
	inlineResize(this, pos + sz * sizeof(Long));
	const Byte* p = reinterpret_cast<const Byte*>(&v[0]);
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(p, p + sizeof(Long), b.begin() + pos);
	    p += sizeof(Long);
	    pos += sizeof(Long);
	}
#else
	ice_copy(p, p + sz * sizeof(Long), b.begin() + pos);
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
    Container::iterator begin = i;
    i += sizeof(Long);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    ice_copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Long>& v)
{
    Int sz;
    readSize(sz);
    const int length = sz * static_cast<int>(sizeof(Long));
    if(b.end() - i < length)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += length;
    v.resize(sz);
    if(sz > 0)
    {
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(begin, begin + sizeof(Long), reinterpret_cast<Byte*>(&v[j]));
	    begin += sizeof(Long);
	}
#else
	ice_copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
}

void
IceInternal::BasicStream::write(Float v)
{
    size_t pos = b.size();
    inlineResize(this, pos + sizeof(Float));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Float), b.begin() + pos);
#else
    ice_copy(p, p + sizeof(Float), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Float>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	size_t pos = b.size();
	inlineResize(this, pos + sz * sizeof(Float));
	const Byte* p = reinterpret_cast<const Byte*>(&v[0]);
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(p, p + sizeof(Float), b.begin() + pos);
	    p += sizeof(Float);
	    pos += sizeof(Float);
	}
#else
	ice_copy(p, p + sz * sizeof(Float), b.begin() + pos);
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
    Container::iterator begin = i;
    i += sizeof(Float);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    ice_copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Float>& v)
{
    Int sz;
    readSize(sz);
    const int length = sz * static_cast<int>(sizeof(Float));
    if(b.end() - i < length)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += length;
    v.resize(sz);
    if(sz > 0)
    {
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(begin, begin + sizeof(Float), reinterpret_cast<Byte*>(&v[j]));
	    begin += sizeof(Float);
	}
#else
	ice_copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
}

void
IceInternal::BasicStream::write(Double v)
{
    size_t pos = b.size();
    inlineResize(this, pos + sizeof(Double));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Double), b.begin() + pos);
#else
    ice_copy(p, p + sizeof(Double), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Double>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
	size_t pos = b.size();
	inlineResize(this, pos + sz * sizeof(Double));
	const Byte* p = reinterpret_cast<const Byte*>(&v[0]);
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(p, p + sizeof(Double), b.begin() + pos);
	    p += sizeof(Double);
	    pos += sizeof(Double);
	}
#else
	ice_copy(p, p + sz * sizeof(Double), b.begin() + pos);
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
    Container::iterator begin = i;
    i += sizeof(Double);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    ice_copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Double>& v)
{
    Int sz;
    readSize(sz);
    const int length = sz * static_cast<int>(sizeof(Double));
    if(b.end() - i < length)
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Container::iterator begin = i;
    i += length;
    v.resize(sz);
    if(sz > 0)
    {
#ifdef ICE_BIG_ENDIAN
	for(int j = 0 ; j < sz ; ++j)
	{
	    reverse_copy(begin, begin + sizeof(Double), reinterpret_cast<Byte*>(&v[j]));
	    begin += sizeof(Double);
	}
#else
	ice_copy(begin, i, reinterpret_cast<Byte*>(&v[0]));
#endif
    }
}

void
IceInternal::BasicStream::write(const string& v)
{
    Int len = static_cast<Int>(v.size());
    writeSize(len);
    if(len > 0)
    {
	size_t pos = b.size();
	inlineResize(this, pos + len);
	ice_copy(v.begin(), v.end(), b.begin() + pos);
    }
}

void
IceInternal::BasicStream::write(const vector<string>& v)
{
    writeSize(Int(v.size()));
    vector<string>::const_iterator p;
    for(p = v.begin(); p != v.end(); ++p)
    {
	write(*p);
    }
}

void
IceInternal::BasicStream::read(string& v)
{
    Int len;
    readSize(len);

    if(len <= 0)
    {
	v.erase();
    }
    else
    {
	if(b.end() - i < len)
	{
	    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	Container::iterator begin = i;
	i += len;
	v.resize(len);
	ice_copy(begin, i, v.begin());
    }
}

void
IceInternal::BasicStream::read(vector<string>& v)
{
    Int sz;
    readSize(sz);
    while(sz--)
    {
	string s;
	read(s);
	v.push_back(s);
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
    if(!_currentWriteEncaps) 			// Lazy initialization
    {
	_writeEncapsStack.push_back();
	_currentWriteEncaps = &_writeEncapsStack.back();
	_currentWriteEncaps->start = b.size();
	_currentWriteEncaps->toBeMarshaledMap = 0;
    }

    if(!_currentWriteEncaps->toBeMarshaledMap)	// Lazy initialization
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
		// We haven't seen this instance previously, create a new index, and
		// insert it into the to-be-marshaled map.
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
	write(0);	// Write null pointer
    }
}

void
IceInternal::BasicStream::read(PatchFunc patchFunc, void* patchAddr)
{
    if(!_currentReadEncaps) 		// Lazy initialization
    {
	_readEncapsStack.push_back();
	_currentReadEncaps = &_readEncapsStack.back();
    }

    if(!_currentReadEncaps->patchMap)	// Lazy initialization
    {
	_currentReadEncaps->patchMap = new PatchMap;
	_currentReadEncaps->unmarshaledMap = new IndexToPtrMap;
	_currentReadEncaps->typeIdMap = new TypeIdReadMap;
    }

    ObjectPtr v;

    Int index;
    read(index);

    if(index == 0)
    {
	patchFunc(patchAddr, v);	// Null Ptr
	return;
    }

    if(index < 0 && patchAddr)
    {
	PatchMap::iterator p = _currentReadEncaps->patchMap->find(-index);
	if(p == _currentReadEncaps->patchMap->end())
	{
	    //
	    // We have no outstanding instances to be patched for this index, so make a new entry
	    // in the patch map.
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
    assert(index > 0);

    while(true)
    {
	string id;
	readTypeId(id);
	if(id == Ice::Object::ice_staticId())
	{
	    v = new ::Ice::Object;
	}
	else
	{
	    ObjectFactoryPtr userFactory = _instance->servantFactoryManager()->find(id);
	    if(userFactory)
	    {
		v = userFactory->create(id);
	    }
	    if(!v)
	    {
		ObjectFactoryPtr of = Ice::factoryTable->getObjectFactory(id);
		if(of)
		{
		    v = of->create(id);
		    assert(v);
		}
	    }
	    if(!v)
	    {
//		traceSlicing("class", id, _instance->logger(), _instance->traceLevels());
		skipSlice();	// Slice off this derived part -- we don't understand it.
		continue;
	    }
	}

	IndexToPtrMap::const_iterator unmarshaledPos =
			    _currentReadEncaps->unmarshaledMap->insert(make_pair(index, v)).first;

	v->__read(this, false);
	patchPointers(index, unmarshaledPos, _currentReadEncaps->patchMap->end());
	return;
    }

    //
    // We can't possibly end up here: at the very least, the type ID "::Ice::Object" must be recognized, or
    // client and server were compiled with mismatched Slice definitions.
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
    read(id);
    while(!id.empty())
    {
	//
	// Look for a factory for this ID.
	//
	UserExceptionFactoryPtr factory = factoryTable->getExceptionFactory(id);
	if(factory)
	{
	    //
	    // Got factory -- get the factory to instantiate the exception, initialize the
	    // exception members, and throw the exception.
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
//	    traceSlicing("exception", id, _instance->logger(), _instance->traceLevels());
	    skipSlice();	// Slice off what we don't understand
	    read(id);		// Read type id for next slice
	}
    }
    //
    // Getting here should be impossible: we can get here only if the sender has marshaled a sequence
    // of type IDs, none of which we have factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    throw UnknownUserException(__FILE__, __LINE__);
}

void
BasicStream::writePendingObjects()
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
		// Add an instance from the old to-be-marshaled map to the marshaled map and then
		// ask the instance to marshal itself. Any new class instances that are triggered
		// by the classes marshaled are added to toBeMarshaledMap.
		//
		_currentWriteEncaps->marshaledMap->insert(*p);
		writeInstance(p->first, p->second);
	    }

	    //
	    // We have marshaled all the instances for this pass, substract what we have
	    // marshaled from the toBeMarshaledMap.
	    //
	    PtrToIndexMap newMap;
	    set_difference(_currentWriteEncaps->toBeMarshaledMap->begin(), _currentWriteEncaps->toBeMarshaledMap->end(),
			   savedMap.begin(), savedMap.end(),
			   insert_iterator<PtrToIndexMap>(newMap, newMap.begin()));
	    *_currentWriteEncaps->toBeMarshaledMap = newMap;
	}
    }
    writeSize(0);	// Zero marker indicates end of sequence of sequences of instances.
}

void
BasicStream::readPendingObjects()
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
}

void
BasicStream::writeInstance(const ObjectPtr& v, Int index)
{
    write(index);
    v->__write(this);
}

void
BasicStream::patchPointers(Int index, IndexToPtrMap::const_iterator unmarshaledPos, PatchMap::iterator patchPos)
{
    //
    // Called whenever we have unmarshaled a new instance. The index is the index of the instance.
    // UnmarshaledPos denotes the instance just unmarshaled and patchPost denotes the patch map entry for
    // the index just unmarshaled. (Exactly one of these two iterators must be end().)
    // Patch any pointers in the patch map with the new address.
    //
    assert(   (unmarshaledPos != _currentReadEncaps->unmarshaledMap->end()
	       && patchPos == _currentReadEncaps->patchMap->end())
	   || (unmarshaledPos == _currentReadEncaps->unmarshaledMap->end()
	       && patchPos != _currentReadEncaps->patchMap->end())
	  );

    if(unmarshaledPos != _currentReadEncaps->unmarshaledMap->end())
    {
	//
	// We have just unmarshaled an instance -- check if something needs patching for that instance.
	//
	patchPos = _currentReadEncaps->patchMap->find(index);
	if(patchPos == _currentReadEncaps->patchMap->end())
	{
	    return;	// We don't have anything to patch for the instance just unmarshaled
	}
    }
    else
    {
	//
	// We have just unmarshaled an index -- check if we have unmarshaled the instance for that index yet.
	//
	unmarshaledPos = _currentReadEncaps->unmarshaledMap->find(index);
	if(unmarshaledPos == _currentReadEncaps->unmarshaledMap->end())
	{
	    return;	// We haven't unmarshaled the instance yet
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
    // Clear out the patch map for that index -- there is nothing left to patch for that
    // index for the time being.
    //
    _currentReadEncaps->patchMap->erase(patchPos);
}
