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
#include <Ice/Current.h>

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
Ice::Object::ice_hash() const
{
    return reinterpret_cast<Int>(this);
}

const char* Ice::Object::__classIds[] =
{
    "::Ice::Object"
};

const char**
Ice::Object::__getClassIds()
{
    return __classIds;
}

bool
Ice::Object::ice_isA(const string& s, const Current&)
{
    return s == "::Ice::Object";
}

void
Ice::Object::ice_ping(const Current&)
{
    // Nothing to do.
}

DispatchStatus
Ice::Object::___ice_isA(Incoming& __in, const Current& __current)
{
    BasicStream* __is = __in.is();
    BasicStream* __os = __in.os();
    string __id;
    __is->read(__id);
    bool __ret = ice_isA(__id, __current);
    __os->write(__ret);
    return DispatchOK;
}

DispatchStatus
Ice::Object::___ice_ping(Incoming&, const Current& __current)
{
    ice_ping(__current);
    return DispatchOK;
}

const char* Ice::Object::__all[] =
{
    "ice_isA"
    "ice_ping"
};

DispatchStatus
Ice::Object::__dispatch(Incoming& in, const Current& current)
{
    const char** b = __all;
    const char** e = __all + sizeof(__all) / sizeof(const char*);
    pair<const char**, const char**> r = equal_range(b, e, current.operation);
    if (r.first == r.second)
    {
	return DispatchOperationNotExist;
    }					     

    switch (r.first - __all)
    {
	case 0:
	{
	    return ___ice_isA(in, current);
	}
	case 1:
	{
	    return ___ice_ping(in, current);
	}
    }

    assert(false);
    return DispatchOperationNotExist;
}

void
Ice::Object::__write(::IceInternal::BasicStream* __os) const
{
    JTCSyncT<JTCMutex> sync(_activeFacetMapMutex);
    
    __os->write(Int(_activeFacetMap.size()));
    for (map<string, ObjectPtr>::const_iterator p = _activeFacetMap.begin(); p != _activeFacetMap.end(); ++p)
    {
	__os->write(p->first);
	__os->write(p->second);
    }
}

void
Ice::Object::__read(::IceInternal::BasicStream* __is)
{
    JTCSyncT<JTCMutex> sync(_activeFacetMapMutex);

    Int sz;
    __is->read(sz);

    _activeFacetMap.clear();
    _activeFacetMapHint = _activeFacetMap.end();

    while (sz-- > 0)
    {
	pair<string, ObjectPtr> v;
	__is->read(v.first);
	__is->read("", v.second);
	_activeFacetMapHint = _activeFacetMap.insert(_activeFacetMapHint, v);
    }
}

void
Ice::Object::ice_addFacet(const ObjectPtr& facet, const string& name)
{
    JTCSyncT<JTCMutex> sync(_activeFacetMapMutex);

    _activeFacetMapHint = _activeFacetMap.insert(_activeFacetMapHint, make_pair(name, facet));
}

void
Ice::Object::ice_removeFacet(const string& name)
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
Ice::Object::ice_removeAllFacets()
{
    JTCSyncT<JTCMutex> sync(_activeFacetMapMutex);

    _activeFacetMap.clear();
    _activeFacetMapHint = _activeFacetMap.end();
}

ObjectPtr
Ice::Object::ice_findFacet(const string& name)
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

DispatchStatus
Ice::Blobject::__dispatch(Incoming& in, const Current& current)
{
    vector<Byte> inParams;
    vector<Byte> outParams;
    Int sz = in.is()->getReadEncapsSize();
    in.is()->readBlob(inParams, sz);
    ice_invoke(inParams, outParams, current);
    in.is()->writeBlob(outParams);
    return ::IceInternal::DispatchOK;
}
