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
    _encapsStack(1)
{
    _encapsStack.resize(1);
    _encapsStack.back().start = 0;
    _encapsStack.back().encoding = 0;
}

IceInternal::BasicStream::~BasicStream()
{
    //
    // No check for exactly one, because an error might have aborted
    // marshalling/unmarshalling
    //
    assert(_encapsStack.size() > 0);
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
    _encapsStack.swap(other._encapsStack);
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
    write(Int(0)); // Placeholder for the encapsulation length
    _encapsStack.resize(_encapsStack.size() + 1);
    _encapsStack.back().start = b.size();
    _encapsStack.back().encoding = 0;
    write(_encapsStack.back().encoding);
}

void
IceInternal::BasicStream::endWriteEncaps()
{
    int start = _encapsStack.back().start;
    _encapsStack.pop_back();
    Int sz = b.size() - start;
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + start - sizeof(Int));
#else
    copy(p, p + sizeof(Int), b.begin() + start - sizeof(Int));
#endif
}

void
IceInternal::BasicStream::startReadEncaps()
{
    Int sz;
    read(sz);
    _encapsStack.resize(_encapsStack.size() + 1);
    _encapsStack.back().start = i - b.begin();
    read(_encapsStack.back().encoding);
    if (_encapsStack.back().encoding != 0)
    {
	throw UnsupportedEncodingException(__FILE__, __LINE__);
    }
}

void
IceInternal::BasicStream::endReadEncaps()
{
    int start = _encapsStack.back().start;
    _encapsStack.pop_back();
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

void
IceInternal::BasicStream::skipEncaps()
{
    Int sz;
    read(sz);
    i += sz;
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
}

void
IceInternal::BasicStream::write(const vector<Byte>& v)
{
    int pos = b.size();
    Int sz = v.size();
    resize(pos + sizeof(Int) + sz);
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
#endif
    copy(v.begin(), v.end(), b.begin() + pos + sizeof(Int));
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
    read(sz);
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
    int pos = b.size();
    Int sz = v.size();
    resize(pos + sizeof(Int) + sz);
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
#endif
    copy(v.begin(), v.end(), b.begin() + pos + sizeof(Int));
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
    read(sz);
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
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Short), b.begin() + pos);
#else
    copy(p, p + sizeof(Short), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Short>& v)
{
    int pos = b.size();
    Int sz = v.size();
    resize(pos + sizeof(Int) + sz * sizeof(Short));
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
    pos += sizeof(Int);
    p = reinterpret_cast<const Byte*>(v.begin());
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Short), b.begin() + pos);
	p += sizeof(Short);
	pos += sizeof(Short);
    }
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
    p = reinterpret_cast<const Byte*>(v.begin());
    copy(p, p + sz * sizeof(Short), b.begin() + pos + sizeof(Int));
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
#ifdef ICE_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Short>& v)
{
    Int sz;
    read(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Short);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_BIGENDIAN
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
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Int>& v)
{
    int pos = b.size();
    Int sz = v.size();
    resize(pos + sizeof(Int) + sz * sizeof(Int));
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
    pos += sizeof(Int);
    p = reinterpret_cast<const Byte*>(v.begin());
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Short), b.begin() + pos);
	p += sizeof(Int);
	pos += sizeof(Int);
    }
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
    p = reinterpret_cast<const Byte*>(v.begin());
    copy(p, p + sz * sizeof(Int), b.begin() + pos + sizeof(Int));
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
#ifdef ICE_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Int>& v)
{
    Int sz;
    read(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Int);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_BIGENDIAN
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
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Long), b.begin() + pos);
#else
    copy(p, p + sizeof(Long), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Long>& v)
{
    int pos = b.size();
    Int sz = v.size();
    resize(pos + sizeof(Int) + sz * sizeof(Long));
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
    pos += sizeof(Int);
    p = reinterpret_cast<const Byte*>(v.begin());
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Long), b.begin() + pos);
	p += sizeof(Long);
	pos += sizeof(Long);
    }
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
    p = reinterpret_cast<const Byte*>(v.begin());
    copy(p, p + sz * sizeof(Long), b.begin() + pos + sizeof(Int));
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
#ifdef ICE_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Long>& v)
{
    Int sz;
    read(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Long);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_BIGENDIAN
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
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Float), b.begin() + pos);
#else
    copy(p, p + sizeof(Float), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Float>& v)
{
    int pos = b.size();
    Int sz = v.size();
    resize(pos + sizeof(Int) + sz * sizeof(Float));
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
    pos += sizeof(Int);
    p = reinterpret_cast<const Byte*>(v.begin());
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Float), b.begin() + pos);
	p += sizeof(Float);
	pos += sizeof(Float);
    }
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
    p = reinterpret_cast<const Byte*>(v.begin());
    copy(p, p + sz * sizeof(Float), b.begin() + pos + sizeof(Int));
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
#ifdef ICE_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Float>& v)
{
    Int sz;
    read(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Float);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_BIGENDIAN
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
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Double), b.begin() + pos);
#else
    copy(p, p + sizeof(Double), b.begin() + pos);
