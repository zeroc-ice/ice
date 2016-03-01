// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/Object.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Reference.h>
#include <Ice/EndpointI.h>
#include <Ice/Instance.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocatorInfo.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/ConnectionI.h> // To convert from ConnectionIPtr to ConnectionPtr in ice_getConnection().
#include <Ice/Stream.h>
#include <Ice/ImplicitContextI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

const string ice_ping_name = "ice_ping";
const string ice_ids_name = "ice_ids";
const string ice_id_name = "ice_id";
const string ice_isA_name = "ice_isA";
const string ice_invoke_name = "ice_invoke";
const string ice_getConnection_name = "ice_getConnection";
const string ice_flushBatchRequests_name = "ice_flushBatchRequests";

}

::Ice::ObjectPrx
IceInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId, const Context* context)
{
    if(b)
    {
        ObjectPrx bb = b->ice_facet(f);
        try
        {
            if(context == 0 ? bb->ice_isA(typeId) : bb->ice_isA(typeId, *context))
            {
                return bb;
            }
#ifndef NDEBUG
            else
            {
                assert(typeId != "::Ice::Object");
            }
#endif
        }
        catch(const FacetNotExistException&)
        {
        }
    }
    return 0;
}

#ifdef ICE_CPP11

IceInternal::Cpp11FnCallbackNC::Cpp11FnCallbackNC(const ::std::function<void (const ::Ice::Exception&)>& excb,
                                                  const ::std::function<void (bool)>& sentcb) :
    _exception(excb),
    _sent(sentcb)
{
}

IceInternal::CallbackBasePtr
IceInternal::Cpp11FnCallbackNC::verify(const ::Ice::LocalObjectPtr&)
{
    return this;
}

void
IceInternal::Cpp11FnCallbackNC::sent(const ::Ice::AsyncResultPtr& result) const
{
    if(_sent != nullptr)
    {
        _sent(result->sentSynchronously());
    }
}

bool
IceInternal::Cpp11FnCallbackNC::hasSentCallback() const
{
    return _sent != nullptr;
}

void
IceInternal::Cpp11FnCallbackNC::exception(const ::Ice::AsyncResultPtr&, const ::Ice::Exception& ex) const
{
    if(_exception != nullptr)
    {
        _exception(ex);
    }
}

IceInternal::Cpp11FnOnewayCallbackNC::Cpp11FnOnewayCallbackNC(const ::std::function<void ()>& cb,
                                                              const ::std::function<void (const ::Ice::Exception&)>& excb,
                                                              const ::std::function<void (bool)>& sentcb) :
    Cpp11FnCallbackNC(excb, sentcb),
    _cb(cb)
{
    CallbackBase::checkCallback(true, cb || excb != nullptr);
}

void
IceInternal::Cpp11FnOnewayCallbackNC::completed(const ::Ice::AsyncResultPtr& result) const
{
    try
    {
        result->getProxy()->__end(result, result->getOperation());
    }
    catch(const ::Ice::Exception& ex)
    {
        Cpp11FnCallbackNC::exception(result, ex);
        return;
    }
    if(_cb != nullptr)
    {
        _cb();
    }
}
#endif

bool
IceProxy::Ice::Object::operator==(const Object& r) const
{
    return _reference == r._reference;
}

bool
IceProxy::Ice::Object::operator!=(const Object& r) const
{
    return _reference != r._reference;
}

bool
IceProxy::Ice::Object::operator<(const Object& r) const
{
    return _reference < r._reference;
}

CommunicatorPtr
IceProxy::Ice::Object::ice_getCommunicator() const
{
    return _reference->getCommunicator();
}

string
IceProxy::Ice::Object::ice_toString() const
{
    //
    // Returns the stringified proxy. There's no need to convert the
    // string to a native string: a stringified proxy only contains
    // printable ASCII which is a subset of all native character sets.
    //
    return _reference->toString();
}


bool
IceProxy::Ice::Object::ice_isA(const string& typeId, const Context* context)
{
    __checkTwowayOnly(ice_isA_name);
    Outgoing __og(this, ice_isA_name, ::Ice::Nonmutating, context);
    try
    {
        BasicStream* __os = __og.startWriteParams(DefaultFormat);
        __os->write(typeId, false);
        __og.endWriteParams();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    if(!__og.invoke())
    {
        try
        {
            __og.throwUserException();
        }
        catch(const ::Ice::UserException& __ex)
        {
            throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
        }
    }
    bool __ret;
    BasicStream* __is = __og.startReadParams();
    __is->read(__ret);
    __og.endReadParams();
    return __ret;
}

Ice::AsyncResultPtr
IceProxy::Ice::Object::begin_ice_isA(const string& typeId,
                                     const Context* ctx,
                                     const ::IceInternal::CallbackBasePtr& del,
                                     const ::Ice::LocalObjectPtr& cookie)
{
    __checkAsyncTwowayOnly(ice_isA_name);
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_isA_name, del, cookie);
    try
    {
        __result->prepare(ice_isA_name, Nonmutating, ctx);
        IceInternal::BasicStream* __os = __result->startWriteParams(DefaultFormat);
        __os->write(typeId);
        __result->endWriteParams();
        __result->invoke();
    }
    catch(const Exception& __ex)
    {
        __result->abort(__ex);
    }
    return __result;
}

