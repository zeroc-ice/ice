// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Object.h>
#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/LocalException.h>
#include <Ice/Stream.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Object* p) { p->__incRef(); }
void IceInternal::decRef(Object* p) { p->__decRef(); }

Ice::Object::Object()
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

static const string __Ice__Object_ids[] =
{
    "::Ice::Object"
};

bool
Ice::Object::ice_isA(const string& s, const Current&) const
{
    return s == __Ice__Object_ids[0];
}

void
Ice::Object::ice_ping(const Current&) const
{
    // Nothing to do.
}

vector<string>
Ice::Object::ice_ids(const Current&) const
{
    return vector<string>(&__Ice__Object_ids[0], &__Ice__Object_ids[1]);
}

const string&
Ice::Object::ice_id(const Current&) const
{
    return __Ice__Object_ids[0];
}

const ::std::string&
Ice::Object::ice_staticId()
{
    return __Ice__Object_ids[0];
}

void
Ice::Object::__copyMembers(ObjectPtr to) const
{
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

void
Ice::Object::ice_preMarshal()
{
}

void
Ice::Object::ice_postUnmarshal()
{
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

string Ice::Object::__all[] =
{
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

void
Ice::Object::__write(BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->writeSize(0); // For compatibility with the old AFM.
    __os->endWriteSlice();
}

void
Ice::Object::__read(BasicStream* __is, bool __rid)
{
    if(__rid)
    {
	string myId;
	__is->readTypeId(myId);
    }

    __is->startReadSlice();

    // For compatibility with the old AFM.
    Int sz;
    __is->readSize(sz);
    if(sz != 0)
    {
	throw Ice::MarshalException(__FILE__, __LINE__);
    }

    __is->endReadSlice();
}

void
Ice::Object::__write(const OutputStreamPtr& __out) const
{
    __out->writeTypeId(ice_staticId());
    __out->startSlice();
    __out->writeSize(0); // For compatibility with the old AFM.
    __out->endSlice();
}

void
Ice::Object::__read(const InputStreamPtr& __in, bool __rid)
{
    if(__rid)
    {
	__in->readTypeId();
    }

    __in->startSlice();

    // For compatibility with the old AFM.
    Int sz = __in->readSize();
    if(sz != 0)
    {
	throw Ice::MarshalException(__FILE__, __LINE__);
    }

    __in->endSlice();
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

void
Ice::ice_writeObject(const OutputStreamPtr& out, const ObjectPtr& p)
{
    out->writeObject(p);
}

void
Ice::ice_readObject(const InputStreamPtr& in, ObjectPtr& p)
{
    Ice::ReadObjectCallbackPtr cb = new ReadObjectCallbackI(__patch__ObjectPtr, &p);
    in->readObject(cb);
}
