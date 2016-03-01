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
#include <Ice/Stream.h>
#include <Ice/SlicedData.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Object* Ice::upCast(Object* p) { return p; }

void 
Ice::__patch(ObjectPtr& obj, const ObjectPtr& v)
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


namespace
{

const string __Ice__Object_ids[] =
{
    "::Ice::Object"
};

}

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

Ice::ObjectPtr
Ice::Object::ice_clone() const
{
    throw CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // avoid warning with some compilers
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
Ice::Object::___ice_isA(Incoming& __inS, const Current& __current)
{
    BasicStream* __is = __inS.startReadParams();
    string __id;
    __is->read(__id, false);
    __inS.endReadParams();
    bool __ret = ice_isA(__id, __current);
    BasicStream* __os = __inS.__startWriteParams(DefaultFormat);
    __os->write(__ret);
    __inS.__endWriteParams(true);
    return DispatchOK;
}

DispatchStatus
Ice::Object::___ice_ping(Incoming& __inS, const Current& __current)
{
    __inS.readEmptyParams();
    ice_ping(__current);
    __inS.__writeEmptyParams();
    return DispatchOK;
}

DispatchStatus
Ice::Object::___ice_ids(Incoming& __inS, const Current& __current)
{
    __inS.readEmptyParams();
    vector<string> __ret = ice_ids(__current);
    BasicStream* __os = __inS.__startWriteParams(DefaultFormat);
    __os->write(&__ret[0], &__ret[0] + __ret.size(), false);
    __inS.__endWriteParams(true);
    return DispatchOK;
}

DispatchStatus
Ice::Object::___ice_id(Incoming& __inS, const Current& __current)
{
    __inS.readEmptyParams();
    string __ret = ice_id(__current);
    BasicStream* __os = __inS.__startWriteParams(DefaultFormat);
    __os->write(__ret, false);
    __inS.__endWriteParams(true);
    return DispatchOK;
}

Ice::Int
Ice::Object::ice_operationAttributes(const string&) const
{
    return 0;
}


string Ice::Object::__all[] =
{
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};


DispatchStatus
Ice::Object::ice_dispatch(Request& request, const DispatchInterceptorAsyncCallbackPtr& cb)
{
    class PushCb
    {
    public:
        PushCb(IceInternal::Incoming& in, const DispatchInterceptorAsyncCallbackPtr& cb) :
            _in(in),
            _cb(cb)
        {
            if(_cb != 0)
            {
                _in.push(_cb);
            }
        }

        ~PushCb()
        {
            if(_cb != 0)
            {
                _in.pop();
            }
        }
    private:
        IceInternal::Incoming& _in;
        const DispatchInterceptorAsyncCallbackPtr& _cb;
    };


    IceInternal::Incoming& in = dynamic_cast<IceInternal::IncomingRequest&>(request)._in;
    
    PushCb pusbCb(in, cb);
    in.startOver(); // may raise ResponseSentException
    return __dispatch(in, in.getCurrent());
}

DispatchStatus
Ice::Object::__dispatch(Incoming& in, const Current& current)
{
    pair<string*, string*> r =
        equal_range(__all, __all + sizeof(__all) / sizeof(string), current.operation);

    if(r.first == r.second)
    {
        throw OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
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
    throw OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
}

void
Ice::Object::__write(IceInternal::BasicStream* os) const
{
    os->startWriteObject(0);
    __writeImpl(os);
    os->endWriteObject();
}
 
void 
Ice::Object::__read(IceInternal::BasicStream* is)
{
   is->startReadObject();
   __readImpl(is);
   is->endReadObject(false);
}
    
void 
Ice::Object::__write(const OutputStreamPtr& os) const
{
    os->startObject(0);
    __writeImpl(os);
    os->endObject();
}
 
void 
Ice::Object::__read(const InputStreamPtr& is)
{
    is->startObject();
   __readImpl(is);
   is->endObject(false);
}
 
void 
Ice::Object::__writeImpl(const OutputStreamPtr&) const
{
    throw MarshalException(__FILE__, __LINE__, "class was not generated with stream support");
}
 
void 
Ice::Object::__readImpl(const InputStreamPtr&)
{
    throw MarshalException(__FILE__, __LINE__, "class was not generated with stream support");
}

namespace
{

string
operationModeToString(OperationMode mode)
{
    switch(mode)
    {
    case Normal:
        return "::Ice::Normal";

    case Nonmutating:
        return "::Ice::Nonmutating";

    case Idempotent:
        return "::Ice::Idempotent";
    }

    ostringstream os;
    os << "unknown value (" << mode << ")";
    return os.str();
}

}

void
Ice::Object::__checkMode(OperationMode expected, OperationMode received)
{
    if(expected != received)
    {
        if(expected == Idempotent && received == Nonmutating)
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
            std::ostringstream __reason;
            __reason << "unexpected operation mode. expected = "
                     << operationModeToString(expected)
                     << " received = "
                     << operationModeToString(received);
            ex.reason = __reason.str();
            throw ex;
        }
    }
}

DispatchStatus
Ice::Blobject::__dispatch(Incoming& in, const Current& current)
{
    const Byte* inEncaps;
    Int sz;
    in.readParamEncaps(inEncaps, sz);
    vector<Byte> outEncaps;
    bool ok = ice_invoke(vector<Byte>(inEncaps, inEncaps + sz), outEncaps, current);
    if(outEncaps.empty())
    {
        in.__writeParamEncaps(0, 0, ok);
    }
    else
    {
        in.__writeParamEncaps(&outEncaps[0], static_cast<Ice::Int>(outEncaps.size()), ok);
    }
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
Ice::BlobjectArray::__dispatch(Incoming& in, const Current& current)
{
    pair<const Byte*, const Byte*> inEncaps;
    Int sz;
    in.readParamEncaps(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    vector<Byte> outEncaps;
    bool ok = ice_invoke(inEncaps, outEncaps, current);
    if(outEncaps.empty())
    {
        in.__writeParamEncaps(0, 0, ok);
    }
    else
    {
        in.__writeParamEncaps(&outEncaps[0], static_cast<Ice::Int>(outEncaps.size()), ok);
    }
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
    const Byte* inEncaps;
    Int sz;
    in.readParamEncaps(inEncaps, sz);
    AMD_Object_ice_invokePtr cb = new ::IceAsync::Ice::AMD_Object_ice_invoke(in);
    try
    {
        ice_invoke_async(cb, vector<Byte>(inEncaps, inEncaps + sz), current);
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

DispatchStatus
Ice::BlobjectArrayAsync::__dispatch(Incoming& in, const Current& current)
{
    pair<const Byte*, const Byte*> inEncaps;
    Int sz;
    in.readParamEncaps(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    AMD_Object_ice_invokePtr cb = new ::IceAsync::Ice::AMD_Object_ice_invoke(in);
    try
    {
        ice_invoke_async(cb, inEncaps, current);
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
    out->write(p);
}

void
Ice::ice_readObject(const InputStreamPtr& in, ObjectPtr& p)
{
    in->read(p);
}