#ifdef ICE_CPP11

Ice::AsyncResultPtr
IceProxy::Ice::Object::__begin_ice_isA(const ::std::string& typeId,
                                       const ::Ice::Context* ctx,
                                       const ::IceInternal::Function<void (bool)>& response,
                                       const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception,
                                       const ::IceInternal::Function<void (bool)>& sent)
{
    class Cpp11CB : public ::IceInternal::Cpp11FnCallbackNC
    {
    public:

        Cpp11CB(const ::std::function<void (bool)>& responseFunc,
                const ::std::function<void (const ::Ice::Exception&)>& exceptionFunc,
                const ::std::function<void (bool)>& sentFunc) :
            ::IceInternal::Cpp11FnCallbackNC(exceptionFunc, sentFunc),
            _response(responseFunc)
        {
            CallbackBase::checkCallback(true, responseFunc || exceptionFunc != nullptr);
        }

        virtual void completed(const ::Ice::AsyncResultPtr& __result) const
        {
            ::Ice::ObjectPrx __proxy = ::Ice::ObjectPrx::uncheckedCast(__result->getProxy());
            bool __ret;
            try
            {
                __ret = __proxy->end_ice_isA(__result);
            }
            catch(const ::Ice::Exception& ex)
            {
                Cpp11FnCallbackNC::exception(__result, ex);
                return;
            }
            if(_response != nullptr)
            {
                _response(__ret);
            }
        }

    private:

        ::std::function<void (bool)> _response;
    };

    return begin_ice_isA(typeId, ctx, new Cpp11CB(response, exception, sent), 0);
}

Ice::AsyncResultPtr
IceProxy::Ice::Object::__begin_ice_id(const ::Ice::Context* ctx,
                                      const ::IceInternal::Function<void (const ::std::string&)>& response,
                                      const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception,
                                      const ::IceInternal::Function<void (bool)>& sent)
{
    class Cpp11CB : public ::IceInternal::Cpp11FnCallbackNC
    {
    public:

        Cpp11CB(const ::std::function<void (const ::std::string&)>& responseFunc,
                const ::std::function<void (const ::Ice::Exception&)>& exceptionFunc,
                const ::std::function<void (bool)>& sentFunc) :
            ::IceInternal::Cpp11FnCallbackNC(exceptionFunc, sentFunc),
            _response(responseFunc)
        {
            CallbackBase::checkCallback(true, responseFunc || exceptionFunc != nullptr);
        }

        virtual void completed(const ::Ice::AsyncResultPtr& __result) const
        {
            ::Ice::ObjectPrx __proxy = ::Ice::ObjectPrx::uncheckedCast(__result->getProxy());
            ::std::string __ret;
            try
            {
                __ret = __proxy->end_ice_id(__result);
            }
            catch(const ::Ice::Exception& ex)
            {
                Cpp11FnCallbackNC::exception(__result, ex);
                return;
            }
            if(_response != nullptr)
            {
                _response(__ret);
            }
        }

    private:

        ::std::function<void (const ::std::string&)> _response;
    };
    return begin_ice_id(ctx, new Cpp11CB(response, exception, sent), 0);
}

Ice::AsyncResultPtr
IceProxy::Ice::Object::__begin_ice_ids(
    const ::Ice::Context* ctx,
    const ::IceInternal::Function<void (const ::std::vector< ::std::string>&)>& response,
    const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception,
    const ::IceInternal::Function<void (bool)>& sent)
{
    class Cpp11CB : public ::IceInternal::Cpp11FnCallbackNC
    {
    public:

        Cpp11CB(const ::std::function<void (const ::std::vector< ::std::string>&)>& responseFunc,
                const ::std::function<void (const ::Ice::Exception&)>& exceptionFunc,
                const ::std::function<void (bool)>& sentFunc) :
            ::IceInternal::Cpp11FnCallbackNC(exceptionFunc, sentFunc),
            _response(responseFunc)
        {
            CallbackBase::checkCallback(true, responseFunc || exceptionFunc != nullptr);
        }

        virtual void completed(const ::Ice::AsyncResultPtr& __result) const
        {
            ::Ice::ObjectPrx __proxy = ::Ice::ObjectPrx::uncheckedCast(__result->getProxy());
            ::std::vector< ::std::string> __ret;
            try
            {
                __ret = __proxy->end_ice_ids(__result);
            }
            catch(const ::Ice::Exception& ex)
            {
                Cpp11FnCallbackNC::exception(__result, ex);
                return;
            }
            if(_response != nullptr)
            {
                _response(__ret);
            }
        }

    private:

        ::std::function<void (const ::std::vector< ::std::string>&)> _response;
    };
    return begin_ice_ids(ctx, new Cpp11CB(response, exception, sent), 0);
}