#endif
}

void
IceInternal::BasicStream::write(const vector<Double>& v)
{
    int pos = b.size();
    Int sz = v.size();
    resize(pos + sizeof(Int) + sz * sizeof(Double));
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + pos);
    pos += sizeof(Int);
    p = reinterpret_cast<const Byte*>(v.begin());
    for (int j = 0 ; j < sz ; ++j)
    {
	reverse_copy(p, p + sizeof(Double), b.begin() + pos);
	p += sizeof(Double);
	pos += sizeof(Double);
    }
#else
    copy(p, p + sizeof(Int), b.begin() + pos);
    p = reinterpret_cast<const Byte*>(v.begin());
    copy(p, p + sz * sizeof(Double), b.begin() + pos + sizeof(Int));
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
#ifdef ICE_BIGENDIAN
    reverse_copy(begin, i, reinterpret_cast<Byte*>(&v));
#else
    copy(begin, i, reinterpret_cast<Byte*>(&v));
#endif
}

void
IceInternal::BasicStream::read(vector<Double>& v)
{
    Int sz;
    read(sz);
    Container::iterator begin = i;
    i += sz * sizeof(Double);
    if (i > b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v.resize(sz);
#ifdef ICE_BIGENDIAN
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
    map<string, Int>::const_iterator p = _encapsStack.back().stringsWritten.find(v);
    if (p != _encapsStack.back().stringsWritten.end())
    {
	write(p->second);
    }
    else
    {
	Int len = v.size();
	write(len);
	if (len > 0)
	{
	    Int num = _encapsStack.back().stringsWritten.size();
	    _encapsStack.back().stringsWritten[v] = -(num + 1);
	    int pos = b.size();
	    resize(pos + len);
	    copy(v.begin(), v.end(), b.begin() + pos);
	}
    }
}

void
IceInternal::BasicStream::write(const char* v)
{
    write(string(v));
}

void
IceInternal::BasicStream::write(const vector<string>& v)
{
    write(Int(v.size()));
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
    read(len);

    if (len < 0)
    {
	if (static_cast<vector<string>::size_type>(-(len + 1)) >= _encapsStack.back().stringsRead.size())
	{
	    throw IllegalIndirectionException(__FILE__, __LINE__);
	}
	v = _encapsStack.back().stringsRead[-(len + 1)];
    }
    else
    {
	if (len == 0)
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
	    _encapsStack.back().stringsRead.push_back(v);
	}
    }
}

void
IceInternal::BasicStream::read(vector<string>& v)
{
    Int sz;
    read(sz);
    // Don't use v.resize(sz) or v.reserve(sz) here, as it cannot be
    // checked whether sz is a reasonable value
    while (sz--)
    {
#ifdef WIN32 // STLBUG
	v.push_back(string());
#else
	v.push_back();
#endif
	read(v.back());
    }
}

void
IceInternal::BasicStream::write(const wstring& v)
{
    map<wstring, Int>::const_iterator p = _encapsStack.back().wstringsWritten.find(v);
    if (p != _encapsStack.back().wstringsWritten.end())
    {
	write(p->second);
    }
    else
    {
	Int len = v.size();
	write(len);
	if (len > 0)
	{
	    Int num = _encapsStack.back().wstringsWritten.size();
	    _encapsStack.back().wstringsWritten[v] = -(num + 1);
	    wstring::const_iterator p;
	    for (p = v.begin(); p != v.end(); ++p)
	    {
		write(static_cast<Short>(*p));
	    }
	}
    }
}

