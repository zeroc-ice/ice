// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/BasicStream.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ObjectFactory.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/UserExceptionFactoryManager.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::BasicStream::BasicStream(const InstancePtr& instance) :
    _instance(instance),
    _currentReadEncaps(0),
    _currentWriteEncaps(0)
{
}

InstancePtr
IceInternal::BasicStream::instance() const
{
    return _instance;
}

void
IceInternal::BasicStream::swap(BasicStream& other)
{
    assert(_instance.get() == other._instance.get());

    b.swap(other.b);
    std::swap(i, other.i);
    _readEncapsStack.swap(other._readEncapsStack);
    _writeEncapsStack.swap(other._writeEncapsStack);
    std::swap(_currentReadEncaps, other._currentReadEncaps);
    std::swap(_currentWriteEncaps, other._currentWriteEncaps);
}

void
IceInternal::BasicStream::resize(int total)
{
    if (total > 1024 * 1024) // TODO: configurable
    {
	throw MemoryLimitException(__FILE__, __LINE__);
    }
    b.resize(total);
}

void
IceInternal::BasicStream::reserve(int total)
{
    if (total > 1024 * 1024) // TODO: configurable
    {
	throw MemoryLimitException(__FILE__, __LINE__);
    }
    b.reserve(total);
}

void
IceInternal::BasicStream::startWriteEncaps()
{
    write(Byte(0)); // Encoding
    write(Int(0)); // Placeholder for the encapsulation length
    _writeEncapsStack.resize(_writeEncapsStack.size() + 1);
    _currentWriteEncaps = &_writeEncapsStack.back();
    _currentWriteEncaps->start = b.size();
}

void
IceInternal::BasicStream::endWriteEncaps()
{
    assert(_currentWriteEncaps);
    int start = _currentWriteEncaps->start;
    _writeEncapsStack.pop_back();
    if (_writeEncapsStack.empty())
    {
	_currentWriteEncaps = 0;
    }
    else
    {
	_currentWriteEncaps = &_writeEncapsStack.back();
    }
    Int sz = b.size() - start;
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + start - sizeof(Int));
#else
    copy(p, p + sizeof(Int), b.begin() + start - sizeof(Int));
#endif
}

void
IceInternal::BasicStream::startReadEncaps()
{
    Byte encoding;
    read(encoding);
    if (encoding != 0)
    {
	throw UnsupportedEncodingException(__FILE__, __LINE__);
    }
    Int sz;
    read(sz);
    _readEncapsStack.resize(_readEncapsStack.size() + 1);
    _currentReadEncaps = &_readEncapsStack.back();
    _currentReadEncaps->encoding = encoding;
    _currentReadEncaps->start = i - b.begin();
}

void
IceInternal::BasicStream::endReadEncaps()
{
    assert(_currentReadEncaps);
    int start = _currentReadEncaps->start;
    _readEncapsStack.pop_back();
    if (_readEncapsStack.empty())
    {
	_currentReadEncaps = 0;
    }
    else
    {
	_currentReadEncaps = &_readEncapsStack.back();
    }
    i = b.begin() + start - sizeof(Int);
    Int sz;
    read(sz);
    i += sz;
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
}

void
IceInternal::BasicStream::checkReadEncaps()
{
    assert(_currentReadEncaps);
    int start = _currentReadEncaps->start;
    Container::iterator save = i;
    i = b.begin() + start - sizeof(Int);
    Int sz;
    read(sz);
    i = save;
    if (sz != i - (b.begin() + start))
    {
        throw EncapsulationException(__FILE__, __LINE__);
    }
}

Int
IceInternal::BasicStream::getReadEncapsSize()
{
    assert(_currentReadEncaps);
    int start = _currentReadEncaps->start;
    Container::iterator save = i;
    i = b.begin() + start - sizeof(Int);
    Int sz;
    read(sz);
    i = save;
    return sz;
}

void
IceInternal::BasicStream::skipEncaps()
{
    Byte encoding;
    read(encoding);
    if (encoding != 0)
    {
	throw UnsupportedEncodingException(__FILE__, __LINE__);
    }
    Int sz;
    read(sz);
    i += sz;
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
}

