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

#ifndef ICE_PROXY_HANDLE_H
#define ICE_PROXY_HANDLE_H

#include <IceUtil/Handle.h>
#include <Ice/Config.h>

namespace IceInternal
{

template<typename T> class ProxyHandle;

}

namespace IceProxy 
{ 
namespace Ice
{

class Object;

}
}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}

namespace IceInternal
{

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx&);

template<typename P> P 
uncheckedCastImpl(const ::Ice::ObjectPrx&);

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx&, const std::string&);

template<typename P> P
uncheckedCastImpl(const ::Ice::ObjectPrx&, const std::string&);

//
// Upcast
//
template<typename T, typename Y> inline ProxyHandle<T> 
checkedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, T*)
{
    return b;
}

template<typename T, typename Y> inline ProxyHandle<T> 
uncheckedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, T*)
{
    return b;
}

//
// Downcast
//
template<typename T, typename Y> inline ProxyHandle<T> 
checkedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, void*)
{
    return checkedCastImpl<ProxyHandle<T> >(b);
}

template<typename T, typename Y> inline ProxyHandle<T> 
uncheckedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, void*)
{
    return uncheckedCastImpl<ProxyHandle<T> >(b);
}


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
	this->_ptr = p;

	if(this->_ptr)
	{
	    incRef(this->_ptr);
	}
    }
    
    template<typename Y>
    ProxyHandle(const ProxyHandle<Y>& r)
    {
	this->_ptr = r._ptr;

	if(this->_ptr)
	{
	    incRef(this->_ptr);
	}
    }

    template<typename Y>
    ProxyHandle(const ::IceUtil::Handle<Y>& r)
    {
	this->_ptr = r._ptr;

	if(this->_ptr)
	{
	    incRef(this->_ptr);
	}
    }

#ifdef _WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    ProxyHandle(const ProxyHandle<T>& r)
#else
    ProxyHandle(const ProxyHandle& r)
#endif
    {
	this->_ptr = r._ptr;

	if(this->_ptr)
	{
	    incRef(this->_ptr);
	}
    }
    
    ~ProxyHandle()
    {
	if(this->_ptr)
	{
	    decRef(this->_ptr);
	}
    }
    
    ProxyHandle& operator=(T* p)
    {
	if(this->_ptr != p)
	{
	    if(p)
	    {
		incRef(p);
	    }

	    if(this->_ptr)
	    {
		decRef(this->_ptr);
	    }
	    
	    this->_ptr = p;
	}
	return *this;
    }
        
    template<typename Y>
    ProxyHandle& operator=(const ProxyHandle<Y>& r)
    {
	if(this->_ptr != r._ptr)
	{
	    if(r._ptr)
	    {
		incRef(r._ptr);
	    }

	    if(this->_ptr)
	    {
		decRef(this->_ptr);
	    }
	    
	    this->_ptr = r._ptr;
	}
	return *this;
    }

    template<typename Y>
    ProxyHandle& operator=(const ::IceUtil::Handle<Y>& r)
    {
	if(this->_ptr != r._ptr)
	{
	    if(r._ptr)
	    {
		incRef(r._ptr);
	    }

	    if(this->_ptr)
	    {
		decRef(this->_ptr);
	    }
	    
	    this->_ptr = r._ptr;
	}
	return *this;
    }

#ifdef _WIN32 // COMPILERBUG: Is VC++ or GNU C++ right here???
    template<>
    ProxyHandle& operator=(const ProxyHandle<T>& r)
#else
    ProxyHandle& operator=(const ProxyHandle& r)
#endif
    {
	if(this->_ptr != r._ptr)
	{
	    if(r._ptr)
	    {
		incRef(r._ptr);
	    }

	    if(this->_ptr)
	    {
		decRef(this->_ptr);
	    }
	    
	    this->_ptr = r._ptr;
	}
	return *this;
    }
        
    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r)
    {
	Y* tag = 0;
	return ::IceInternal::checkedCastHelper<T>(r, tag);
    }

    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r, const std::string& f)
    {
	return ::IceInternal::checkedCastImpl<ProxyHandle>(r, f);
    }

    template<class Y>
    static ProxyHandle uncheckedCast(const ProxyHandle<Y>& r)
    {
	Y* tag = 0;
	return::IceInternal::uncheckedCastHelper<T>(r, tag);
    }

    template<class Y>
    static ProxyHandle uncheckedCast(const ProxyHandle<Y>& r, const std::string& f)
    {
	return ::IceInternal::uncheckedCastImpl<ProxyHandle>(r, f);
    }
};

}

#endif
