// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Stream.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ValueFactory.h>
#include <Ice/ValueFactoryManager.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

static const Byte stringEncodingNormal = 0;
static const Byte stringEncodingRedirect = 1;

IceInternal::Stream::Stream(const InstancePtr& instance) :
    _instance(instance),
    _stringSet(CmpPosPos(b))
{
}

InstancePtr
IceInternal::Stream::instance() const
{
    return _instance;
}

void
IceInternal::Stream::swap(Stream& other)
{
    assert(_instance.get() == other._instance.get());

    b.swap(other.b);
    std::swap(i, other.i);
    _encapsStartStack.swap(other._encapsStartStack);
    _stringSet.swap(other._stringSet);
}

void
IceInternal::Stream::resize(int total)
{
    if (total > 1024 * 1024) // TODO: configurable
    {
	throw ::Ice::MemoryLimitException(__FILE__, __LINE__);
    }
    b.resize(total);
}

void
IceInternal::Stream::reserve(int total)
{
    if (total > 1024 * 1024) // TODO: configurable
    {
	throw ::Ice::MemoryLimitException(__FILE__, __LINE__);
    }
    b.reserve(total);
}

void
IceInternal::Stream::startWriteEncaps()
{
    write(Byte(0)); // Encoding version
    write(Int(0)); // Placeholder for the encapsulation length
    _encapsStartStack.push_back(b.size());
}

void
IceInternal::Stream::endWriteEncaps()
{
    int start = _encapsStartStack.back();
    _encapsStartStack.pop_back();
    Int sz = b.size() - start;
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIGENDIAN
    reverse_copy(p, p + sizeof(Int), b.begin() + start - sizeof(Int));
#else
    copy(p, p + sizeof(Int), b.begin() + start - sizeof(Int));
#endif
}

void
IceInternal::Stream::startReadEncaps()
{
    //
    // If in the future several encoding versions are supported, we
    // need a pushEncoding() and popEncoding() operation.
    //
    Byte encVer;
    read(encVer);
    if (encVer != 0)
    {
	throw UnsupportedEncodingException(__FILE__, __LINE__);
    }
    Int sz;
    read(sz);
    _encapsStartStack.push_back(i - b.begin());
}