Ice::AsyncResultPtr
IceProxy::Ice::Object::__begin_ice_invoke(
    const ::std::string& operation,
    ::Ice::OperationMode mode,
    const ::std::vector< ::Ice::Byte>& inParams,
    const ::Ice::Context* ctx,
    const ::IceInternal::Function<void (bool, const ::std::vector< ::Ice::Byte>&)>& response,
    const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception,
    const ::IceInternal::Function<void (bool)>& sent)
{
    class Cpp11CB : public ::IceInternal::Cpp11FnCallbackNC
    {
    public:

        Cpp11CB(const ::std::function<void (bool, const ::std::vector< ::Ice::Byte>&)>& responseFunc,
                const ::std::function<void (const ::Ice::Exception&)>& exceptionFunc,
                const ::std::function<void (bool)>& sentFunc) :
            ::IceInternal::Cpp11FnCallbackNC(exceptionFunc, sentFunc),
            _response(responseFunc)
        {
            CallbackBase::checkCallback(true, responseFunc || exceptionFunc != nullptr);
        }

        virtual void completed(const ::Ice::AsyncResultPtr& __result) const
        {
            ::Ice::ObjectPrx __proxy = ::Ice::ObjectPrx::uncheckedCast(__result->getProxy());
            bool __ret;
            ::std::vector< ::Ice::Byte> p1;
            try
            {
                __ret = __proxy->end_ice_invoke(p1, __result);
            }
            catch(const ::Ice::Exception& ex)
            {
                Cpp11FnCallbackNC::exception(__result, ex);
                return;
            }
            if(_response != nullptr)
            {
                _response(__ret, p1);
            }
        }

    private:

        ::std::function<void (bool, const ::std::vector< ::Ice::Byte>&)> _response;
    };

    return begin_ice_invoke(operation, mode, inParams, ctx, new Cpp11CB(response, exception, sent), 0);
}

Ice::AsyncResultPtr
IceProxy::Ice::Object::__begin_ice_invoke(
    const ::std::string& operation,
    ::Ice::OperationMode mode,
    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
    const ::Ice::Context* ctx,
    const ::IceInternal::Function<void (bool, const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&)>& response,
    const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception,
    const ::IceInternal::Function<void (bool)>& sent)
{
    class Cpp11CB : public ::IceInternal::Cpp11FnCallbackNC
    {
    public:

        Cpp11CB(const ::std::function<void (bool, const ::std::pair<const ::Ice::Byte*,
                                            const ::Ice::Byte*>&)>& responseFunc,
                const ::std::function<void (const ::Ice::Exception&)>& exceptionFunc,
                const ::std::function<void (bool)>& sentFunc) :
            ::IceInternal::Cpp11FnCallbackNC(exceptionFunc, sentFunc),
            _response(responseFunc)
        {
            CallbackBase::checkCallback(true, _response || _exception != nullptr);
        }

        virtual void completed(const ::Ice::AsyncResultPtr& __result) const
        {
            bool __ret;
            ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*> p1;
            try
            {
                __ret = __result->getProxy()->___end_ice_invoke(p1, __result);
            }
            catch(const ::Ice::Exception& ex)
            {
                Cpp11FnCallbackNC::exception(__result, ex);
                return;
            }
            if(_response != nullptr)
            {
                _response(__ret, p1);
            }
        }

    private:

        ::std::function<void (bool, const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&)> _response;
    };
    return begin_ice_invoke(operation, mode, inParams, ctx, new Cpp11CB(response, exception, sent), 0);
}

Ice::AsyncResultPtr
IceProxy::Ice::Object::begin_ice_getConnection(
    const ::IceInternal::Function<void (const ::Ice::ConnectionPtr&)>& response,
    const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception)
{
    class Cpp11CB : public ::IceInternal::Cpp11FnCallbackNC
    {
    public:

        Cpp11CB(const ::std::function<void (const ::Ice::ConnectionPtr&)>& responseFunc,
                const ::std::function<void (const ::Ice::Exception&)>& exceptionFunc) :
            ::IceInternal::Cpp11FnCallbackNC(exceptionFunc, nullptr),
            _response(responseFunc)
        {
            CallbackBase::checkCallback(true, responseFunc || exceptionFunc != nullptr);
        }

        virtual void completed(const ::Ice::AsyncResultPtr& __result) const
        {
            ::Ice::ObjectPrx __proxy = ::Ice::ObjectPrx::uncheckedCast(__result->getProxy());
            ::Ice::ConnectionPtr __ret;
            try
            {
                __ret = __proxy->end_ice_getConnection(__result);
            }
            catch(const ::Ice::Exception& ex)
            {
                Cpp11FnCallbackNC::exception(__result, ex);
                return;
            }
            if(_response != nullptr)
            {
                _response(__ret);
            }
        }

    private:

        ::std::function<void (const ::Ice::ConnectionPtr&)> _response;
    };
    return begin_ice_getConnectionInternal(new Cpp11CB(response, exception), 0);
}

