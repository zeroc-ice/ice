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

Ice::Object::Object() :
    _activeFacetMapHint(_activeFacetMap.end())
{
}

bool
Ice::Object::operator==(const Object& r) const
{
    return this == &r;
}

bool
Ice::Object::operator<(const Object& r) const
{
    return this < &r;
}

Int
Ice::Object::_hash() const
{
    return reinterpret_cast<Int>(this);
}

bool
Ice::Object::_isA(const string& s)
{
    return s == "::Ice::Object";
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
Ice::Object::____ping(Incoming&)
{
    _ping();
    return DispatchOK;
}

const char* Ice::Object::__all[] =
{
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
    //
    // None of the Ice::Object operations accessible via __dispatch()
    // is mutating.
    //
    return false;
}

void
Ice::Object::_addFacet(const ObjectPtr& facet, const string& name)
{
    JTCSyncT<JTCMutex> sync(_activeFacetMapMutex);

    _activeFacetMapHint = _activeFacetMap.insert(_activeFacetMapHint, make_pair(name, facet));
}

void
Ice::Object::_removeFacet(const string& name)
{
    JTCSyncT<JTCMutex> sync(_activeFacetMapMutex);

    map<string, ObjectPtr>::iterator p = _activeFacetMap.end();
    
    if (_activeFacetMapHint != _activeFacetMap.end())
    {
	if (_activeFacetMapHint->first == name)
	{
	    p = _activeFacetMapHint;
	}
    }
    
    if (p == _activeFacetMap.end())
    {
	p = _activeFacetMap.find(name);
    }
    
    if (p != _activeFacetMap.end())
    {
	_activeFacetMap.erase(p);
	_activeFacetMapHint = _activeFacetMap.end();
    }
}

void
Ice::Object::_removeAllFacets()
{
    JTCSyncT<JTCMutex> sync(_activeFacetMapMutex);

    _activeFacetMap.clear();
    _activeFacetMapHint = _activeFacetMap.end();
}

ObjectPtr
Ice::Object::_findFacet(const string& name)
{
    JTCSyncT<JTCMutex> sync(_activeFacetMapMutex);
    
    map<string, ObjectPtr>::iterator p = _activeFacetMap.end();
    
    if (_activeFacetMapHint != _activeFacetMap.end())
    {
	if (_activeFacetMapHint->first == name)
	{
	    p = _activeFacetMapHint;
	}
    }
    
    if (p == _activeFacetMap.end())
    {
	p = _activeFacetMap.find(name);
    }
    
    if (p != _activeFacetMap.end())
    {
	_activeFacetMapHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }
}
