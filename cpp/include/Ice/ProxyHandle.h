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

#include <IceUtil/Handle.h>
#include <Ice/Config.h>

namespace IceInternal
{

//
// Like IceInternal::Handle, but specifically for proxies, with
// support for checkedCast() and uncheckedCast() instead of
// dynamicCast().
//
template<typename T>
class ProxyHandle : public ::IceUtil::HandleBase<T>
{
public:
    
    ProxyHandle(T* p = 0)
    {
	_ptr = p;

	if (_ptr)
	{
	    incRef(_ptr);
	}
    }
    
    template<typename Y>
    ProxyHandle(const ProxyHandle<Y>& r)
    {
	_ptr = r._ptr;

	if (_ptr)
	{
	    incRef(_ptr);
	}
    }

    template<typename Y>
    ProxyHandle(const ::IceUtil::Handle<Y>& r)
    {
	_ptr = r._ptr;

	if (_ptr)
	{
	    incRef(_ptr);
	}
    }

#ifdef WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    ProxyHandle(const ProxyHandle<T>& r)
#else
    ProxyHandle(const ProxyHandle& r)
#endif
    {
	_ptr = r._ptr;

	if (_ptr)
	{
	    incRef(_ptr);
	}
    }
    
    ~ProxyHandle()
    {
	if (_ptr)
	{
	    decRef(_ptr);
	}
    }
    
    ProxyHandle& operator=(T* p)
    {
	if (_ptr != p)
	{
	    if (p)
	    {
		incRef(p);
	    }

	    if (_ptr)
	    {
		decRef(_ptr);
	    }
	    
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
	    {
		incRef(r._ptr);
	    }

	    if (_ptr)
	    {
		decRef(_ptr);
	    }
	    
	    _ptr = r._ptr;
	}
	return *this;
    }

    template<typename Y>
    ProxyHandle& operator=(const ::IceUtil::Handle<Y>& r)
    {
	if (_ptr != r._ptr)
	{
	    if (r._ptr)
	    {
		incRef(r._ptr);
	    }

	    if (_ptr)
	    {
		decRef(_ptr);
	    }
	    
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
	    {
		incRef(r._ptr);
	    }

	    if (_ptr)
	    {
		decRef(_ptr);
	    }
	    
	    _ptr = r._ptr;
	}
	return *this;
    }
        
    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r, const std::string& f = "")
    {
	ProxyHandle p;
	::IceInternal::checkedCast(r, f, p);
	return p;
    }

    template<class Y>
    static ProxyHandle uncheckedCast(const ProxyHandle<Y>& r, const std::string& f = "")
    {
	ProxyHandle p;
	::IceInternal::uncheckedCast(r, f, p);
	return p;
    }
};

}

#endif
