// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Object.h>
#include <Ice/Incoming.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Object* p) { p->__incRef(); }
void IceInternal::decRef(Object* p) { p->__decRef(); }

Ice::LocationForward::LocationForward(const LocationForward& p) :
    _prx(p._prx)
{
}

Ice::LocationForward::LocationForward(const ObjectPrx& p) :
    _prx(p)
{
}

Ice::ObjectPtrE::ObjectPtrE(const ObjectPtrE& p) :
    _ptr(p._ptr)
{
}

Ice::ObjectPtrE::ObjectPtrE(const ObjectPtr& p) :
    _ptr(p)
{
}

Ice::ObjectPtrE::operator ObjectPtr() const
{
    return _ptr;
}

::Ice::Object*
Ice::ObjectPtrE::operator->() const
{
    return _ptr.get();
}

Ice::ObjectPtrE::operator bool() const
{
    return _ptr.get() ? true : false;
}

Ice::Object::Object()
{
}

Ice::Object::~Object()
{
}

void
Ice::Object::_throw()
{
    throw ObjectPtrE(this);
}

bool
Ice::Object::_isA(const string& s)
{
    return s == "::Ice::Object";
}

void
Ice::Object::_ping()
{
}

DispatchStatus
Ice::Object::____isA(Incoming& __in)
{
    Stream* __is = __in.is();
    Stream* __os = __in.os();
    string s;
    __is->read(s);
    bool __ret = _isA(s);
    __os->write(__ret);
    return DispatchOK;
}

DispatchStatus
Ice::Object::____ping(Incoming&)
{
    _ping();
    return DispatchOK;
}

string Ice::Object::__names[] =
{
    "_isA"
    "_ping"
};

DispatchStatus
Ice::Object::__dispatch(Incoming& in, const string& s)
{
    string* b = __names;
    string* e = __names + sizeof(__names) / sizeof(string);
    pair<string*, string*> r = equal_range(b, e, s);
    if (r.first == r.second)
    {
	return DispatchOperationNotExist;
    }					     

    switch (r.first - __names)
    {
	case 0:
	{
	    return ____isA(in);
	}
	case 1:
	{
	    return ____ping(in);
	}
    }

    assert(false);
    return DispatchOperationNotExist;
}

bool
Ice::Object::__isMutating(const std::string& s)
{
    return false;
}
