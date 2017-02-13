// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Object.h>
#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/IncomingRequest.h>
#include <Ice/LocalException.h>
#include <Ice/SlicedData.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace Ice
{
const Current noExplicitCurrent = Current();
}

#ifndef ICE_CPP11_MAPPING
Object* Ice::upCast(Object* p) { return p; }

void
Ice::_icePatchObjectPtr(ObjectPtr& obj, const ObjectPtr& v)
{
    obj = v;
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
#endif

namespace
{

const string object_ids[] =
{
    "::Ice::Object"
};

const string object_all[] =
{
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};

}

#ifndef ICE_CPP11_MAPPING
Ice::DispatchInterceptorAsyncCallback::~DispatchInterceptorAsyncCallback()
{
    // Out of line to avoid weak vtable
}
#endif

Ice::Request::~Request()
{
    // Out of line to avoid weak vtable
}

bool
#ifdef ICE_CPP11_MAPPING
Ice::Object::ice_isA(string s, const Current&) const
#else
Ice::Object::ice_isA(const string& s, const Current&) const
#endif
{
    return s == object_ids[0];
}

void
Ice::Object::ice_ping(const Current&) const
{
    // Nothing to do.
}

vector<string>
Ice::Object::ice_ids(const Current&) const
{
    return vector<string>(&object_ids[0], &object_ids[1]);
}

#ifdef ICE_CPP11_MAPPING
string
#else
const string&
#endif
Ice::Object::ice_id(const Current&) const
{
    return object_ids[0];
}

const ::std::string&
Ice::Object::ice_staticId()
{
    return object_ids[0];
}

#ifndef ICE_CPP11_MAPPING
Ice::ObjectPtr
Ice::Object::ice_clone() const
{
    throw CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // avoid warning with some compilers
}
#endif

bool
Ice::Object::_iceD_ice_isA(Incoming& inS, const Current& current)
{
    InputStream* istr = inS.startReadParams();
    string iceP_id;
    istr->read(iceP_id, false);
    inS.endReadParams();
#ifdef ICE_CPP11_MAPPING
    bool ret = ice_isA(move(iceP_id), current);
#else
    bool ret = ice_isA(iceP_id, current);
#endif
    OutputStream* ostr = inS.startWriteParams();
    ostr->write(ret);
    inS.endWriteParams();
    return false;
}

bool
Ice::Object::_iceD_ice_ping(Incoming& inS, const Current& current)
{
    inS.readEmptyParams();
    ice_ping(current);
    inS.writeEmptyParams();
    return false;
}

bool
Ice::Object::_iceD_ice_ids(Incoming& inS, const Current& current)
{
    inS.readEmptyParams();
    vector<string> ret = ice_ids(current);
    OutputStream* ostr = inS.startWriteParams();
    ostr->write(&ret[0], &ret[0] + ret.size(), false);
    inS.endWriteParams();
    return false;
}

bool
Ice::Object::_iceD_ice_id(Incoming& inS, const Current& current)
{
    inS.readEmptyParams();
    string ret = ice_id(current);
    OutputStream* ostr = inS.startWriteParams();
    ostr->write(ret, false);
    inS.endWriteParams();
    return false;
}


bool
#ifdef ICE_CPP11_MAPPING
Ice::Object::ice_dispatch(Request& request, std::function<bool()> r, std::function<bool(std::exception_ptr)> e)
#else
Ice::Object::ice_dispatch(Request& request, const DispatchInterceptorAsyncCallbackPtr& cb)
#endif
{
    IceInternal::Incoming& in = dynamic_cast<IceInternal::IncomingRequest&>(request)._in;
    in.startOver();
#ifdef ICE_CPP11_MAPPING
    if(r || e)
    {
        in.push(r, e);
#else
    if(cb)
    {
        in.push(cb);
#endif
        try
        {
            return _iceDispatch(in, in.getCurrent());
            in.pop();
        }
        catch(...)
        {
            in.pop();
            throw;
        }
    }
    else
    {
        return _iceDispatch(in, in.getCurrent());
    }
}

bool
Ice::Object::_iceDispatch(Incoming& in, const Current& current)
{
    pair<const string*, const string*> r = equal_range(object_all, object_all + sizeof(object_all) / sizeof(string), current.operation);

    if(r.first == r.second)
    {
        throw OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch(r.first - object_all)
    {
        case 0:
        {
            return _iceD_ice_id(in, current);
        }
        case 1:
        {
            return _iceD_ice_ids(in, current);
        }
        case 2:
        {
            return _iceD_ice_isA(in, current);
        }
        case 3:
        {
            return _iceD_ice_ping(in, current);
        }
        default:
        {
            assert(false);
            throw OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
        }
    }
}

#ifndef ICE_CPP11_MAPPING
void
Ice::Object::ice_preMarshal()
{
}

void
Ice::Object::ice_postUnmarshal()
{
}

void
Ice::Object::_iceWrite(Ice::OutputStream* os) const
{
    os->startValue(0);
    _iceWriteImpl(os);
    os->endValue();
}

void
Ice::Object::_iceRead(Ice::InputStream* is)
{
   is->startValue();
   _iceReadImpl(is);
   is->endValue(false);
}

Ice::Int
Ice::Object::ice_operationAttributes(const string&) const
{
    return 0;
}
#endif

namespace
{

string
operationModeToString(OperationMode mode)
{
    switch(mode)
    {
    case ICE_ENUM(OperationMode, Normal):
        return "::Ice::Normal";

    case ICE_ENUM(OperationMode, Nonmutating):
        return "::Ice::Nonmutating";

    case ICE_ENUM(OperationMode, Idempotent):
        return "::Ice::Idempotent";
    }
    //
    // This could not happen with C++11 strong type enums
    //
#ifdef ICE_CPP11_MAPPING
    assert(false);
    return "";
#else
    ostringstream os;
    os << "unknown value (" << mode << ")";
    return os.str();
#endif
}

}

void
Ice::Object::_iceCheckMode(OperationMode expected, OperationMode received)
{
    if(expected != received)
    {
        if(expected == ICE_ENUM(OperationMode, Idempotent) && received == ICE_ENUM(OperationMode, Nonmutating))
        {
            //
            // Fine: typically an old client still using the deprecated nonmutating keyword
            //

            //
            // Note that expected == Nonmutating and received == Idempotent is not ok:
            // the server may still use the deprecated nonmutating keyword to detect updates
            // and the client should not break this (deprecated) feature.
            //
        }
        else
        {
            Ice::MarshalException ex(__FILE__, __LINE__);
            std::ostringstream reason;
            reason << "unexpected operation mode. expected = "
                   << operationModeToString(expected)
                   << " received = "
                   << operationModeToString(received);
            ex.reason = reason.str();
            throw ex;
        }
    }
}

bool
Ice::Blobject::_iceDispatch(Incoming& in, const Current& current)
{
    const Byte* inEncaps;
    Int sz;
    in.readParamEncaps(inEncaps, sz);
    vector<Byte> outEncaps;
    bool ok = ice_invoke(vector<Byte>(inEncaps, inEncaps + sz), outEncaps, current);
    if(outEncaps.empty())
    {
        in.writeParamEncaps(0, 0, ok);
    }
    else
    {
        in.writeParamEncaps(&outEncaps[0], static_cast<Ice::Int>(outEncaps.size()), ok);
    }
    return false;
}

bool
Ice::BlobjectArray::_iceDispatch(Incoming& in, const Current& current)
{
    pair<const Byte*, const Byte*> inEncaps;
    Int sz;
    in.readParamEncaps(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    vector<Byte> outEncaps;
    bool ok = ice_invoke(inEncaps, outEncaps, current);
    if(outEncaps.empty())
    {
        in.writeParamEncaps(0, 0, ok);
    }
    else
    {
        in.writeParamEncaps(&outEncaps[0], static_cast<Ice::Int>(outEncaps.size()), ok);
    }
    return false;
}

bool
Ice::BlobjectAsync::_iceDispatch(Incoming& in, const Current& current)
{
    const Byte* inEncaps;
    Int sz;
    in.readParamEncaps(inEncaps, sz);
#ifdef ICE_CPP11_MAPPING
    auto async = IncomingAsync::create(in);
    ice_invokeAsync(vector<Byte>(inEncaps, inEncaps + sz),
                    [async](bool ok, const vector<Byte>& outEncaps)
                    {
                        if(outEncaps.empty())
                        {
                            async->writeParamEncaps(0, 0, ok);
                        }
                        else
                        {
                            async->writeParamEncaps(&outEncaps[0], static_cast<Int>(outEncaps.size()), ok);
                        }
                        async->completed();
                    },
                    async->exception(), current);
#else
    ice_invoke_async(new ::IceAsync::Ice::AMD_Object_ice_invoke(in), vector<Byte>(inEncaps, inEncaps + sz), current);
#endif
    return true;
}

bool
Ice::BlobjectArrayAsync::_iceDispatch(Incoming& in, const Current& current)
{
    pair<const Byte*, const Byte*> inEncaps;
    Int sz;
    in.readParamEncaps(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
#ifdef ICE_CPP11_MAPPING
    auto async = IncomingAsync::create(in);
    ice_invokeAsync(inEncaps,
                    [async](bool ok, const pair<const Byte*, const Byte*>& outE)
                    {
                        async->writeParamEncaps(outE.first, static_cast<Int>(outE.second - outE.first), ok);
                        async->completed();
                    },
                    async->exception(), current);
#else
    ice_invoke_async(new ::IceAsync::Ice::AMD_Object_ice_invoke(in), inEncaps, current);
#endif
    return true;
}
