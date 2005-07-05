// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Object.h>
#include <IceE/Incoming.h>
#include <IceE/LocalException.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(Object* p) { p->__incRef(); }
void IceEInternal::decRef(Object* p) { p->__decRef(); }

bool
IceE::Object::operator==(const Object& r) const
{
    return this == &r;
}

bool
IceE::Object::operator!=(const Object& r) const
{
    return this != &r;
}

bool
IceE::Object::operator<(const Object& r) const
{
    return this < &r;
}

Int
IceE::Object::ice_hash() const
{
    return static_cast<Int>(reinterpret_cast<Long>(this) >> 4);
}

static const string __Ice__Object_ids[] =
{
    "::IceE::Object"
};

bool
IceE::Object::ice_isA(const string& s, const Current&) const
{
    return s == __Ice__Object_ids[0];
}

void
IceE::Object::ice_ping(const Current&) const
{
    // Nothing to do.
}

vector<string>
IceE::Object::ice_ids(const Current&) const
{
    return vector<string>(&__Ice__Object_ids[0], &__Ice__Object_ids[1]);
}

const string&
IceE::Object::ice_id(const Current&) const
{
    return __Ice__Object_ids[0];
}

const ::std::string&
IceE::Object::ice_staticId()
{
    return __Ice__Object_ids[0];
}

ObjectPtr
IceE::Object::ice_clone() const
{
    throw new CloneNotImplementedException(__FILE__, __LINE__);
}

void
IceE::Object::ice_preMarshal()
{
}

void
IceE::Object::ice_postUnmarshal()
{
}

DispatchStatus
IceE::Object::___ice_isA(Incoming& __inS, const Current& __current)
{
    BasicStream* __is = __inS.is();
    BasicStream* __os = __inS.os();
    string __id;
    __is->read(__id);
    bool __ret = ice_isA(__id, __current);
    __os->write(__ret);
    return DispatchOK;
}

DispatchStatus
IceE::Object::___ice_ping(Incoming&, const Current& __current)
{
    ice_ping(__current);
    return DispatchOK;
}

DispatchStatus
IceE::Object::___ice_ids(Incoming& __inS, const Current& __current)
{
    BasicStream* __os = __inS.os();
    vector<string> __ret = ice_ids(__current);
    __os->write(__ret);
    return DispatchOK;
}

DispatchStatus
IceE::Object::___ice_id(Incoming& __inS, const Current& __current)
{
    BasicStream* __os = __inS.os();
    string __ret = ice_id(__current);
    __os->write(__ret);
    return DispatchOK;
}

string IceE::Object::__all[] =
{
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};

DispatchStatus
IceE::Object::__dispatch(Incoming& in, const Current& current)
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
	    return ___ice_id(in, current);
        }
        case 1:
        {
	    return ___ice_ids(in, current);
        }
        case 2:
	{
	    return ___ice_isA(in, current);
	}
	case 3:
	{
	    return ___ice_ping(in, current);
	}
    }

    assert(false);
    return DispatchOperationNotExist;
}

DispatchStatus
IceE::Blobject::__dispatch(Incoming& in, const Current& current)
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