void
IceInternal::Stream::endReadEncaps()
{
    int start = _encapsStartStack.back();
    _encapsStartStack.pop_back();
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
IceInternal::Stream::skipEncaps()
{
    Byte encVer;
    read(encVer);
    Int sz;
    read(sz);
    i += sz;
    if (i >= b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
}

void
IceInternal::Stream::write(const vector<Byte>& v)
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
IceInternal::Stream::read(Byte& v)
{
    if (i >= b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v = *i++;
}

void
IceInternal::Stream::read(vector<Byte>& v)
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
IceInternal::Stream::write(const vector<bool>& v)
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
IceInternal::Stream::read(bool& v)
{
    if (i >= b.end())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    v = *i++;
}

void
IceInternal::Stream::read(vector<bool>& v)
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
IceInternal::Stream::write(Short v)
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
IceInternal::Stream::write(const vector<Short>& v)
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
IceInternal::Stream::read(Short& v)
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
IceInternal::Stream::read(vector<Short>& v)
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
IceInternal::Stream::write(Int v)
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
IceInternal::Stream::write(const vector<Int>& v)
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
IceInternal::Stream::read(Int& v)
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
IceInternal::Stream::read(vector<Int>& v)
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
IceInternal::Stream::write(Long v)
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
IceInternal::Stream::write(const vector<Long>& v)
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
IceInternal::Stream::read(Long& v)
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
IceInternal::Stream::read(vector<Long>& v)
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
IceInternal::Stream::write(Float v)
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
IceInternal::Stream::write(const vector<Float>& v)
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
IceInternal::Stream::read(Float& v)
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
IceInternal::Stream::read(vector<Float>& v)
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
IceInternal::Stream::write(Double v)
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
IceInternal::Stream::write(const vector<Double>& v)
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
IceInternal::Stream::read(Double& v)
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
IceInternal::Stream::read(vector<Double>& v)
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
IceInternal::Stream::write(const string& v)
{
    pair<multiset<int, CmpPosPos>::const_iterator, multiset<int, CmpPosPos>::const_iterator> p =
	equal_range(_stringSet.begin(), _stringSet.end(), v, CmpPosString(b));
    if (p.first != p.second)
    {
	write(stringEncodingRedirect);
	Int diff = b.size() - *p.first;
	write(diff);
    }
    else
    {
	write(stringEncodingNormal);
	int pos = b.size();
	resize(pos + v.size() + 1);
	copy(v.begin(), v.end(), b.begin() + pos);
	b.back() = 0;
	_stringSet.insert(pos);
    }
}

void
IceInternal::Stream::write(const char* v)
{
    write(stringEncodingNormal);
    int pos = b.size();
    int len = strlen(v);
    resize(pos + len + 1);
    copy(v, v + len + 1, b.begin() + pos);
    _stringSet.insert(pos);
}

void
IceInternal::Stream::write(const vector<string>& v)
{
    write(Ice::Int(v.size()));
    vector<string>::const_iterator p;
    for (p = v.begin(); p != v.end(); ++p)
    {
	write(*p);
    }
}

void
IceInternal::Stream::read(string& v)
{
    Byte stringEnc;
    read(stringEnc);
    
    switch (stringEnc)
    {
	case stringEncodingNormal:
	{
	    Container::iterator begin = i;
	    do
	    {
		if (i >= b.end())
		{
		    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
		}
	    }
	    while (*i++);
	    v = begin;
	    _stringSet.insert(begin - b.begin());
	    break;
	}
	
	case stringEncodingRedirect:
	{
	    Int diff;
	    read(diff);
	    Container::iterator j = i - 4 - diff;
	    Container::iterator begin = j;
	    if (j < b.begin())
	    {
		throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
	    }
	    
	    do
	    {
		if (j >= b.end())
		{
		    throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
		}
	    }
	    while (*j++);
	    v = begin;
	    break;
	}
	
	default:
	{
	    throw StringEncodingException(__FILE__, __LINE__);
	}
    }
}

void
IceInternal::Stream::read(vector<string>& v)
{
    Ice::Int sz;
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
IceInternal::Stream::write(const wstring& v)
{
    write(stringEncodingNormal);
    wstring::const_iterator p;
    for (p = v.begin(); p != v.end(); ++p)
    {
	write(static_cast<Short>(*p));
    }
    write(Short(0));
}

void
IceInternal::Stream::write(const vector<wstring>& v)
{
    write(Ice::Int(v.size()));
    vector<wstring>::const_iterator p;
    for (p = v.begin(); p != v.end(); ++p)
    {
	write(*p);
    }
}

void
IceInternal::Stream::read(wstring& v)
{
    Byte stringEnc;
    read(stringEnc);
    if (stringEnc != stringEncodingNormal)
    {
	throw StringEncodingException(__FILE__, __LINE__);
    }
    // TODO: This can be optimized
    while (true)
    {
	Short s;
	read(s);
	if (!s)
	    break;
	v += static_cast<wchar_t>(s);
    }
}

void
IceInternal::Stream::read(vector<wstring>& v)
{
    Ice::Int sz;
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
IceInternal::Stream::write(const ::Ice::ObjectPrx& v)
{
    _instance->proxyFactory()->proxyToStream(v, this);
}

void
IceInternal::Stream::read(::Ice::ObjectPrx& v)
{
    v = _instance->proxyFactory()->streamToProxy(this);
}

void
IceInternal::Stream::write(const ObjectPtr& v)
{
    const string* classIds = v->_classIds();
    Ice::Int sz = 0;
    while (classIds[sz] != "::Ice::Object")
    {
	++sz;
    }
    write(sz);
    for (int i = 0; i < sz; i++)
    {
	write(classIds[i]);
    }
    v->__write(this);
}

void
IceInternal::Stream::read(ObjectPtr& v, const string& signatureType)
{
    vector<string> classIds;
    read(classIds);
    classIds.push_back("::Ice::Object");
    vector<string>::const_iterator p;
    for (p = classIds.begin(); p != classIds.end(); ++p)
    {
	ValueFactoryPtr factory = _instance->valueFactoryManager()->lookup(*p);
	
	if (factory)
	{
	    v = factory->create(*p);
	    v->__read(this);
	    
	    for (; p != classIds.end(); ++p)
	    {
		if (*p == signatureType)
		{
		    return;
		}
	    }
	    
	    throw ValueUnmarshalException(__FILE__, __LINE__);
	}
	
	if (*p == signatureType)
	{
	    return;
	}
	
	skipEncaps();
    }
    
    throw ValueUnmarshalException(__FILE__, __LINE__);
}

IceInternal::Stream::CmpPosPos::CmpPosPos(const Container& cont) :
    _cont(cont)
{
}

IceInternal::Stream::CmpPosPos::CmpPosPos(const CmpPosPos& cmp) :
    _cont(cmp._cont)
{
}

IceInternal::Stream::CmpPosPos&
IceInternal::Stream::CmpPosPos::operator=(const CmpPosPos&)
{
    // Do *not* assign anything! I want CmpPosPos to ignore std::swap().
    return *this;
}

bool
IceInternal::Stream::CmpPosPos::operator()(int p, int q) const
{
    assert(!_cont.empty());
    return strcmp(_cont.begin() + p, _cont.begin() + q) < 0;
}

IceInternal::Stream::CmpPosString::CmpPosString(const Container& cont) :
    _cont(cont)
{
}

IceInternal::Stream::CmpPosString::CmpPosString(const CmpPosString& cmp) :
    _cont(cmp._cont)
{
}

IceInternal::Stream::CmpPosString&
IceInternal::Stream::CmpPosString::operator=(const CmpPosString&)
{
    // Do *not* assign anything! I want CmpPosString to ignore std::swap().
    return *this;
}

bool
IceInternal::Stream::CmpPosString::operator()(int i, const string& s) const
{
    assert(!_cont.empty());
    return _cont.begin() + i < s;
}

bool
IceInternal::Stream::CmpPosString::operator()(const string& s, int i) const
{
    assert(!_cont.empty());
    return s < _cont.begin() + i;
}
