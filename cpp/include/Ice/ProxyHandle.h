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

namespace __Ice
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
    
//    ProxyHandle() : ptr_(0) { }

    ProxyHandle(T* p = 0)
	: ptr_(p)
    {
	if(ptr_)
	    incRef(ptr_);
    }
    
    template<typename Y>
    ProxyHandle(const ProxyHandle<Y>& r)
	: ptr_(r.ptr_)
    {
	if(ptr_)
	    incRef(ptr_);
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    ProxyHandle(const ProxyHandle<T>& r)
#else
    ProxyHandle(const ProxyHandle& r)
#endif
	: ptr_(r.ptr_)
    {
	if(ptr_)
	    incRef(ptr_);
    }
    
    ~ProxyHandle()
    {
	if(ptr_)
	    decRef(ptr_);
    }
    
    ProxyHandle& operator=(T* p)
    {
	if(ptr_ != p)
	{
	    if(p)
		incRef(p);

	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = p;
	}
	return *this;
    }
        
    template<typename Y>
    ProxyHandle& operator=(const ProxyHandle<Y>& r)
    {
	if(ptr_ != r.ptr_)
	{
	    if(r.ptr_)
		incRef(r.ptr_);

	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = r.ptr_;
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
	if(ptr_ != r.ptr_)
	{
	    if(r.ptr_)
		incRef(r.ptr_);

	    if(ptr_)
		decRef(ptr_);
	    
	    ptr_ = r.ptr_;
	}
	return *this;
    }
        
    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r)
    {
	T* p;
	::__Ice::checkedCast(r.ptr_, p);
	return ProxyHandle(p);
    }

    template<class Y>
    static ProxyHandle uncheckedCast(const ProxyHandle<Y>& r)
    {
	T* p;
	::__Ice::checkedCast(r.ptr_, p);
	return ProxyHandle(p);
    }

    T* get() const { return ptr_; }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    operator bool() const { return ptr_ ? true : false; }

    void swap(ProxyHandle& other) { std::swap(ptr_, other.ptr_); }

#ifndef WIN32 // COMPILERBUG: VC++ 6.0 doesn't understand this
 
    template<typename Y> friend class ProxyHandle;

protected:

#endif

    T* ptr_;
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
