// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/LocalObject.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(LocalObject* p) { p->__incRef(); }
void IceInternal::decRef(LocalObject* p) { p->__decRef(); }

Ice::LocalObjectPtrE::LocalObjectPtrE(const LocalObjectPtrE& p) :
    _ptr(p._ptr)
{
}

Ice::LocalObjectPtrE::LocalObjectPtrE(const LocalObjectPtr& p) :
    _ptr(p)
{
}

Ice::LocalObjectPtrE::operator LocalObjectPtr() const
{
    return LocalObjectPtr(dynamic_cast<LocalObject*>(_ptr.get()));
}

::Ice::LocalObject*
Ice::LocalObjectPtrE::operator->() const
{
    return _ptr.get();
}

Ice::LocalObjectPtrE::operator bool() const
{
    return _ptr.get() ? true : false;
}

Ice::LocalObject::LocalObject()
{
}

Ice::LocalObject::~LocalObject()
{
}
