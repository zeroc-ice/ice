// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_HANDLE_H
#define ICE_OBJECT_HANDLE_H

#include <Ice/StubF.h>

namespace __Ice
{

//
// Special handle class for Ice objects, which supports the static
// member function cast().
//
template<typename T>
class ObjectHandle : public Handle<T>
{
public:
    
    ObjectHandle(T* p = 0) : Handle<T>(p) { }
    ObjectHandle(const ObjectHandle& r) : Handle<T>(r) { }

    ObjectHandle& operator=(const ObjectHandle& r)
    {
	Handle<T>::operator=(r);
	return *this;
    }

    static ObjectHandle<T> cast(::__IceIntf::Ice::Object* from)
    {
	T* to;
	_cast(from, to);
	return ObjectHandle<T>(to);
    }
};

}

#endif