#endif


bool
IceProxy::Ice::Object::end_ice_isA(const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_isA_name);
    bool __ok = __result->__wait();
    if(!__ok)
    {
        try
        {
            __result->__throwUserException();
        }
        catch(const UserException& __ex)
        {
            throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
        }
    }
    bool __ret;
    IceInternal::BasicStream* __is = __result->__startReadParams();
    __is->read(__ret);
    __result->__endReadParams();
    return __ret;
}

void
IceProxy::Ice::Object::ice_ping(const Context* context)
{
    Outgoing __og(this, ice_ping_name, ::Ice::Nonmutating, context);
    __og.writeEmptyParams();
    bool __ok = __og.invoke();
    if(__og.hasResponse())
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        __og.readEmptyParams();
    }
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_ping(const Context* ctx,
                                      const ::IceInternal::CallbackBasePtr& del,
                                      const ::Ice::LocalObjectPtr& cookie)
{
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_ping_name, del, cookie);
    try
    {
        __result->prepare(ice_ping_name, Nonmutating, ctx);
        __result->writeEmptyParams();
        __result->invoke();
    }
    catch(const Exception& __ex)
    {
        __result->abort(__ex);
    }
    return __result;
}

void
IceProxy::Ice::Object::end_ice_ping(const AsyncResultPtr& __result)
{
    __end(__result, ice_ping_name);
}

vector<string>
IceProxy::Ice::Object::ice_ids(const Context* context)
{
    __checkTwowayOnly(ice_ids_name);
    Outgoing __og(this, ice_ids_name, ::Ice::Nonmutating, context);
    __og.writeEmptyParams();
    if(!__og.invoke())
    {
        try
        {
            __og.throwUserException();
        }
        catch(const ::Ice::UserException& __ex)
        {
            throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
        }
    }
    vector<string> __ret;
    BasicStream* __is = __og.startReadParams();
    __is->read(__ret, false);
    __og.endReadParams();
    return __ret;
}

string
IceProxy::Ice::Object::ice_id(const Context* context)
{
    __checkTwowayOnly(ice_id_name);
    Outgoing __og(this, ice_id_name, ::Ice::Nonmutating, context);
    __og.writeEmptyParams();
    if(!__og.invoke())
    {
        try
        {
            __og.throwUserException();
        }
        catch(const ::Ice::UserException& __ex)
        {
            throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
        }
    }
    string __ret;
    BasicStream* __is = __og.startReadParams();
    __is->read(__ret, false);
    __og.endReadParams();
    return __ret;
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_ids(const Context* ctx,
                                     const ::IceInternal::CallbackBasePtr& del,
                                     const ::Ice::LocalObjectPtr& cookie)
{
    __checkAsyncTwowayOnly(ice_ids_name);
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_ids_name, del, cookie);
    try
    {
        __result->prepare(ice_ids_name, Nonmutating, ctx);
        __result->writeEmptyParams();
        __result->invoke();
    }
    catch(const Exception& __ex)
    {
        __result->abort(__ex);
    }
    return __result;
}

vector<string>
IceProxy::Ice::Object::end_ice_ids(const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_ids_name);
    bool __ok = __result->__wait();
    if(!__ok)
    {
        try
        {
            __result->__throwUserException();
        }
        catch(const UserException& __ex)
        {
            throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
        }
    }
    vector<string> __ret;
    IceInternal::BasicStream* __is = __result->__startReadParams();
    __is->read(__ret);
    __result->__endReadParams();
    return __ret;
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_id(const Context* ctx,
                                    const ::IceInternal::CallbackBasePtr& del,
                                    const ::Ice::LocalObjectPtr& cookie)
{
    __checkAsyncTwowayOnly(ice_id_name);
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_id_name, del, cookie);
    try
    {
        __result->prepare(ice_id_name, Nonmutating, ctx);
        __result->writeEmptyParams();
        __result->invoke();
    }
    catch(const Exception& __ex)
    {
        __result->abort(__ex);
    }
    return __result;
}

string
IceProxy::Ice::Object::end_ice_id(const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_id_name);
    bool __ok = __result->__wait();
    if(!__ok)
    {
        try
        {
            __result->__throwUserException();
        }
        catch(const UserException& __ex)
        {
            throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
        }
    }
    string __ret;
    IceInternal::BasicStream* __is = __result->__startReadParams();
    __is->read(__ret);
    __result->__endReadParams();
    return __ret;
}

bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
                                  OperationMode mode,
                                  const vector<Byte>& inEncaps,
                                  vector<Byte>& outEncaps,
                                  const Context* context)
{
    pair<const Byte*, const Byte*> inPair;
    if(inEncaps.empty())
    {
        inPair.first = inPair.second = 0;
    }
    else
    {
        inPair.first = &inEncaps[0];
        inPair.second = inPair.first + inEncaps.size();
    }
    return ice_invoke(operation, mode, inPair, outEncaps, context);
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_invoke(const string& operation,
                                        OperationMode mode,
                                        const vector<Byte>& inEncaps,
                                        const Context* ctx,
                                        const ::IceInternal::CallbackBasePtr& del,
                                        const ::Ice::LocalObjectPtr& cookie)
{
    pair<const Byte*, const Byte*> inPair;
    if(inEncaps.empty())
    {
        inPair.first = inPair.second = 0;
    }
    else
    {
        inPair.first = &inEncaps[0];
        inPair.second = inPair.first + inEncaps.size();
    }
    return begin_ice_invoke(operation, mode, inPair, ctx, del, cookie);
}

bool
IceProxy::Ice::Object::end_ice_invoke(vector<Byte>& outEncaps, const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_invoke_name);
    bool ok = __result->__wait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        const Byte* v;
        Int sz;
        __result->__readParamEncaps(v, sz);
        vector<Byte>(v, v + sz).swap(outEncaps);
    }
    return ok;
}

bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
                                  OperationMode mode,
                                  const pair<const Byte*, const Byte*>& inEncaps,
                                  vector<Byte>& outEncaps,
                                  const Context* context)
{
    Outgoing __og(this, operation, mode, context);
    try
    {
        __og.writeParamEncaps(inEncaps.first, static_cast<Int>(inEncaps.second - inEncaps.first));
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool ok = __og.invoke();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        const Byte* v;
        Int sz;
        __og.readParamEncaps(v, sz);
        vector<Byte>(v, v + sz).swap(outEncaps);
    }
    return ok;
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_invoke(const string& operation,
                                        OperationMode mode,
                                        const pair<const Byte*, const Byte*>& inEncaps,
                                        const Context* ctx,
                                        const ::IceInternal::CallbackBasePtr& del,
                                        const ::Ice::LocalObjectPtr& cookie)
{
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_invoke_name, del, cookie);
    try
    {
        __result->prepare(operation, mode, ctx);
        __result->writeParamEncaps(inEncaps.first, static_cast<Int>(inEncaps.second - inEncaps.first));
        __result->invoke();
    }
    catch(const Exception& __ex)
    {
        __result->abort(__ex);
    }
    return __result;
}

bool
IceProxy::Ice::Object::___end_ice_invoke(pair<const Byte*, const Byte*>& outEncaps, const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_invoke_name);
    bool ok = __result->__wait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        Int sz;
        __result->__readParamEncaps(outEncaps.first, sz);
        outEncaps.second = outEncaps.first + sz;
    }
    return ok;
}

Identity
IceProxy::Ice::Object::ice_getIdentity() const
{
    return _reference->getIdentity();
}

ObjectPrx
IceProxy::Ice::Object::ice_identity(const Identity& newIdentity) const
{
    if(newIdentity.name.empty())
    {
        throw IllegalIdentityException(__FILE__, __LINE__);
    }
    if(newIdentity == _reference->getIdentity())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = new Object;
        proxy->setup(_reference->changeIdentity(newIdentity));
        return proxy;
    }
}

Context
IceProxy::Ice::Object::ice_getContext() const
{
    return _reference->getContext()->getValue();
}

ObjectPrx
IceProxy::Ice::Object::ice_context(const Context& newContext) const
{
    ObjectPrx proxy = __newInstance();
    proxy->setup(_reference->changeContext(newContext));
    return proxy;
}

const string&
IceProxy::Ice::Object::ice_getFacet() const
{
    return _reference->getFacet();
}

ObjectPrx
IceProxy::Ice::Object::ice_facet(const string& newFacet) const
{
    if(newFacet == _reference->getFacet())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = new Object;
        proxy->setup(_reference->changeFacet(newFacet));
        return proxy;
    }
}

string
IceProxy::Ice::Object::ice_getAdapterId() const
{
    return _reference->getAdapterId();
}

ObjectPrx
IceProxy::Ice::Object::ice_adapterId(const string& newAdapterId) const
{
    if(newAdapterId == _reference->getAdapterId())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeAdapterId(newAdapterId));
        return proxy;
    }
}

EndpointSeq
IceProxy::Ice::Object::ice_getEndpoints() const
{
    vector<EndpointIPtr> endpoints = _reference->getEndpoints();
    EndpointSeq retSeq;
    for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        retSeq.push_back(EndpointPtr::dynamicCast(*p));
    }
    return retSeq;
}

ObjectPrx
IceProxy::Ice::Object::ice_endpoints(const EndpointSeq& newEndpoints) const
{
    vector<EndpointIPtr> endpoints;
    for(EndpointSeq::const_iterator p = newEndpoints.begin(); p != newEndpoints.end(); ++p)
    {
        endpoints.push_back(EndpointIPtr::dynamicCast(*p));
    }

    if(endpoints == _reference->getEndpoints())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeEndpoints(endpoints));
        return proxy;
    }
}

