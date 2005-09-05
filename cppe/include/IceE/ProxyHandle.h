// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROXY_HANDLE_H
#define ICEE_PROXY_HANDLE_H

#include <IceE/Handle.h>
#include <IceE/Config.h>

namespace IceInternal
{

template<typename T> class ProxyHandle;
template<typename T> class Handle;

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

class ObjectAdapter;
typedef ::IceInternal::Handle< ::Ice::ObjectAdapter> ObjectAdapterPtr;

typedef ::std::map< ::std::string, ::std::string> Context;
}

namespace IceInternal
{

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx&);

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx&, const std::string&);

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx&, const ::Ice::Context&);

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx&, const std::string&, const ::Ice::Context&);

template<typename P> P 
uncheckedCastImpl(const ::Ice::ObjectPrx&);

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
checkedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, T*, const ::Ice::Context&)
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
checkedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, const ::Ice::Context& ctx)
{
    return checkedCastImpl<ProxyHandle<T> >(b, ctx);
}

template<typename T, typename Y> inline ProxyHandle<T> 
checkedCastHelper(const ::IceInternal::ProxyHandle<Y>& b, void*, const ::Ice::Context& ctx)
{
    return checkedCastImpl<ProxyHandle<T> >(b, ctx);
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

#ifdef _MSC_VER // COMPILERBUG: Is VC++ or GNU C++ right here???
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

#ifdef _MSC_VER // COMPILERBUG: Is VC++ or GNU C++ right here???
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
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r, const ::Ice::Context& ctx)
    {
	return ::IceInternal::checkedCastHelper<T>(r, ctx);
    }

    template<class Y>
    static ProxyHandle checkedCast(const ProxyHandle<Y>& r, const std::string& f, const ::Ice::Context& ctx)
    {
	return ::IceInternal::checkedCastImpl<ProxyHandle>(r, f, ctx);
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