void
IceInternal::BasicStream::writeSize(Int v)
{
    if (v > 0x7f)
    {
	write(Byte(0xff));
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
    Byte b;
    read(b);
    if (b > Byte(0x7f))
    {
	read(v);
    }
    else
    {
	v = static_cast<Int>(b);     
    }
}

void
IceInternal::BasicStream::writeBlob(const vector<Byte>& v)
{
    int pos = b.size();
    resize(pos + v.size());
    copy(v.begin(), v.end(), b.begin() + pos);
}

void
IceInternal::BasicStream::readBlob(vector<Byte>& v, Int sz)
{
    Container::iterator begin = i;
    i += sz;
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
    copy(begin, i, v.begin());
}

void
IceInternal::BasicStream::write(const vector<Byte>& v)
{
    Int sz = v.size();
    writeSize(sz);
    int pos = b.size();
    resize(pos + sz);
    copy(v.begin(), v.end(), b.begin() + pos);
}

void
IceInternal::BasicStream::read(Byte& v)
{
    if (i >= b.end())
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
    Container::iterator begin = i;
    i += sz;
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
    copy(begin, i, v.begin());
}

void
IceInternal::BasicStream::write(const vector<bool>& v)
{
    Int sz = v.size();
    writeSize(sz);
    int pos = b.size();
    resize(pos + sz);
    copy(v.begin(), v.end(), b.begin() + pos);
}

void
IceInternal::BasicStream::read(bool& v)
{
    if (i >= b.end())
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
    Container::iterator begin = i;
    i += sz;
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
    copy(begin, i, v.begin());
}

void
IceInternal::BasicStream::write(Short v)
{
    int pos = b.size();
    resize(pos + sizeof(Short));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(p, p + sizeof(Short), b.begin() + pos);
#else
    copy(p, p + sizeof(Short), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Short>& v)
{
    Int sz = v.size();
    writeSize(sz);
    int pos = b.size();
    resize(pos + sz * sizeof(Short));
    const Byte* p = reinterpret_cast<const Byte*>(v.begin());
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Short), b.begin() + pos);
	p += sizeof(Short);
	pos += sizeof(Short);
    }
#else
    copy(p, p + sz * sizeof(Short), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::read(Short& v)
{
    Container::iterator begin = i;
    i += sizeof(Short);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Short>& v)
{
    Int sz;
    readSize(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Short);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(begin, begin + sizeof(Short), reinterpret_cast<Byte*>(&v[j]));
	begin += sizeof(Short);
    }
#else
    copy(begin, i, reinterpret_cast<Byte*>(v.begin()));
#endif
}

void
IceInternal::BasicStream::write(Int v)
{
    int pos = b.size();
    resize(pos + sizeof(Int));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Int>& v)
{
    Int sz = v.size();
    writeSize(sz);
    int pos = b.size();
    resize(pos + sz * sizeof(Int));
    const Byte* p = reinterpret_cast<const Byte*>(v.begin());
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Int), b.begin() + pos);
	p += sizeof(Int);
	pos += sizeof(Int);
    }
#else
    copy(p, p + sz * sizeof(Int), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::read(Int& v)
{
    Container::iterator begin = i;
    i += sizeof(Int);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Int>& v)
{
    Int sz;
    readSize(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Int);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(begin, begin + sizeof(Int), reinterpret_cast<Byte*>(&v[j]));
	begin += sizeof(Int);
    }
#else
    copy(begin, i, reinterpret_cast<Byte*>(v.begin()));
#endif
}

void
IceInternal::BasicStream::write(Long v)
{
    int pos = b.size();
    resize(pos + sizeof(Long));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(p, p + sizeof(Long), b.begin() + pos);
#else
    copy(p, p + sizeof(Long), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Long>& v)
{
    Int sz = v.size();
    writeSize(sz);
    int pos = b.size();
    resize(pos + sz * sizeof(Long));
    const Byte* p = reinterpret_cast<const Byte*>(v.begin());
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Long), b.begin() + pos);
	p += sizeof(Long);
	pos += sizeof(Long);
    }
#else
    copy(p, p + sz * sizeof(Long), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::read(Long& v)
{
    Container::iterator begin = i;
    i += sizeof(Long);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Long>& v)
{
    Int sz;
    readSize(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Long);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(begin, begin + sizeof(Long), reinterpret_cast<Byte*>(&v[j]));
	begin += sizeof(Long);
    }
#else
    copy(begin, i, reinterpret_cast<Byte*>(v.begin()));
#endif
}

void
IceInternal::BasicStream::write(Float v)
{
    int pos = b.size();
    resize(pos + sizeof(Float));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(p, p + sizeof(Float), b.begin() + pos);
#else
    copy(p, p + sizeof(Float), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Float>& v)
{
    Int sz = v.size();
    writeSize(sz);
    int pos = b.size();
    resize(pos + sz * sizeof(Float));
    const Byte* p = reinterpret_cast<const Byte*>(v.begin());
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Float), b.begin() + pos);
	p += sizeof(Float);
	pos += sizeof(Float);
    }
#else
    copy(p, p + sz * sizeof(Float), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::read(Float& v)
{
    Container::iterator begin = i;
    i += sizeof(Float);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Float>& v)
{
    Int sz;
    readSize(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Float);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(begin, begin + sizeof(Float), reinterpret_cast<Byte*>(&v[j]));
	begin += sizeof(Float);
    }
#else
    copy(begin, i, reinterpret_cast<Byte*>(v.begin()));
#endif
}

void
IceInternal::BasicStream::write(Double v)
{
    int pos = b.size();
    resize(pos + sizeof(Double));
    const Byte* p = reinterpret_cast<const Byte*>(&v);
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(p, p + sizeof(Double), b.begin() + pos);
#else
    copy(p, p + sizeof(Double), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Double>& v)
{
    Int sz = v.size();
    writeSize(sz);
    int pos = b.size();
    resize(pos + sz * sizeof(Double));
    const Byte* p = reinterpret_cast<const Byte*>(v.begin());
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Double), b.begin() + pos);
	p += sizeof(Double);
	pos += sizeof(Double);
    }
