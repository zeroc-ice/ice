// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
Ice::icePatch(ObjectPtr& obj, const ObjectPtr& v)
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

const string iceC_Ice_Object_ids[] =
{
    "::Ice::Object"
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
    return s == iceC_Ice_Object_ids[0];
}

void
Ice::Object::ice_ping(const Current&) const
{
    // Nothing to do.
}

vector<string>
Ice::Object::ice_ids(const Current&) const
{
    return vector<string>(&iceC_Ice_Object_ids[0], &iceC_Ice_Object_ids[1]);
}

#ifdef ICE_CPP11_MAPPING
string
#else
const string&
#endif
Ice::Object::ice_id(const Current&) const
{
    return iceC_Ice_Object_ids[0];
}

const ::std::string&
Ice::Object::ice_staticId()
{
    return iceC_Ice_Object_ids[0];
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
Ice::Object::iceD_ice_isA(Incoming& iceInS, const Current& iceCurrent)
{
    InputStream* iceIs = iceInS.startReadParams();
    string iceId;
    iceIs->read(iceId, false);
    iceInS.endReadParams();
#ifdef ICE_CPP11_MAPPING
    bool iceRet = ice_isA(move(iceId), iceCurrent);
#else
    bool iceRet = ice_isA(iceId, iceCurrent);
#endif
    OutputStream* iceOs = iceInS.startWriteParams();
    iceOs->write(iceRet);
    iceInS.endWriteParams();
    return false;
}

bool
Ice::Object::iceD_ice_ping(Incoming& iceInS, const Current& iceCurrent)
{
    iceInS.readEmptyParams();
    ice_ping(iceCurrent);
    iceInS.writeEmptyParams();
    return false;
}

bool
Ice::Object::iceD_ice_ids(Incoming& iceInS, const Current& iceCurrent)
{
    iceInS.readEmptyParams();
    vector<string> iceRet = ice_ids(iceCurrent);
    OutputStream* iceOs = iceInS.startWriteParams();
    iceOs->write(&iceRet[0], &iceRet[0] + iceRet.size(), false);
    iceInS.endWriteParams();
    return false;
}

bool
Ice::Object::iceD_ice_id(Incoming& iceInS, const Current& iceCurrent)
{
    iceInS.readEmptyParams();
    string iceRet = ice_id(iceCurrent);
    OutputStream* iceOs = iceInS.startWriteParams();
    iceOs->write(iceRet, false);
    iceInS.endWriteParams();
    return false;
}


string Ice::Object::s_iceAll[] =
{
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};


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
            return iceDispatch(in, in.getCurrent());
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
        return iceDispatch(in, in.getCurrent());
    }
}

bool
Ice::Object::iceDispatch(Incoming& in, const Current& current)
{
    pair<string*, string*> r = equal_range(s_iceAll, s_iceAll + sizeof(s_iceAll) / sizeof(string), current.operation);

    if(r.first == r.second)
    {
        throw OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch(r.first - s_iceAll)
    {
        case 0:
        {
            return iceD_ice_id(in, current);
        }
        case 1:
        {
            return iceD_ice_ids(in, current);
        }
        case 2:
        {
            return iceD_ice_isA(in, current);
        }
        case 3:
        {
            return iceD_ice_ping(in, current);
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
Ice::Object::iceWrite(Ice::OutputStream* os) const
{
    os->startValue(0);
    iceWriteImpl(os);
    os->endValue();
}

void
Ice::Object::iceRead(Ice::InputStream* is)
{
   is->startValue();
   iceReadImpl(is);
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
Ice::Object::iceCheckMode(OperationMode expected, OperationMode received)
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
            std::ostringstream iceReason;
            iceReason << "unexpected operation mode. expected = "
                     << operationModeToString(expected)
                     << " received = "
                     << operationModeToString(received);
            ex.reason = iceReason.str();
            throw ex;
        }
    }
}

bool
Ice::Blobject::iceDispatch(Incoming& in, const Current& current)
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
Ice::BlobjectArray::iceDispatch(Incoming& in, const Current& current)
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
Ice::BlobjectAsync::iceDispatch(Incoming& in, const Current& current)
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
Ice::BlobjectArrayAsync::iceDispatch(Incoming& in, const Current& current)
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