void
IceInternal::BasicStream::write(const vector<wstring>& v)
{
    write(Int(v.size()));
    vector<wstring>::const_iterator p;
    for (p = v.begin(); p != v.end(); ++p)
    {
	write(*p);
    }
}

void
IceInternal::BasicStream::read(wstring& v)
{
    Int len;
    read(len);

    if (len < 0)
    {
	if (static_cast<vector<wstring>::size_type>(-(len + 1)) >= _encapsStack.back().wstringsRead.size())
	{
	    throw IllegalIndirectionException(__FILE__, __LINE__);
	}
	v = _encapsStack.back().wstringsRead[-(len + 1)];
    }
    else
    {
	v.erase();
	if (len > 0)
	{
	    while (len--)
	    {
		Short s;
		read(s);
		v += static_cast<wchar_t>(s);
	    }
	    _encapsStack.back().wstringsRead.push_back(v);
	}
    }
}

void
IceInternal::BasicStream::write(const wchar_t* v)
{
    write(wstring(v));
}

void
IceInternal::BasicStream::read(vector<wstring>& v)
{
    Int sz;
    read(sz);
    // Don't use v.resize(sz) or v.reserve(sz) here, as it cannot be
    // checked whether sz is a reasonable value
    while (sz--)
    {
#ifdef WIN32 // STLBUG
	v.push_back(wstring());
#else
	v.push_back();
#endif
	read(v.back());
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
    map<ObjectPtr, Int>::const_iterator p = _encapsStack.back().objectsWritten.find(v);
    if (p != _encapsStack.back().objectsWritten.end())
    {
	write(p->second);
    }
    else
    {
	write(Int(-1));
	
	if (v)
	{
	    Int num = _encapsStack.back().objectsWritten.size();
	    _encapsStack.back().objectsWritten[v] = num;
	    write(v->__getClassIds()[0]);
	    v->__write(this);
	}
	else
	{
	    write("");
	}
    }
}

bool
IceInternal::BasicStream::read(const char* signatureType, ObjectPtr& v)
{
    Int pos;
    read(pos);
    
    if (pos >= 0)
    {
	if (static_cast<vector<ObjectPtr>::size_type>(pos) >= _encapsStack.back().objectsRead.size())
	{
	    throw IllegalIndirectionException(__FILE__, __LINE__);
	}
	v = _encapsStack.back().objectsRead[pos];
	return true;
    }
    else
    {
	string id;
	read(id);

	if (id.empty())
	{
	    v = 0;
	    return true;
	}
	else
	{
	    ObjectFactoryPtr factory = _instance->servantFactoryManager()->find(id);
	    
	    if (factory)
	    {
		v = factory->create(id);
		if (v)
		{
		    _encapsStack.back().objectsRead.push_back(v);
		    v->__read(this);
		    return true;
		}
	    }
	    
	    if (id == signatureType)
	    {
		_encapsStack.back().objectsRead.push_back(v);
		return false;
	    }
	    
	    throw NoObjectFactoryException(__FILE__, __LINE__);
	}
    }
}

void
IceInternal::BasicStream::write(const UserException& v)
{
    write(v.__getExceptionIds()[0]);
    v.__write(this);
}

Int
IceInternal::BasicStream::throwException(const char** throwsBegin, const char** throwsEnd)
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
	    for (const char** p = ex.__getExceptionIds(); strcmp(*p, "::Ice::UserException") != 0; ++p)
	    {
		if (binary_search(throwsBegin, throwsEnd, string(*p)))
		{
		    ex.__read(this);
		    ex._throw();
		}
	    }
	
	    throw UnknownUserException(__FILE__, __LINE__);
	}
    }

    pair<const char**, const char**> p = equal_range(throwsBegin, throwsEnd, id);
    if (p.first != p.second)
    {
	return p.first - throwsBegin;
    }
    
    throw NoUserExceptionFactoryException(__FILE__, __LINE__);
}