#else
    copy(p, p + sz * sizeof(Double), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::read(Double& v)
{
    Container::iterator begin = i;
    i += sizeof(Double);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
#ifdef ICE_UTIL_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Double>& v)
{
    Int sz;
    readSize(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Double);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_UTIL_BIGENDIAN
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(begin, begin + sizeof(Double), reinterpret_cast<Byte*>(&v[j]));
	begin += sizeof(Double);
    }
#else
    copy(begin, i, reinterpret_cast<Byte*>(v.begin()));
#endif
}

void
IceInternal::BasicStream::write(const string& v)
{
    Int len = v.size();
    writeSize(len);
    if (len > 0)
    {
	int pos = b.size();
	resize(pos + len);
	copy(v.begin(), v.end(), b.begin() + pos);
    }
}

void
IceInternal::BasicStream::write(const vector<string>& v)
{
    writeSize(Int(v.size()));
    vector<string>::const_iterator p;
    for (p = v.begin(); p != v.end(); ++p)
    {
	write(*p);
    }
}

void
IceInternal::BasicStream::read(string& v)
{
    Int len;
    readSize(len);

    if (len <= 0)
    {
	v.erase();
    }
    else
    {
	Container::iterator begin = i;
	i += len;
	if (i > b.end())
	{
	    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v.resize(len);
	copy(begin, i, v.begin());
    }
}

void
IceInternal::BasicStream::read(vector<string>& v)
{
    Int sz;
    readSize(sz);
    // Don't use v.resize(sz) or v.reserve(sz) here, as it cannot be
    // checked whether sz is a reasonable value
    while (sz--)
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
    if (!_currentWriteEncaps) // Lazy initialization
    {
	_writeEncapsStack.resize(1);
	_currentWriteEncaps = &_writeEncapsStack.back();
    }

    map<ObjectPtr, Int>::const_iterator p = _currentWriteEncaps->objectsWritten.find(v.get());
    if (p != _currentWriteEncaps->objectsWritten.end())
    {
	write(p->second);
    }
    else
    {
	write(Int(-1));
	
	if (v)
	{
	    Int num = _currentWriteEncaps->objectsWritten.size();
	    _currentWriteEncaps->objectsWritten[v.get()] = num;
	    write(v->__getClassIds()[0]);
	    v->__write(this);
	}
	else
	{
	    static const string empty;
	    write(empty);
	}
    }
}

void
IceInternal::BasicStream::read(const string& signatureType, const ObjectFactoryPtr& factory, ObjectPtr& v)
{
    if (!_currentReadEncaps) // Lazy initialization
    {
	_readEncapsStack.resize(1);
	_currentReadEncaps = &_readEncapsStack.back();
    }

    Int pos;
    read(pos);
    
    if (pos >= 0)
    {
	if (static_cast<vector<ObjectPtr>::size_type>(pos) >= _currentReadEncaps->objectsRead.size())
	{
	    throw IllegalIndirectionException(__FILE__, __LINE__);
	}
	v = _currentReadEncaps->objectsRead[pos];
    }
    else
    {
	static const string iceObject("::Ice::Object");
	string id;
	read(id);

	if (id.empty())
	{
	    v = 0;
	    return;
	}
	else if (id == iceObject)
	{
	    v = new ::Ice::Object;
	}
	else
	{
	    ObjectFactoryPtr userFactory = _instance->servantFactoryManager()->find(id);
	    if (userFactory)
	    {
		v = userFactory->create(id);
	    }

	    if (!v && id == signatureType)
	    {
		assert(factory);
		v = factory->create(id);
		assert(v);
	    }
	    
	    if (!v)
	    {
		throw NoObjectFactoryException(__FILE__, __LINE__);
	    }
	}
	_currentReadEncaps->objectsRead.push_back(v.get());
	v->__read(this);
    }
}

void
IceInternal::BasicStream::write(const UserException& v)
{
    write(v.__getExceptionIds()[0]);
    v.__write(this);
}

Int
IceInternal::BasicStream::throwException(const string* throwsBegin, const string* throwsEnd)
{
    string id;
    read(id);
    UserExceptionFactoryPtr factory = _instance->userExceptionFactoryManager()->find(id);
	
    if (factory)
    {
	try
	{
	    factory->createAndThrow(id);
	}
	catch (UserException& ex)
	{
	    static const string userException("::Ice::UserException");
	    for (const string* p = ex.__getExceptionIds(); *p != userException != 0; ++p)
	    {
		if (binary_search(throwsBegin, throwsEnd, string(*p)))
		{
		    ex.__read(this);
		    ex.ice_throw();
		}
	    }
	
	    throw UnknownUserException(__FILE__, __LINE__);
	}
    }

    pair<const string*, const string*> p = equal_range(throwsBegin, throwsEnd, id);
    if (p.first != p.second)
    {
	return p.first - throwsBegin;
    }
    
    throw NoUserExceptionFactoryException(__FILE__, __LINE__);
}