Int
IceProxy::Ice::Object::ice_getLocatorCacheTimeout() const
{
    return _reference->getLocatorCacheTimeout();
}

ObjectPrx
IceProxy::Ice::Object::ice_locatorCacheTimeout(Int newTimeout) const
{
    if(newTimeout < -1)
    {
        ostringstream s;
        s << "invalid value passed to ice_locatorCacheTimeout: " << newTimeout;
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, s.str());
    }
    if(newTimeout == _reference->getLocatorCacheTimeout())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeLocatorCacheTimeout(newTimeout));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isConnectionCached() const
{
    return _reference->getCacheConnection();
}

ObjectPrx
IceProxy::Ice::Object::ice_connectionCached(bool newCache) const
{
    if(newCache == _reference->getCacheConnection())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeCacheConnection(newCache));
        return proxy;
    }
}

EndpointSelectionType
IceProxy::Ice::Object::ice_getEndpointSelection() const
{
    return _reference->getEndpointSelection();
}

ObjectPrx
IceProxy::Ice::Object::ice_endpointSelection(EndpointSelectionType newType) const
{
    if(newType == _reference->getEndpointSelection())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeEndpointSelection(newType));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isSecure() const
{
    return _reference->getSecure();
}

ObjectPrx
IceProxy::Ice::Object::ice_secure(bool b) const
{
    if(b == _reference->getSecure())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeSecure(b));
        return proxy;
    }
}

::Ice::EncodingVersion
IceProxy::Ice::Object::ice_getEncodingVersion() const
{
    return _reference->getEncoding();
}

ObjectPrx
IceProxy::Ice::Object::ice_encodingVersion(const ::Ice::EncodingVersion& encoding) const
{
    if(encoding == _reference->getEncoding())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeEncoding(encoding));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isPreferSecure() const
{
    return _reference->getPreferSecure();
}

ObjectPrx
IceProxy::Ice::Object::ice_preferSecure(bool b) const
{
    if(b == _reference->getPreferSecure())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changePreferSecure(b));
        return proxy;
    }
}

RouterPrx
IceProxy::Ice::Object::ice_getRouter() const
{
    RouterInfoPtr ri = _reference->getRouterInfo();
    return ri ? ri->getRouter() : RouterPrx();
}

ObjectPrx
IceProxy::Ice::Object::ice_router(const RouterPrx& router) const
{
    ReferencePtr ref = _reference->changeRouter(router);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

LocatorPrx
IceProxy::Ice::Object::ice_getLocator() const
{
    LocatorInfoPtr ri = _reference->getLocatorInfo();
    return ri ? ri->getLocator() : LocatorPrx();
}

ObjectPrx
IceProxy::Ice::Object::ice_locator(const LocatorPrx& locator) const
{
    ReferencePtr ref = _reference->changeLocator(locator);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isCollocationOptimized() const
{
    return _reference->getCollocationOptimized();
}

ObjectPrx
IceProxy::Ice::Object::ice_collocationOptimized(bool b) const
{
    if(b == _reference->getCollocationOptimized())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeCollocationOptimized(b));
        return proxy;
    }
}

Int
IceProxy::Ice::Object::ice_getInvocationTimeout() const
{
    return _reference->getInvocationTimeout();
}

ObjectPrx
IceProxy::Ice::Object::ice_invocationTimeout(Int newTimeout) const
{
    if(newTimeout < 1 && newTimeout != -1 && newTimeout != -2)
    {
        ostringstream s;
        s << "invalid value passed to ice_invocationTimeout: " << newTimeout;
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, s.str());
    }
    if(newTimeout == _reference->getInvocationTimeout())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeInvocationTimeout(newTimeout));
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_twoway() const
{
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeTwoway));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isTwoway() const
{
    return _reference->getMode() == Reference::ModeTwoway;
}

ObjectPrx
IceProxy::Ice::Object::ice_oneway() const
{
    if(_reference->getMode() == Reference::ModeOneway)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeOneway));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isOneway() const
{
    return _reference->getMode() == Reference::ModeOneway;
}

ObjectPrx
IceProxy::Ice::Object::ice_batchOneway() const
{
    if(_reference->getMode() == Reference::ModeBatchOneway)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeBatchOneway));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isBatchOneway() const
{
    return _reference->getMode() == Reference::ModeBatchOneway;
}

ObjectPrx
IceProxy::Ice::Object::ice_datagram() const
{
    if(_reference->getMode() == Reference::ModeDatagram)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeDatagram));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isDatagram() const
{
    return _reference->getMode() == Reference::ModeDatagram;
}

ObjectPrx
IceProxy::Ice::Object::ice_batchDatagram() const
{
    if(_reference->getMode() == Reference::ModeBatchDatagram)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeBatchDatagram));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isBatchDatagram() const
{
    return _reference->getMode() == Reference::ModeBatchDatagram;
}

