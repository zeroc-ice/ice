// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Object.h>
#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/IncomingRequest.h>
#include <Ice/Direct.h>
#include <Ice/LocalException.h>
#include <Ice/Stream.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::GCShared* IceInternal::upCast(Object* p) { return p; }

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

ObjectPtr
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
    BasicStream* __is = __inS.is();
    BasicStream* __os = __inS.os();
    string __id;
    __is->read(__id, false);
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
Ice::Object::___ice_ids(Incoming& __inS, const Current& __current)
{
    BasicStream* __os = __inS.os();
    vector<string> __ret = ice_ids(__current);
    __os->write(&__ret[0], &__ret[0] + __ret.size(), false);
    return DispatchOK;
}

DispatchStatus
Ice::Object::___ice_id(Incoming& __inS, const Current& __current)
{
    BasicStream* __os = __inS.os();
    string __ret = ice_id(__current);
    __os->write(__ret, false);
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


    if(request.isCollocated())
    {
        return __collocDispatch(dynamic_cast<IceInternal::Direct&>(request));
    }
    else
    {
        IceInternal::Incoming& in = dynamic_cast<IceInternal::IncomingRequest&>(request)._in;
        
        PushCb pusbCb(in, cb);
        in.startOver(); // may raise ResponseSentException
        return __dispatch(in, in.getCurrent());
    }
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

DispatchStatus
Ice::Object::__collocDispatch(IceInternal::Direct& request)
{
    return request.run(this);
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
Ice::Object::__write(const OutputStreamPtr& __outS) const
{
    __outS->writeTypeId(ice_staticId());
    __outS->startSlice();
    __outS->writeSize(0); // For compatibility with the old AFM.
    __outS->endSlice();
}

void
Ice::Object::__read(const InputStreamPtr& __inS, bool __rid)
{
    if(__rid)
    {
        __inS->readTypeId();
    }

    __inS->startSlice();

    // For compatibility with the old AFM.
    Int sz = __inS->readSize();
    if(sz != 0)
    {
        throw Ice::MarshalException(__FILE__, __LINE__);
    }

    __inS->endSlice();
}

void
Ice::__patch__ObjectPtr(void* __addr, ObjectPtr& v)
{
    ObjectPtr* p = static_cast<ObjectPtr*>(__addr);
    *p = v;
}

static string
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
Ice::BlobjectArray::__dispatch(Incoming& in, const Current& current)
{
    pair<const Byte*, const Byte*> inParams;
    vector<Byte> outParams;
    Int sz = in.is()->getReadEncapsSize();
    in.is()->readBlob(inParams.first, sz);
    inParams.second = inParams.first + sz;
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

DispatchStatus
Ice::BlobjectArrayAsync::__dispatch(Incoming& in, const Current& current)
{
    pair<const Byte*, const Byte*> inParams;
    Int sz = in.is()->getReadEncapsSize();
    in.is()->readBlob(inParams.first, sz);
    inParams.second = inParams.first + sz;
    AMD_Array_Object_ice_invokePtr cb = new ::IceAsync::Ice::AMD_Array_Object_ice_invoke(in);
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
