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
    
    typedef T element_type;
    
//    ProxyHandle() : Ptr(0) { }

    ProxyHandle(T* p = 0)
	: Ptr(p)
    {
	if(Ptr)
	    incRef(Ptr);
    }
    
    template<typename Y>
    ProxyHandle(const ProxyHandle<Y>& r)
	: Ptr(r.Ptr)
    {
	if(Ptr)
	    incRef(Ptr);
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    ProxyHandle(const ProxyHandle<T>& r)
#else
    ProxyHandle(const ProxyHandle& r)
#endif
	: Ptr(r.Ptr)
    {
	if(Ptr)
	    incRef(Ptr);
    }
    
    ~ProxyHandle()
    {
	if(Ptr)
	    decRef(Ptr);
    }
    
    ProxyHandle& operator=(T* p)
    {
	if(Ptr != p)
	{
	    if(p)
		incRef(p);

	    if(Ptr)
		decRef(Ptr);
	    
	    Ptr = p;
	}
	return *this;
    }
        
    template<typename Y>
    ProxyHandle& operator=(const ProxyHandle<Y>& r)
    {
	if(Ptr != r.Ptr)
	{
	    if(r.Ptr)
		incRef(r.Ptr);

	    if(Ptr)
		decRef(Ptr);
	    
	    Ptr = r.Ptr;
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
	if(Ptr != r.Ptr)
	{
	    if(r.Ptr)
		incRef(r.Ptr);

	    if(Ptr)
		decRef(Ptr);
	    
	    Ptr = r.Ptr;
	}
	return *this;
    }
        
    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r)
    {
	T* p;
	::IceInternal::checkedCast(r.Ptr, p);
	return ProxyHandle(p);
    }

    template<class Y>
    static ProxyHandle uncheckedCast(const ProxyHandle<Y>& r)
    {
	T* p;
	::IceInternal::uncheckedCast(r.Ptr, p);
	return ProxyHandle(p);
    }

    T* get() const { return Ptr; }

    T& operator*() const { return *Ptr; }
    T* operator->() const { return Ptr; }
    operator bool() const { return Ptr ? true : false; }

    void swap(ProxyHandle& other) { std::swap(Ptr, other.Ptr); }

#ifndef WIN32 // COMPILERBUG: VC++ 6.0 doesn't understand this
 
    template<typename Y> friend class ProxyHandle;

protected:

#endif

    T* Ptr;
};

template<typename T, typename U>
inline bool operator==(const ProxyHandle<T>& a, const ProxyHandle<U>& b)
{
    return *a.get() == *b.get();
}

template<typename T, typename U>
inline bool operator!=(const ProxyHandle<T>& a, const ProxyHandle<U>& b)
{
    return *a.get() != *b.get();
}

template<typename T, typename U>
inline bool operator<(const ProxyHandle<T>& a, const ProxyHandle<U>& b)
{
    return *a.get() < *b.get();
}

}

#endif
