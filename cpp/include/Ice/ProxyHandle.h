// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_HANDLE_H
#define ICE_PROXY_HANDLE_H

#include <Ice/Config.h>
#include <algorithm>

namespace IceInternal
{

//
// Like Handle<>, but specifically for proxies, with support for
// checkedCast() and uncheckedCast().
//
template<typename T>
class ProxyHandle
{
public:
    
    ProxyHandle(T* p = 0)
	: _ptr(p)
    {
	if (_ptr)
	    incRef(_ptr);
    }
    
    template<typename Y>
    ProxyHandle(const ProxyHandle<Y>& r)
	: _ptr(r._ptr)
    {
	if (_ptr)
	    incRef(_ptr);
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    ProxyHandle(const ProxyHandle<T>& r)
#else
    ProxyHandle(const ProxyHandle& r)
#endif
	: _ptr(r._ptr)
    {
	if (_ptr)
	    incRef(_ptr);
    }
    
    ~ProxyHandle()
    {
	if (_ptr)
	    decRef(_ptr);
    }
    
    ProxyHandle& operator=(T* p)
    {
	if (_ptr != p)
	{
	    if (p)
		incRef(p);

	    if (_ptr)
		decRef(_ptr);
	    
	    _ptr = p;
	}
	return *this;
    }
        
    template<typename Y>
    ProxyHandle& operator=(const ProxyHandle<Y>& r)
    {
	if (_ptr != r._ptr)
	{
	    if (r._ptr)
		incRef(r._ptr);

	    if (_ptr)
		decRef(_ptr);
	    
	    _ptr = r._ptr;
	}
	return *this;
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    ProxyHandle& operator=(const ProxyHandle<T>& r)
#else
    ProxyHandle& operator=(const ProxyHandle& r)
#endif
    {
	if (_ptr != r._ptr)
	{
	    if (r._ptr)
		incRef(r._ptr);

	    if (_ptr)
		decRef(_ptr);
	    
	    _ptr = r._ptr;
	}
	return *this;
    }
        
    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r)
    {
	T* p;
	::IceInternal::checkedCast(r._ptr, p);
	return ProxyHandle(p);
    }

    template<class Y>
    static ProxyHandle uncheckedCast(const ProxyHandle<Y>& r)
    {
	T* p;
	::IceInternal::uncheckedCast(r._ptr, p);
	return ProxyHandle(p);
    }

    typedef T element_type;
    
    T* get() const { return _ptr; }
    T* operator->() const { return _ptr; }
    operator bool() const { return _ptr ? true : false; }

    void swap(ProxyHandle& other) { std::swap(_ptr, other._ptr); }

#ifndef WIN32 // COMPILERBUG: VC++ 6.0 doesn't understand this
 
    template<typename Y> friend class ProxyHandle;

protected:

#endif

    T* _ptr;
};

template<typename T, typename U>
inline bool operator==(const ProxyHandle<T>& a, const ProxyHandle<U>& b)
{
    T* ap = a.get();
    T* bp = b.get();
    if (ap == bp)
    {
	return true;
    }
    else if (!ap)
    {
	assert(bp);
	return false;
    }
    else
    {
	return *ap == *bp;
    }
}

template<typename T, typename U>
inline bool operator<(const ProxyHandle<T>& a, const ProxyHandle<U>& b)
{
    T* ap = a.get();
    T* bp = b.get();
    if (!ap || !bp)
    {
	if (!ap && bp)
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }
    else
    {
	return *a.get() < *b.get();
    }
}

}

#endif