ObjectPrx
IceProxy::Ice::Object::ice_compress(bool b) const
{
    ReferencePtr ref = _reference->changeCompress(b);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_timeout(int t) const
{
    if(t < 1 && t != -1)
    {
        ostringstream s;
        s << "invalid value passed to ice_timeout: " << t;
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, s.str());
    }
    ReferencePtr ref = _reference->changeTimeout(t);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_connectionId(const string& id) const
{
    ReferencePtr ref = _reference->changeConnectionId(id);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

string
IceProxy::Ice::Object::ice_getConnectionId() const
{
    return _reference->getConnectionId();
}

ConnectionPtr
IceProxy::Ice::Object::ice_getConnection()
{
    InvocationObserver observer(this, "ice_getConnection", 0);
    int cnt = 0;
    while(true)
    {
        RequestHandlerPtr handler;
        try
        {
            handler = __getRequestHandler();
            return handler->waitForConnection(); // Wait for the connection to be established.
        }
        catch(const IceInternal::RetryException&)
        {
            __updateRequestHandler(handler, 0); // Clear request handler and retry.
        }
        catch(const Exception& ex)
        {
            try
            {
                int interval = __handleException(ex, handler, Idempotent, false, cnt);
                observer.retried();
                if(interval > 0)
                {
                    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(interval));
                }
            }
            catch(const Exception& exc)
            {
                observer.failed(exc.ice_name());
                throw;
            }
        }
    }
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_getConnectionInternal(const ::IceInternal::CallbackBasePtr& del,
                                                       const ::Ice::LocalObjectPtr& cookie)
{
    ProxyGetConnectionPtr result = new ProxyGetConnection(this, ice_getConnection_name, del, cookie);
    try
    {
        result->invoke();
    }
    catch(const Exception& ex)
    {
        result->abort(ex);
    }
    return result;
}

ConnectionPtr
IceProxy::Ice::Object::end_ice_getConnection(const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_getConnection_name);
    __result->__wait();
    return ice_getCachedConnection();
}

ConnectionPtr
IceProxy::Ice::Object::ice_getCachedConnection() const
{
    RequestHandlerPtr __handler;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        __handler =  _requestHandler;
    }

    if(__handler)
    {
        try
        {
            return __handler->getConnection();
        }
        catch(const LocalException&)
        {
        }
    }
    return 0;
}

void
IceProxy::Ice::Object::ice_flushBatchRequests()
{
    ProxyFlushBatch og(this, ice_flushBatchRequests_name);
    og.invoke();
}

::Ice::AsyncResultPtr
IceProxy::Ice::Object::begin_ice_flushBatchRequestsInternal(const ::IceInternal::CallbackBasePtr& del,
                                                            const ::Ice::LocalObjectPtr& cookie)
{
    ProxyFlushBatchAsyncPtr result = new ProxyFlushBatchAsync(this, ice_flushBatchRequests_name, del, cookie);
    try
    {
        result->invoke();
    }
    catch(const Exception& ex)
    {
        result->abort(ex);
    }
    return result;
}

void
IceProxy::Ice::Object::end_ice_flushBatchRequests(const AsyncResultPtr& result)
{
    AsyncResult::__check(result, this, ice_flushBatchRequests_name);
    result->__wait();
}

Int
IceProxy::Ice::Object::__hash() const
{
    return _reference->hash();
}

void
IceProxy::Ice::Object::__copyFrom(const ObjectPrx& from)
{
    IceUtil::Mutex::Lock sync(from->_mutex);
    _reference = from->_reference;
    _requestHandler = from->_requestHandler;
}

int
IceProxy::Ice::Object::__handleException(const Exception& ex,
                                         const RequestHandlerPtr& handler,
                                         OperationMode mode,
                                         bool sent,
                                         int& cnt)
{
    __updateRequestHandler(handler, 0); // Clear the request handler

    //
    // We only retry local exception, system exceptions aren't retried.
    //
    // A CloseConnectionException indicates graceful server shutdown, and is therefore
    // always repeatable without violating "at-most-once". That's because by sending a
    // close connection message, the server guarantees that all outstanding requests
    // can safely be repeated.
    //
    // An ObjectNotExistException can always be retried as well without violating
    // "at-most-once" (see the implementation of the checkRetryAfterException method
    //  of the ProxyFactory class for the reasons why it can be useful).
    //
    // If the request didn't get sent or if it's non-mutating or idempotent it can
    // also always be retried if the retry count isn't reached.
    //
    const LocalException* localEx = dynamic_cast<const LocalException*>(&ex);
    if(localEx && (!sent ||
                   mode == Nonmutating || mode == Idempotent ||
                   dynamic_cast<const CloseConnectionException*>(&ex) ||
                   dynamic_cast<const ObjectNotExistException*>(&ex)))
    {
        try
        {
            return _reference->getInstance()->proxyFactory()->checkRetryAfterException(*localEx, _reference, cnt);
        }
        catch(const CommunicatorDestroyedException&)
        {
            //
            // The communicator is already destroyed, so we cannot retry.
            //
            ex.ice_throw();
        }
    }
    else
    {
        ex.ice_throw(); // Retry could break at-most-once semantics, don't retry.
    }
    return 0; // Keep the compiler happy.
}

