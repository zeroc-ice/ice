// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_HANDLE_H
#define ICE_PROXY_HANDLE_H

#include <Ice/ProxyF.h>

namespace __Ice
{

//
// Special handle class for proxies, which supports the static cast
// operations.
//
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
	_checkedCast(from, to);
	return ProxyHandle<T>(to);
    }

    static ProxyHandle<T> uncheckedCast(::__IceProxy::Ice::Object* from)
    {
	T* to;
	_uncheckedCast(from, to);
	return ProxyHandle<T>(to);
    }
};

}

#endif
