// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Object.h>
#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Object* p) { p->__incRef(); }
void IceInternal::decRef(Object* p) { p->__decRef(); }

const char * const Ice::Object::_kindOfObject = "facet";

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
    return static_cast<Int>(reinterpret_cast<Long>(this) >> 4);
}

const string Ice::Object::__ids[] =
{
    "::Ice::Object"
};

bool
Ice::Object::ice_isA(const string& s, const Current&) const
{
    return s == __ids[0];
}

void
Ice::Object::ice_ping(const Current&) const
{
    // Nothing to do.
}

vector<string>
Ice::Object::ice_ids(const Current&) const
{
    return vector<string>(&__ids[0], &__ids[1]);
}

const string&
Ice::Object::ice_id(const Current&) const
{
    return __ids[0];
}

vector<string>
Ice::Object::ice_facets(const Current&) const
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

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

void
Ice::Object::__copyMembers(ObjectPtr to) const
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);
    to->_activeFacetMap = _activeFacetMap;
}

ObjectPtr
Ice::Object::ice_clone() const
{
    ObjectPtr __p = new Ice::Object;
#ifdef _WIN32
    Object::__copyMembers(__p);
#else
    ::Ice::Object::__copyMembers(__p);
#endif
    return __p;
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

DispatchStatus
Ice::Object::___ice_id(Incoming& __in, const Current& __current)
{
    BasicStream* __os = __in.os();
    string __ret = ice_id(__current);
    __os->write(__ret);
    return DispatchOK;
}

DispatchStatus
Ice::Object::___ice_facets(Incoming& __in, const Current& __current)
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
    pair<string*, string*> r =
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
Ice::Object::__write(BasicStream* __os, bool __marshalFacets) const
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);
    
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    if(__marshalFacets)
    {
        __os->writeSize(Int(_activeFacetMap.size()));
        for(map<string, ObjectPtr>::const_iterator p = _activeFacetMap.begin(); p != _activeFacetMap.end(); ++p)
        {
            __os->write(p->first);
            __os->write(p->second);
        }
    }
    else
    {
        __os->writeSize(0);
    }
    __os->endWriteSlice();
}

void
Ice::Object::__read(BasicStream* __is, bool __rid)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    if(__rid)
    {
	string myId;
	__is->readTypeId(myId);
    }

    __is->startReadSlice();

    Int sz;
    __is->readSize(sz);

    _activeFacetMap.clear();
    _activeFacetMapHint = _activeFacetMap.end();

    while(sz-- > 0)
    {
	string s;
	__is->read(s);
	pair<const string, ObjectPtr> v(s, ObjectPtr());
	_activeFacetMapHint = _activeFacetMap.insert(_activeFacetMapHint, v);
	__is->read(::Ice::__patch__ObjectPtr, &_activeFacetMapHint->second);
    }

    __is->endReadSlice();
}

void
Ice::Object::ice_addFacet(const ObjectPtr& facet, const string& name)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    if((_activeFacetMapHint != _activeFacetMap.end() && _activeFacetMapHint->first == name)
       || _activeFacetMap.find(name) != _activeFacetMap.end())
    {
	AlreadyRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = _kindOfObject;
	ex.id = name;
	throw ex;
    }

    _activeFacetMapHint = _activeFacetMap.insert(_activeFacetMapHint, pair<const string, ObjectPtr>(name, facet));
}

ObjectPtr
Ice::Object::ice_removeFacet(const string& name)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    ObjectPtr result;

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
	if(p == _activeFacetMap.end())
	{
	    NotRegisteredException ex(__FILE__, __LINE__);
	    ex.kindOfObject = _kindOfObject;
	    ex.id = name;
	    throw ex;
	}
    }
    assert(p != _activeFacetMap.end());
    
    result = p->second;

    if(p == _activeFacetMapHint)
    {
	_activeFacetMap.erase(p++);
	_activeFacetMapHint = p;
    }
    else
    {
	_activeFacetMap.erase(p);
    }

    return result;
}

ObjectPtr
Ice::Object::ice_updateFacet(const ObjectPtr& facet, const string& name)
{
    IceUtil::Mutex::Lock sync(_activeFacetMapMutex);

    ObjectPtr result;

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
	if(p == _activeFacetMap.end())
	{
	    NotRegisteredException ex(__FILE__, __LINE__);
	    ex.kindOfObject = _kindOfObject;
	    ex.id = name;
	    throw ex;
	}
    }
    assert(p != _activeFacetMap.end());
    
    result = p->second;
    p->second = facet;
    return result;
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

ObjectPtr
Ice::Object::ice_findFacetPath(const vector<string>& path, int start)
{
    int sz = static_cast<int>(path.size());

    if(start > sz)
    {
	return 0;
    }

    if(start == sz)
    {
	return this;
    }

    ObjectPtr f = ice_findFacet(path[start]);
    if(f)
    {
	return f->ice_findFacetPath(path, start + 1);
    }
    else
    {
	return f;
    }
}

void
Ice::__patch__ObjectPtr(void* __addr, ObjectPtr& v)
{
    ObjectPtr* p = static_cast<ObjectPtr*>(__addr);
    *p = v;
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
	return DispatchOK;
    }
    else
    {
	return DispatchUserException;
    }
}

DispatchStatus
Ice::BlobjectAsync::__dispatch(Incoming& in, const Current& current)
{
    vector<Byte> inParams;
    Int sz = in.is()->getReadEncapsSize();
    in.is()->readBlob(inParams, sz);
    AMD_Object_ice_invokePtr cb = new ::IceAsync::Ice::AMD_Object_ice_invoke(in);
    try
    {
	ice_invoke_async(cb, inParams, current);
    }
    catch(const Exception& ex)
    {
	cb->ice_exception(ex);
    }
    catch(const ::std::exception& ex)
    {
	cb->ice_exception(ex);
    }
    catch(...)
    {
	cb->ice_exception();
    }
    return DispatchAsync;
}
