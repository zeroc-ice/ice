// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Object.h>
#include <Ice/Incoming.h>
#include <Ice/Current.h>
#include <Ice/Stream.h>

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
Ice::Object::operator!=(const Object& r) const
{
    return this != &r;
}

bool
Ice::Object::operator<(const Object& r) const
{
    return this < &r;
}

Int
Ice::Object::ice_hash() const
{
    return reinterpret_cast<Int>(this) >> 4;
}

const string Ice::Object::__ids[] =
{
    "::Ice::Object"
};

bool
Ice::Object::ice_isA(const string& s, const Current&)
{
    return s == __ids[0];
}

void
Ice::Object::ice_ping(const Current&)
{
    // Nothing to do.
}

vector<string>
Ice::Object::ice_ids(const Current&)
{
    return vector<string>(&__ids[0], &__ids[1]);
}

const string&
Ice::Object::ice_id(const Current&)
{
    return __ids[0];
}

vector<string>
Ice::Object::ice_facets(const Current&)
{
    vector<string> v;
    for(map<string, ObjectPtr>::const_iterator p = _activeFacetMap.begin(); p != _activeFacetMap.end(); ++p)
    {
	v.push_back(p->first);
    }
    return v;
}


const ::std::string&
Ice::Object::ice_staticId()
{
    return __ids[0];
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

DispatchStatus
Ice::Object::___ice_ids(Incoming& __in, const Current& __current)
{
    BasicStream* __os = __in.os();
    vector<string> __ret = ice_ids(__current);
    __os->write(__ret);
    return DispatchOK;
}

::IceInternal::DispatchStatus
Ice::Object::___ice_id(::IceInternal::Incoming& __in, const Current& __current)
{
    BasicStream* __os = __in.os();
    string __ret = ice_id(__current);
    __os->write(__ret);
    return DispatchOK;
}

::IceInternal::DispatchStatus
Ice::Object::___ice_facets(::IceInternal::Incoming& __in, const Current& __current)
{
    BasicStream* __os = __in.os();
    vector<string> __ret = ice_facets(__current);
    __os->write(__ret);
    return DispatchOK;
}


string Ice::Object::__all[] =
{
    "ice_facets",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};

DispatchStatus
Ice::Object::__dispatch(Incoming& in, const Current& current)
{
    pair<const string*, const string*> r =
	equal_range(__all, __all + sizeof(__all) / sizeof(string), current.operation);

    if(r.first == r.second)
    {
	return DispatchOperationNotExist;
    }					     

    switch(r.first - __all)
    {
        case 0:
        {
	    return ___ice_facets(in, current);
        }
        case 1:
        {
	    return ___ice_id(in, current);
        }
        case 2:
        {
	    return ___ice_ids(in, current);
        }
        case 3:
	{
	    return ___ice_isA(in, current);
	}
	case 4:
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
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);
    
    __os->writeSize(Int(_activeFacetMap.size()));
    for(map<string, ObjectPtr>::const_iterator p = _activeFacetMap.begin(); p != _activeFacetMap.end(); ++p)
    {
	__os->write(p->first);
	__os->write(p->second);
    }
}

void
Ice::Object::__read(::IceInternal::BasicStream* __is)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    Int sz;
    __is->readSize(sz);

    _activeFacetMap.clear();
    _activeFacetMapHint = _activeFacetMap.end();

    while(sz-- > 0)
    {
	pair<string, ObjectPtr> v;
	__is->read(v.first);
	__is->read("", 0, v.second);
	_activeFacetMapHint = _activeFacetMap.insert(_activeFacetMapHint, v);
    }
}

void
Ice::Object::__marshal(const ::Ice::StreamPtr& __os) const
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    __os->startWriteDictionary("ice:facets", _activeFacetMap.size());
    for(map<string, ObjectPtr>::const_iterator p = _activeFacetMap.begin(); p != _activeFacetMap.end(); ++p)
    {
	__os->startWriteDictionaryElement();
	__os->writeString("key", p->first);
	__os->writeObject("value", p->second);
	__os->endWriteDictionaryElement();
    }
    __os->endWriteDictionary();
}


void
Ice::Object::__unmarshal(const ::Ice::StreamPtr& __is)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);
 
    static const string facetsName = "ice:facets";
    static const string keyName = "key";
    static const string valueName = "value";

    Int sz = __is->startReadDictionary(facetsName);
    
    _activeFacetMap.clear();
    _activeFacetMapHint = _activeFacetMap.end();
    
    while(sz-- > 0)
    {
	__is->startReadDictionaryElement();
	pair<string, ObjectPtr> v;
	v.first = __is->readString(keyName);
	v.second = __is->readObject(valueName, "", 0);
	_activeFacetMapHint = _activeFacetMap.insert(_activeFacetMapHint, v);
	__is->endReadDictionaryElement();
    }
    __is->endReadDictionary();
}

void
Ice::Object::ice_marshal(const string& name, const ::Ice::StreamPtr& stream)
{
    stream->writeObject(name, this);
}

void
Ice::Object::ice_unmarshal(const string& name, const ::Ice::StreamPtr& stream, ObjectPtr& value)
{
    value = stream->readObject(name, "", 0);
}

void
Ice::Object::ice_addFacet(const ObjectPtr& facet, const string& name)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    _activeFacetMapHint = _activeFacetMap.insert(_activeFacetMapHint, make_pair(name, facet));
}

void
Ice::Object::ice_removeFacet(const string& name)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    map<string, ObjectPtr>::iterator p = _activeFacetMap.end();
    
    if(_activeFacetMapHint != _activeFacetMap.end())
    {
	if(_activeFacetMapHint->first == name)
	{
	    p = _activeFacetMapHint;
	}
    }
    
    if(p == _activeFacetMap.end())
    {
	p = _activeFacetMap.find(name);
    }
    
    if(p != _activeFacetMap.end())
    {
	if(p == _activeFacetMapHint)
	{
	    _activeFacetMap.erase(p++);
	    _activeFacetMapHint = p;
	}
	else
	{
	    _activeFacetMap.erase(p);
	}
    }
}

void
Ice::Object::ice_removeAllFacets()
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    _activeFacetMap.clear();
    _activeFacetMapHint = _activeFacetMap.end();
}

ObjectPtr
Ice::Object::ice_findFacet(const string& name)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);
    
    map<string, ObjectPtr>::iterator p = _activeFacetMap.end();
    
    if(_activeFacetMapHint != _activeFacetMap.end())
    {
	if(_activeFacetMapHint->first == name)
	{
	    p = _activeFacetMapHint;
	}
    }
    
    if(p == _activeFacetMap.end())
    {
	p = _activeFacetMap.find(name);
    }
    
    if(p != _activeFacetMap.end())
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
    bool ok = ice_invoke(inParams, outParams, current);
    in.os()->writeBlob(outParams);
    if(ok)
    {
	return ::IceInternal::DispatchOK;
    }
    else
    {
	return ::IceInternal::DispatchUserException;
    }
}