void
IceProxy::Ice::Object::__checkTwowayOnly(const string& name) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this operation.
    //
    if(!ice_isTwoway())
    {
        TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = name;
        throw ex;
    }
}

void
IceProxy::Ice::Object::__checkAsyncTwowayOnly(const string& name) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this operation.
    //
    if(!ice_isTwoway())
    {
        throw IceUtil::IllegalArgumentException(__FILE__,
                                                __LINE__,
                                                "`" + name + "' can only be called with a twoway proxy");
    }
}

void
IceProxy::Ice::Object::__invoke(Outgoing& __og) const
{
    //
    // Helper for operations without out/return parameters and user
    // exceptions.
    //

    bool __ok = __og.invoke();
    if(__og.hasResponse())
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        __og.readEmptyParams();
    }
}

void
IceProxy::Ice::Object::__end(const ::Ice::AsyncResultPtr& __result, const std::string& operation) const
{
    AsyncResult::__check(__result, this, operation);
    bool __ok = __result->__wait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        if(!__ok)
        {
            try
            {
                __result->__throwUserException();
            }
            catch(const UserException& __ex)
            {
                throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        __result->__readEmptyParams();
    }
}

namespace IceProxy
{

namespace Ice
{

ostream&
operator<<(ostream& os, const ::IceProxy::Ice::Object& p)
{
    return os << p.ice_toString();
}

}

}

::IceInternal::RequestHandlerPtr
IceProxy::Ice::Object::__getRequestHandler()
{
    RequestHandlerPtr handler;
    if(_reference->getCacheConnection())
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(_requestHandler)
        {
            return _requestHandler;
        }
    }
    return _reference->getRequestHandler(this);
}

IceInternal::BatchRequestQueuePtr
IceProxy::Ice::Object::__getBatchRequestQueue()
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_batchRequestQueue)
    {
        _batchRequestQueue = _reference->getBatchRequestQueue();
    }
    return _batchRequestQueue;
}

::IceInternal::RequestHandlerPtr
IceProxy::Ice::Object::__setRequestHandler(const ::IceInternal::RequestHandlerPtr& handler)
{
    if(_reference->getCacheConnection())
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(!_requestHandler)
        {
            _requestHandler = handler;
        }
        return _requestHandler;
    }
    return handler;
}

void
IceProxy::Ice::Object::__updateRequestHandler(const ::IceInternal::RequestHandlerPtr& previous,
                                              const ::IceInternal::RequestHandlerPtr& handler)
{
    if(_reference->getCacheConnection() && previous)
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(_requestHandler && _requestHandler.get() != handler.get())
        {
            //
            // Update the request handler only if "previous" is the same
            // as the current request handler. This is called after
            // connection binding by the connect request handler. We only
            // replace the request handler if the current handler is the
            // connect request handler.
            //
            _requestHandler = _requestHandler->update(previous, handler);
        }
    }
}

IceProxy::Ice::Object*
IceProxy::Ice::Object::__newInstance() const
{
    return new Object;
}

void
IceProxy::Ice::Object::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_requestHandler);

    _reference = ref;
}

bool
Ice::proxyIdentityLess(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return false;
    }
    else if(!lhs && rhs)
    {
        return true;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        return lhs->ice_getIdentity() < rhs->ice_getIdentity();
    }
}

bool
Ice::proxyIdentityEqual(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return true;
    }
    else if(!lhs && rhs)
    {
        return false;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        return lhs->ice_getIdentity() == rhs->ice_getIdentity();
    }
}

bool
Ice::proxyIdentityAndFacetLess(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return false;
    }
    else if(!lhs && rhs)
    {
        return true;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();

        if(lhsIdentity < rhsIdentity)
        {
            return true;
        }
        else if(rhsIdentity < lhsIdentity)
        {
            return false;
        }

        string lhsFacet = lhs->ice_getFacet();
        string rhsFacet = rhs->ice_getFacet();

        if(lhsFacet < rhsFacet)
        {
            return true;
        }
        else if(rhsFacet < lhsFacet)
        {
            return false;
        }

        return false;
    }
}

bool
Ice::proxyIdentityAndFacetEqual(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return true;
    }
    else if(!lhs && rhs)
    {
        return false;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();

        if(lhsIdentity == rhsIdentity)
        {
            string lhsFacet = lhs->ice_getFacet();
            string rhsFacet = rhs->ice_getFacet();

            if(lhsFacet == rhsFacet)
            {
                return true;
            }
        }

        return false;
    }
}

void
Ice::ice_writeObjectPrx(const OutputStreamPtr& out, const ObjectPrx& v)
{
    out->write(v);
}

void
Ice::ice_readObjectPrx(const InputStreamPtr& in, ObjectPrx& v)
{
    in->read(v);
}
