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

#include <Ice/Proxy.h>

namespace __Ice
{

template<typename T>
class ProxyHandle : public Handle<T>
{
public:
    
    ProxyHandle(T* p = 0) : Handle<T>(p) { }
    ProxyHandle(const ProxyHandle& r) : Handle<T>(r) { }

    ProxyHandle& operator=(const ProxyHandle& r)
    {
	Handle<T>::operator=(r);
	return *this;
    }

    static ProxyHandle<T> checkedCast(::__IceProxy::Ice::Object* from)
    {
	T* to;
	::__Ice::checkedCast(from, to);
	return ProxyHandle<T>(to);
    }

    static ProxyHandle<T> uncheckedCast(::__IceProxy::Ice::Object* from)
    {
	T* to;
	::__Ice::uncheckedCast(from, to);
	return ProxyHandle<T>(to);
    }
};

}

#endif
