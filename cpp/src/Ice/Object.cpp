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

Ice::Object::Object()
{
}

Ice::Object::~Object()
{
}

bool
Ice::Object::_isA(const string& s)
{
    return s == "::Ice::Object";
}

bool
Ice::Object::_hasFacet(const string& s)
{
    return false; // TODO
}

void
Ice::Object::_ping()
{
    // Nothing to do.
}

DispatchStatus
Ice::Object::____isA(Incoming& __in)
{
    BasicStream* __is = __in.is();
    BasicStream* __os = __in.os();
    string s;
    __is->read(s);
    bool __ret = _isA(s);
    __os->write(__ret);
    return DispatchOK;
}

DispatchStatus
Ice::Object::____hasFacet(Incoming& __in)
{
    BasicStream* __is = __in.is();
    BasicStream* __os = __in.os();
    string s;
    __is->read(s);
    bool __ret = _hasFacet(s);
    __os->write(__ret);
    return DispatchOK;
}

DispatchStatus
Ice::Object::____ping(Incoming&)
{
    _ping();
    return DispatchOK;
}

const char* Ice::Object::__all[] =
{
    "_hasFacet"
    "_isA"
    "_ping"
};

DispatchStatus
Ice::Object::__dispatch(Incoming& in, const string& s)
{
    const char** b = __all;
    const char** e = __all + sizeof(__all) / sizeof(const char*);
    pair<const char**, const char**> r = equal_range(b, e, s);
    if (r.first == r.second)
    {
	return DispatchOperationNotExist;
    }					     

    switch (r.first - __all)
    {
	case 0:
	{
	    return ____hasFacet(in);
	}
	case 1:
	{
	    return ____isA(in);
	}
	case 2:
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
    //
    // None of the Ice::Object operations accessible via __dispatch()
    // is mutating.
    //
    return false;
}
