// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROXY_H
#define ICE_PROXY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ProxyF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/RequestHandlerF.h>
#include <Ice/EndpointF.h>
#include <Ice/EndpointTypes.h>
#include <Ice/Object.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ReferenceF.h>
#include <Ice/BatchRequestQueueF.h>
#include <Ice/AsyncResult.h>
//#include <Ice/RouterF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
//#include <Ice/LocatorF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
#include <Ice/Current.h>
#include <Ice/CommunicatorF.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/LocalException.h>
#include <iosfwd>

namespace Ice
{

ICE_API extern const Context noExplicitContext;

}

namespace IceInternal
{

//
// Class for handling the proxy's begin_ice_flushBatchRequest request.
//
class ICE_API ProxyFlushBatchAsync : public ProxyOutgoingAsyncBase
{
public:

    ProxyFlushBatchAsync(const Ice::ObjectPrxPtr&);

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void invoke(const std::string&);

private:

    int _batchRequestNum;
};
typedef IceUtil::Handle<ProxyFlushBatchAsync> ProxyFlushBatchAsyncPtr;

//
// Class for handling the proxy's begin_ice_getConnection request.
//
class ICE_API ProxyGetConnection :  public ProxyOutgoingAsyncBase
{
public:

    ProxyGetConnection(const Ice::ObjectPrxPtr&);

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    virtual Ice::ConnectionPtr getConnection() const;

    void invoke(const std::string&);
};
typedef IceUtil::Handle<ProxyGetConnection> ProxyGetConnectionPtr;

}

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace IceInternal
{

template<typename P>
::std::shared_ptr<P> createProxy()
{
    return ::std::shared_ptr<P>(new P());
}

inline ::std::pair<const Ice::Byte*, const Ice::Byte*>
makePair(const Ice::ByteSeq& seq)
{
    if(seq.empty())
    {
        return { nullptr, nullptr };
    }
    else
    {
        return { seq.data(), seq.data() + seq.size() };
    }
}

template<typename R>
class InvokeOutgoingAsyncT : public OutgoingAsync
{
public:

    using OutgoingAsync::OutgoingAsync;

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
           const Ice::Context& context)
    {
        _read = [](bool ok, Ice::InputStream* stream)
        {
            const ::Ice::Byte* encaps;
            ::Ice::Int sz;
            stream->readEncapsulation(encaps, sz);
            return R { ok, { encaps, encaps + sz } };
        };

        try
        {
            prepare(operation, mode, context);
            if(inParams.first == inParams.second)
            {
                _os.writeEmptyEncapsulation(_encoding);
            }
            else
            {
                _os.writeEncapsulation(inParams.first, static_cast<Ice::Int>(inParams.second - inParams.first));
            }
            OutgoingAsync::invoke(operation);
        }
        catch(const Ice::Exception& ex)
        {
            abort(ex);
        }
    }

protected:

    std::function<R(bool, Ice::InputStream*)> _read;
};

template<typename R>
class InvokeLambdaOutgoing : public InvokeOutgoingAsyncT<R>, public LambdaInvoke
{
public:

    InvokeLambdaOutgoing(const ::std::shared_ptr<::Ice::ObjectPrx>& proxy,
                         ::std::function<void(R)> response,
                         ::std::function<void(::std::exception_ptr)> ex,
                         ::std::function<void(bool)> sent) :
        InvokeOutgoingAsyncT<R>(proxy, false), LambdaInvoke(::std::move(ex), ::std::move(sent))
    {
        if(response)
        {
            _response = [this, response](bool ok)
            {
                if(this->_is.b.empty())
                {
                    response(R { ok, { 0, 0 }});
                }
                else
                {
                    response(this->_read(ok, &this->_is));
                }
            };
        }
    }
};

template<typename P, typename R>
class InvokePromiseOutgoing : public InvokeOutgoingAsyncT<R>, public PromiseInvoke<P>
{
public:

    InvokePromiseOutgoing(const std::shared_ptr<Ice::ObjectPrx>& proxy, bool synchronous) :
        InvokeOutgoingAsyncT<R>(proxy, false)
    {
        this->_synchronous = synchronous;
        this->_response = [this](bool ok)
        {
            if(this->_is.b.empty())
            {
                this->_promise.set_value(R { ok, { 0, 0 }});
            }
            else
            {
                this->_promise.set_value(this->_read(ok, &this->_is));
            }
        };
    }

    virtual bool handleSent(bool done, bool) override
    {
        if(done)
        {
            this->_promise.set_value(R { true, { 0, 0 }});
        }
        return false;
    }
};

class ProxyGetConnectionLambda : public ProxyGetConnection, public LambdaInvoke
{
public:

    ProxyGetConnectionLambda(const ::std::shared_ptr<::Ice::ObjectPrx>& proxy,
                             ::std::function<void(::std::shared_ptr<Ice::Connection>)> response,
                             ::std::function<void(::std::exception_ptr)> ex,
                             ::std::function<void(bool)> sent) :
        ProxyGetConnection(proxy), LambdaInvoke(::std::move(ex), ::std::move(sent))
    {
        _response = [&, response](bool)
        {
            response(getConnection());
        };
    }
};

template<typename P>
class ProxyGetConnectionPromise : public ProxyGetConnection, public PromiseInvoke<P>
{
public:

    ProxyGetConnectionPromise(const ::std::shared_ptr<::Ice::ObjectPrx>& proxy) : ProxyGetConnection(proxy)
    {
        this->_response = [&](bool)
        {
            this->_promise.set_value(getConnection());
        };
    }
};

class ProxyFlushBatchLambda : public ProxyFlushBatchAsync, public LambdaInvoke
{
public:

    ProxyFlushBatchLambda(const ::std::shared_ptr<::Ice::ObjectPrx>& proxy,
                          ::std::function<void(::std::exception_ptr)> ex,
                          ::std::function<void(bool)> sent) :
        ProxyFlushBatchAsync(proxy), LambdaInvoke(::std::move(ex), ::std::move(sent))
    {
    }
};

template<typename P>
class ProxyFlushBatchPromise : public ProxyFlushBatchAsync, public PromiseInvoke<P>
{
public:

    using ProxyFlushBatchAsync::ProxyFlushBatchAsync;

    virtual bool handleSent(bool, bool) override
    {
        this->_promise.set_value(_sentSynchronously);
        return false;
    }
};

}

namespace Ice
{

class RouterPrx;
using RouterPrxPtr = ::std::shared_ptr<::Ice::RouterPrx>;

class LocatorPrx;
using LocatorPrxPtr = ::std::shared_ptr<::Ice::LocatorPrx>;

class LocalException;
class OutputStream;

class ICE_API ObjectPrx : public ::std::enable_shared_from_this<ObjectPrx>
{
public:

    virtual ~ObjectPrx() = default;

    friend ICE_API bool operator<(const ObjectPrx&, const ObjectPrx&);
    friend ICE_API bool operator==(const ObjectPrx&, const ObjectPrx&);

    ::std::shared_ptr<::Ice::Communicator> ice_getCommunicator() const;

    ::std::string ice_toString() const;

    bool
    ice_isA(const ::std::string& typeId, const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makePromiseOutgoing<bool>(true, this, &ObjectPrx::_iceI_isA, typeId, context).get();
    }

    ::std::function<void()>
    ice_isAAsync(const ::std::string& typeId,
                 ::std::function<void(bool)> response,
                 ::std::function<void(::std::exception_ptr)> ex = nullptr,
                 ::std::function<void(bool)> sent = nullptr,
                 const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makeLamdaOutgoing<bool>(response, ex, sent, this, &ObjectPrx::_iceI_isA, typeId, context);
    }

    template<template<typename> class P = std::promise> auto
    ice_isAAsync(const ::std::string& typeId, const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<bool>>().get_future())
    {
        return _makePromiseOutgoing<bool, P>(false, this, &ObjectPrx::_iceI_isA, typeId, context);
    }

    void
    _iceI_isA(const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<bool>>&, const ::std::string&, const ::Ice::Context&);

    void
    ice_ping(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        _makePromiseOutgoing<void>(true, this, &ObjectPrx::_iceI_ping, context).get();
    }

    ::std::function<void()>
    ice_pingAsync(::std::function<void()> response,
                  ::std::function<void(::std::exception_ptr)> ex = nullptr,
                  ::std::function<void(bool)> sent = nullptr,
                  const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makeLamdaOutgoing<void>(response, ex, sent, this, &ObjectPrx::_iceI_ping, context);
    }

    template<template<typename> class P = std::promise>
    auto ice_pingAsync(const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<void>>().get_future())
    {
        return _makePromiseOutgoing<void, P>(false, this, &ObjectPrx::_iceI_ping, context);
    }

    void
    _iceI_ping(const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<void>>&, const ::Ice::Context&);

    ::std::vector<::std::string>
    ice_ids(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makePromiseOutgoing<::std::vector<::std::string>>(true, this, &ObjectPrx::_iceI_ids, context).get();
    }

    ::std::function<void()>
    ice_idsAsync(::std::function<void(::std::vector<::std::string>)> response,
                 ::std::function<void(::std::exception_ptr)> ex = nullptr,
                 ::std::function<void(bool)> sent = nullptr,
                 const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makeLamdaOutgoing<::std::vector<::std::string>>(response, ex, sent, this, &ObjectPrx::_iceI_ids, context);
    }

    template<template<typename> class P = std::promise> auto
    ice_idsAsync(const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<::std::vector<::std::string>>>().get_future())
    {
        return _makePromiseOutgoing<::std::vector<::std::string>, P>(false, this, &ObjectPrx::_iceI_ids, context);
    }

    void
    _iceI_ids(const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<::std::vector<::std::string>>>&, const ::Ice::Context&);

    ::std::string
    ice_id(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makePromiseOutgoing<::std::string>(true, this, &ObjectPrx::_iceI_id, context).get();
    }

    ::std::function<void()>
    ice_idAsync(::std::function<void(::std::string)> response,
                ::std::function<void(::std::exception_ptr)> ex = nullptr,
                ::std::function<void(bool)> sent = nullptr,
                const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makeLamdaOutgoing<::std::string>(response, ex, sent, this, &ObjectPrx::_iceI_id, context);
    }

    template<template<typename> class P = std::promise>
    auto ice_idAsync(const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<::std::string>>().get_future())
    {
        return _makePromiseOutgoing<::std::string, P>(false, this, &ObjectPrx::_iceI_id, context);
    }

    void
    _iceI_id(const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<::std::string>>&, const ::Ice::Context&);

    static const ::std::string& ice_staticId()
    {
        return ::Ice::Object::ice_staticId();
    }


    //
    // ice_invoke with default vector mapping for byte-sequence parameters
    //

    bool
    ice_invoke(const ::std::string& operation,
               ::Ice::OperationMode mode,
               const ::std::vector<Byte>& inP,
               ::std::vector<::Ice::Byte>& outParams,
               const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return ice_invoke(operation, mode, ::IceInternal::makePair(inP), outParams, context);
    }

    template<template<typename> class P = std::promise> auto
    ice_invokeAsync(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::vector<Byte>& inP,
                    const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<::Ice::Object::Ice_invokeResult>>().get_future())
    {
        return ice_invokeAsync<P>(operation, mode, ::IceInternal::makePair(inP), context);
    }

    ::std::function<void()>
    ice_invokeAsync(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::vector<::Ice::Byte>& inP,
                    ::std::function<void(bool, ::std::vector<::Ice::Byte>)> response,
                    ::std::function<void(::std::exception_ptr)> ex = nullptr,
                    ::std::function<void(bool)> sent = nullptr,
                    const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        using Outgoing = ::IceInternal::InvokeLambdaOutgoing<::Ice::Object::Ice_invokeResult>;
        ::std::function<void(::Ice::Object::Ice_invokeResult&&)> r;
        if(response)
        {
            r = [response](::Ice::Object::Ice_invokeResult&& result)
            {
                response(result.returnValue, std::move(result.outParams));
            };
        }
        auto outAsync = ::std::make_shared<Outgoing>(shared_from_this(), r, ex, sent);
        outAsync->invoke(operation, mode, ::IceInternal::makePair(inP), context);
        return [outAsync]() { outAsync->cancel(); };
    }


    //
    // ice_invoke with cpp:array mapping for byte sequence parameters
    //

    bool
    ice_invoke(const ::std::string& operation,
               ::Ice::OperationMode mode,
               const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inP,
               ::std::vector<::Ice::Byte>& outParams,
               const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        using Outgoing = ::IceInternal::InvokePromiseOutgoing<
            ::std::promise<::Ice::Object::Ice_invokeResult>, ::Ice::Object::Ice_invokeResult>;
        auto outAsync = ::std::make_shared<Outgoing>(shared_from_this(), true);
        outAsync->invoke(operation, mode, inP, context);
        auto result = outAsync->getFuture().get();
        outParams.swap(result.outParams);
        return result.returnValue;
    }

    template<template<typename> class P = std::promise> auto
    ice_invokeAsync(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inP,
                    const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<::Ice::Object::Ice_invokeResult>>().get_future())
    {
        using Outgoing =
            ::IceInternal::InvokePromiseOutgoing<P<::Ice::Object::Ice_invokeResult>, ::Ice::Object::Ice_invokeResult>;
        auto outAsync = ::std::make_shared<Outgoing>(shared_from_this(), false);
        outAsync->invoke(operation, mode, inP, context);
        return outAsync->getFuture();
    }

    ::std::function<void()>
    ice_invokeAsync(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inP,
                    ::std::function<void(bool, ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>)> response,
                    ::std::function<void(::std::exception_ptr)> ex = nullptr,
                    ::std::function<void(bool)> sent = nullptr,
                    const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        using Result = ::std::tuple<bool, ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>>;
        using Outgoing = ::IceInternal::InvokeLambdaOutgoing<Result>;

        ::std::function<void(Result&&)> r;
        if(response)
        {
            r = [response](Result&& result)
            {
                response(::std::get<0>(result), ::std::move(::std::get<1>(result)));
            };
        }
        auto outAsync = ::std::make_shared<Outgoing>(shared_from_this(), r, ex, sent);
        outAsync->invoke(operation, mode, inP, context);
        return [outAsync]() { outAsync->cancel(); };
    }


    ::Ice::Identity ice_getIdentity() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_identity(const ::Ice::Identity&) const;

    ::Ice::Context ice_getContext() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_context(const ::Ice::Context&) const;

    const ::std::string& ice_getFacet() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_facet(const ::std::string&) const;

    ::std::string ice_getAdapterId() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_adapterId(const ::std::string&) const;

    ::Ice::EndpointSeq ice_getEndpoints() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_endpoints(const ::Ice::EndpointSeq&) const;

    ::Ice::Int ice_getLocatorCacheTimeout() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_locatorCacheTimeout(::Ice::Int) const;

    bool ice_isConnectionCached() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_connectionCached(bool) const;

    ::Ice::EndpointSelectionType ice_getEndpointSelection() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_endpointSelection(::Ice::EndpointSelectionType) const;

    bool ice_isSecure() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_secure(bool) const;

    ::Ice::EncodingVersion ice_getEncodingVersion() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_encodingVersion(const ::Ice::EncodingVersion&) const;

    bool ice_isPreferSecure() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_preferSecure(bool) const;

    ::std::shared_ptr<::Ice::RouterPrx> ice_getRouter() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_router(const ::std::shared_ptr<::Ice::RouterPrx>&) const;

    ::std::shared_ptr<::Ice::LocatorPrx> ice_getLocator() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_locator(const ::std::shared_ptr<::Ice::LocatorPrx>&) const;

    bool ice_isCollocationOptimized() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_collocationOptimized(bool) const;

    ::Ice::Int ice_getInvocationTimeout() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_invocationTimeout(::Ice::Int) const;

    ::std::shared_ptr<::Ice::ObjectPrx> ice_twoway() const;
    bool ice_isTwoway() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_oneway() const;
    bool ice_isOneway() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_batchOneway() const;
    bool ice_isBatchOneway() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_datagram() const;
    bool ice_isDatagram() const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_batchDatagram() const;
    bool ice_isBatchDatagram() const;

    ::std::shared_ptr<::Ice::ObjectPrx> ice_compress(bool) const;
    ::std::shared_ptr<::Ice::ObjectPrx> ice_timeout(int) const;

    ::std::shared_ptr<::Ice::ObjectPrx> ice_connectionId(const ::std::string&) const;
    ::std::string ice_getConnectionId() const;

    ::std::shared_ptr<::Ice::Connection>
    ice_getConnection()
    {
        return ice_getConnectionAsync().get();
    }

    ::std::function<void()>
    ice_getConnectionAsync(::std::function<void(::std::shared_ptr<::Ice::Connection>)> response,
                           ::std::function<void(::std::exception_ptr)> ex = nullptr,
                           ::std::function<void(bool)> sent = nullptr)
    {
        using LambdaOutgoing = ::IceInternal::ProxyGetConnectionLambda;
        auto outAsync = ::std::make_shared<LambdaOutgoing>(shared_from_this(), response, ex, sent);
        _iceI_getConnection(outAsync);
        return [outAsync]() { outAsync->cancel(); };
    }

    template<template<typename> class P = std::promise> auto
    ice_getConnectionAsync() -> decltype(std::declval<P<::std::shared_ptr<::Ice::Connection>>>().get_future())
    {
        using PromiseOutgoing = ::IceInternal::ProxyGetConnectionPromise<P<::std::shared_ptr<::Ice::Connection>>>;
        auto outAsync = ::std::make_shared<PromiseOutgoing>(shared_from_this());
        _iceI_getConnection(outAsync);
        return outAsync->getFuture();
    }

    void _iceI_getConnection(const ::std::shared_ptr<::IceInternal::ProxyGetConnection>&);

    ::std::shared_ptr<::Ice::Connection> ice_getCachedConnection() const;

    bool ice_flushBatchRequests()
    {
        return ice_flushBatchRequestsAsync().get();
    }

    std::function<void()>
    ice_flushBatchRequestsAsync(::std::function<void(::std::exception_ptr)> ex,
                                ::std::function<void(bool)> sent = nullptr)
    {
        using LambdaOutgoing = ::IceInternal::ProxyFlushBatchLambda;
        auto outAsync = ::std::make_shared<LambdaOutgoing>(shared_from_this(), ex, sent);
        _iceI_flushBatchRequests(outAsync);
        return [outAsync]() { outAsync->cancel(); };
    }

    template<template<typename> class P = std::promise> auto
    ice_flushBatchRequestsAsync() -> decltype(std::declval<P<bool>>().get_future())
    {
        using PromiseOutgoing = ::IceInternal::ProxyFlushBatchPromise<P<bool>>;
        auto outAsync = ::std::make_shared<PromiseOutgoing>(shared_from_this());
        _iceI_flushBatchRequests(outAsync);
        return outAsync->getFuture();
    }

    void _iceI_flushBatchRequests(const ::std::shared_ptr<::IceInternal::ProxyFlushBatchAsync>&);

    const ::IceInternal::ReferencePtr& _getReference() const { return _reference; }

    void _copyFrom(const std::shared_ptr<::Ice::ObjectPrx>&);

    int _handleException(const ::Ice::Exception&, const ::IceInternal::RequestHandlerPtr&, ::Ice::OperationMode,
                          bool, int&);

    void _checkTwowayOnly(const ::std::string&) const;

    ::IceInternal::RequestHandlerPtr _getRequestHandler();
    ::IceInternal::BatchRequestQueuePtr _getBatchRequestQueue();
    ::IceInternal::RequestHandlerPtr _setRequestHandler(const ::IceInternal::RequestHandlerPtr&);
    void _updateRequestHandler(const ::IceInternal::RequestHandlerPtr&, const ::IceInternal::RequestHandlerPtr&);

    int _hash() const;

    void _write(OutputStream&) const;

protected:

    template<typename R, template<typename> class P = ::std::promise, typename Obj, typename Fn, typename... Args>
    auto _makePromiseOutgoing(bool sync, Obj obj, Fn fn, Args&&... args)
        -> decltype(std::declval<P<R>>().get_future())
    {
        auto outAsync = ::std::make_shared<::IceInternal::PromiseOutgoing<P<R>, R>>(shared_from_this(), sync);
        (obj->*fn)(outAsync, std::forward<Args>(args)...);
        return outAsync->getFuture();
    }

    template<typename R, typename Re, typename E, typename S, typename Obj, typename Fn, typename... Args>
    ::std::function<void()> _makeLamdaOutgoing(Re r, E e, S s, Obj obj, Fn fn, Args&&... args)
    {
        auto outAsync = ::std::make_shared<::IceInternal::LambdaOutgoing<R>>(shared_from_this(), r, e, s);
        (obj->*fn)(outAsync, std::forward<Args>(args)...);
        return [outAsync]() { outAsync->cancel(); };
    }

    virtual ::std::shared_ptr<ObjectPrx> _newInstance() const;
    ObjectPrx() = default;
    friend ::std::shared_ptr<ObjectPrx> IceInternal::createProxy<ObjectPrx>();

private:

    void setup(const ::IceInternal::ReferencePtr&);
    friend class ::IceInternal::ProxyFactory;

    ::IceInternal::ReferencePtr _reference;
    ::IceInternal::RequestHandlerPtr _requestHandler;
    ::IceInternal::BatchRequestQueuePtr _batchRequestQueue;
    IceUtil::Mutex _mutex;
};

inline bool
operator>(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return rhs < lhs;
}

inline bool
operator<=(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return !(lhs > rhs);
}

inline bool
operator>=(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return !(lhs < rhs);
}

inline bool
operator!=(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return !(lhs == rhs);
}

template<typename Prx, typename... Bases>
class Proxy : public virtual Bases...
{
public:

    ::std::shared_ptr<Prx> ice_context(const ::Ice::Context& context) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_context(context));
    }

    ::std::shared_ptr<Prx> ice_adapterId(const ::std::string& id) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_adapterId(id));
    }

    ::std::shared_ptr<Prx> ice_endpoints(const ::Ice::EndpointSeq& endpoints) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_endpoints(endpoints));
    }

    ::std::shared_ptr<Prx> ice_locatorCacheTimeout(int timeout) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_locatorCacheTimeout(timeout));
    }

    ::std::shared_ptr<Prx> ice_connectionCached(bool cached) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_connectionCached(cached));
    }

    ::std::shared_ptr<Prx> ice_endpointSelection(::Ice::EndpointSelectionType selection) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_endpointSelection(selection));
    }

    ::std::shared_ptr<Prx> ice_secure(bool secure) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_secure(secure));
    }

    ::std::shared_ptr<Prx> ice_preferSecure(bool preferSecure) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_preferSecure(preferSecure));
    }

    ::std::shared_ptr<Prx> ice_router(const ::std::shared_ptr<::Ice::RouterPrx>& router) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_router(router));
    }

    ::std::shared_ptr<Prx> ice_locator(const ::std::shared_ptr<::Ice::LocatorPrx>& locator) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_locator(locator));
    }

    ::std::shared_ptr<Prx> ice_collocationOptimized(bool collocated) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_collocationOptimized(collocated));
    }

    ::std::shared_ptr<Prx> ice_invocationTimeout(int timeout) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_invocationTimeout(timeout));
    }

    ::std::shared_ptr<Prx> ice_twoway() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_twoway());
    }

    ::std::shared_ptr<Prx> ice_oneway() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_oneway());
    }

    ::std::shared_ptr<Prx> ice_batchOneway() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_batchOneway());
    }

    ::std::shared_ptr<Prx> ice_datagram() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_datagram());
    }

    ::std::shared_ptr<Prx> ice_batchDatagram() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_batchDatagram());
    }

    ::std::shared_ptr<Prx> ice_compress(bool compress) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_compress(compress));
    }

    ::std::shared_ptr<Prx> ice_timeout(int timeout) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_timeout(timeout));
    }

    ::std::shared_ptr<Prx> ice_connectionId(const ::std::string& id) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_connectionId(id));
    }

    ::std::shared_ptr<Prx> ice_encodingVersion(const ::Ice::EncodingVersion& version) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_encodingVersion(version));
    }

protected:

    virtual ::std::shared_ptr<ObjectPrx> _newInstance() const = 0;
};

ICE_API ::std::ostream& operator<<(::std::ostream&, const ::Ice::ObjectPrx&);

ICE_API bool proxyIdentityLess(const ::std::shared_ptr<ObjectPrx>&, const ::std::shared_ptr<ObjectPrx>&);
ICE_API bool proxyIdentityEqual(const ::std::shared_ptr<ObjectPrx>&, const ::std::shared_ptr<ObjectPrx>&);

ICE_API bool proxyIdentityAndFacetLess(const ::std::shared_ptr<ObjectPrx>&, const ::std::shared_ptr<ObjectPrx>&);
ICE_API bool proxyIdentityAndFacetEqual(const ::std::shared_ptr<ObjectPrx>&, const ::std::shared_ptr<ObjectPrx>&);

struct ProxyIdentityLess : std::binary_function<bool, ::std::shared_ptr<ObjectPrx>&, ::std::shared_ptr<ObjectPrx>&>
{
    bool operator()(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs) const
    {
        return proxyIdentityLess(lhs, rhs);
    }
};

struct ProxyIdentityEqual : std::binary_function<bool, ::std::shared_ptr<ObjectPrx>&, ::std::shared_ptr<ObjectPrx>&>
{
    bool operator()(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs) const
    {
        return proxyIdentityEqual(lhs, rhs);
    }
};

struct ProxyIdentityAndFacetLess : std::binary_function<bool, ::std::shared_ptr<ObjectPrx>&, ::std::shared_ptr<ObjectPrx>&>
{
    bool operator()(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs) const
    {
        return proxyIdentityAndFacetLess(lhs, rhs);
    }
};

struct ProxyIdentityAndFacetEqual : std::binary_function<bool, ::std::shared_ptr<ObjectPrx>&, ::std::shared_ptr<ObjectPrx>&>
{
    bool operator()(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs) const
    {
        return proxyIdentityAndFacetEqual(lhs, rhs);
    }
};

template<typename P,
         typename T,
         typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, P>::value>::type* = nullptr,
         typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, T>::value>::type* = nullptr> ::std::shared_ptr<P>
uncheckedCast(const ::std::shared_ptr<T>& b)
{
    ::std::shared_ptr<P> r;
    if(b)
    {
        r = ::std::dynamic_pointer_cast<P>(b);
        if(!r)
        {
            r = IceInternal::createProxy<P>();
            r->_copyFrom(b);
        }
    }
    return r;
}

template<typename P,
         typename T,
         typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, P>::value>::type* = nullptr,
         typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, T>::value>::type* = nullptr> ::std::shared_ptr<P>
uncheckedCast(const ::std::shared_ptr<T>& b, const std::string& f)
{
    ::std::shared_ptr<P> r;
    if(b)
    {
        r = IceInternal::createProxy<P>();
        r->_copyFrom(b->ice_facet(f));
    }
    return r;
}

template<typename P,
         typename T,
         typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, P>::value>::type* = nullptr,
         typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, T>::value>::type* = nullptr> ::std::shared_ptr<P>
checkedCast(const ::std::shared_ptr<T>& b, const ::Ice::Context& context = Ice::noExplicitContext)
{
    ::std::shared_ptr<P> r;
    if(b)
    {
        if(b->ice_isA(P::ice_staticId(), context))
        {
            r = IceInternal::createProxy<P>();
            r->_copyFrom(b);
        }
    }
    return r;
}

template<typename P,
         typename T,
         typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, P>::value>::type* = nullptr,
         typename ::std::enable_if<::std::is_base_of<::Ice::ObjectPrx, T>::value>::type* = nullptr> ::std::shared_ptr<P>
checkedCast(const ::std::shared_ptr<T>& b, const std::string& f, const ::Ice::Context& context = Ice::noExplicitContext)
{
    ::std::shared_ptr<P> r;
    if(b)
    {
        try
        {
            ::std::shared_ptr<::Ice::ObjectPrx> bb = b->ice_facet(f);
            if(bb->ice_isA(P::ice_staticId(), context))
            {
                r = IceInternal::createProxy<P>();
                r->_copyFrom(bb);
            }
        }
        catch(const Ice::FacetNotExistException&)
        {
        }
    }
    return r;
}

ICE_API ::std::ostream& operator<<(::std::ostream&, const Ice::ObjectPrx&);

}

#else // C++98 mapping

namespace IceProxy
{

namespace Ice
{

class Locator;
ICE_API ::IceProxy::Ice::Object* upCast(::IceProxy::Ice::Locator*);

class Router;
ICE_API ::IceProxy::Ice::Object* upCast(::IceProxy::Ice::Router*);

}

}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Router> RouterPrx;
typedef RouterPrx RouterPrxPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Locator> LocatorPrx;
typedef LocatorPrx LocatorPrxPtr;

class LocalException;
class OutputStream;

class Callback_Object_ice_isA_Base : public virtual ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_isA_Base> Callback_Object_ice_isAPtr;

class Callback_Object_ice_ping_Base : public virtual ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_ping_Base> Callback_Object_ice_pingPtr;

class Callback_Object_ice_ids_Base : public virtual ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_ids_Base> Callback_Object_ice_idsPtr;

class Callback_Object_ice_id_Base : public virtual ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_id_Base> Callback_Object_ice_idPtr;

class Callback_Object_ice_invoke_Base : public virtual ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_invoke_Base> Callback_Object_ice_invokePtr;

class Callback_Object_ice_flushBatchRequests_Base : public virtual ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_flushBatchRequests_Base> Callback_Object_ice_flushBatchRequestsPtr;

class Callback_Object_ice_getConnection_Base : public virtual ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_getConnection_Base> Callback_Object_ice_getConnectionPtr;

}

namespace IceProxy { namespace Ice
{

class ICE_API Object : public ::IceUtil::Shared
{
public:

    bool operator==(const Object&) const;
    bool operator<(const Object&) const;

    ::Ice::CommunicatorPtr ice_getCommunicator() const;

    ::std::string ice_toString() const;

    bool ice_isA(const ::std::string& typeId, const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return end_ice_isA(_iceI_begin_ice_isA(typeId, context, ::IceInternal::dummyCallback, 0, true));
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _iceI_begin_ice_isA(typeId, context, ::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::CallbackPtr& del,
                                        const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_isA(typeId, ::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::Context& context,
                                        const ::Ice::CallbackPtr& del,
                                        const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_isA(typeId, context, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::Callback_Object_ice_isAPtr& del,
                                        const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_isA(typeId, ::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::Context& context,
                                        const ::Ice::Callback_Object_ice_isAPtr& del,
                                        const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_isA(typeId, context, del, cookie);
    }

    bool end_ice_isA(const ::Ice::AsyncResultPtr&);

    void ice_ping(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        end_ice_ping(_iceI_begin_ice_ping(context, ::IceInternal::dummyCallback, 0, true));
    }

    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _iceI_begin_ice_ping(context, ::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::CallbackPtr& del, const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_ping(::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Context& context, const ::Ice::CallbackPtr& del,
                                         const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_ping(context, del, cookie);
    }


    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Callback_Object_ice_pingPtr& del,
                                         const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_ping(::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Context& context, const ::Ice::Callback_Object_ice_pingPtr& del,
                                         const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_ping(context, del, cookie);
    }

    void end_ice_ping(const ::Ice::AsyncResultPtr&);

    ::std::vector< ::std::string> ice_ids(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return end_ice_ids(_iceI_begin_ice_ids(context, ::IceInternal::dummyCallback, 0, true));
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _iceI_begin_ice_ids(context, ::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::CallbackPtr& del,
                                        const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_ids(::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Context& context,
                                        const ::Ice::CallbackPtr& del,
                                        const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_ids(context, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Callback_Object_ice_idsPtr& del,
                                        const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_ids(::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Context& context,
                                        const ::Ice::Callback_Object_ice_idsPtr& del,
                                        const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_ids(context, del, cookie);
    }

    ::std::vector< ::std::string> end_ice_ids(const ::Ice::AsyncResultPtr&);

    ::std::string ice_id(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return end_ice_id(_iceI_begin_ice_id(context, ::IceInternal::dummyCallback, 0, true));
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _iceI_begin_ice_id(context, ::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::CallbackPtr& del,
                                       const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_id(::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Context& context,
                                       const ::Ice::CallbackPtr& del,
                                       const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_id(context, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Callback_Object_ice_idPtr& del,
                                       const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_id(::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Context& context,
                                       const ::Ice::Callback_Object_ice_idPtr& del,
                                       const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_id(context, del, cookie);
    }

    ::std::string end_ice_id(const ::Ice::AsyncResultPtr&);

    static const ::std::string& ice_staticId()
    {
        return ::Ice::Object::ice_staticId();
    }


    // Returns true if ok, false if user exception.
    bool ice_invoke(const ::std::string&,
                    ::Ice::OperationMode,
                    const ::std::vector< ::Ice::Byte>&,
                    ::std::vector< ::Ice::Byte>&,
                    const ::Ice::Context& = ::Ice::noExplicitContext);

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, ::Ice::noExplicitContext, ::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::Context& context)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, context, ::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::CallbackPtr& del,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, ::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::Context& context,
                                           const ::Ice::CallbackPtr& del,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, context, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::Callback_Object_ice_invokePtr& del,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, ::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::Context& context,
                                           const ::Ice::Callback_Object_ice_invokePtr& del,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, context, del, cookie);
    }

    bool end_ice_invoke(::std::vector< ::Ice::Byte>&, const ::Ice::AsyncResultPtr&);

    bool ice_invoke(const ::std::string& op,
                    ::Ice::OperationMode mode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inP,
                    ::std::vector< ::Ice::Byte>& outP,
                    const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return end_ice_invoke(outP, _iceI_begin_ice_invoke(op, mode, inP, context, ::IceInternal::dummyCallback, 0, true));
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, ::Ice::noExplicitContext, ::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::Context& context,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, context, ::IceInternal::dummyCallback, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::CallbackPtr& del,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, ::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::Context& context,
                                           const ::Ice::CallbackPtr& del,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, context, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::Callback_Object_ice_invokePtr& del,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, ::Ice::noExplicitContext, del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& op,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::Context& context,
                                           const ::Ice::Callback_Object_ice_invokePtr& del,
                                           const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_invoke(op, mode, inParams, context, del, cookie);
    }

    bool _iceI_end_ice_invoke(::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&, const ::Ice::AsyncResultPtr&);

    ::Ice::Identity ice_getIdentity() const;
    ::Ice::ObjectPrx ice_identity(const ::Ice::Identity&) const;

    ::Ice::Context ice_getContext() const;
    ::Ice::ObjectPrx ice_context(const ::Ice::Context&) const;

    const ::std::string& ice_getFacet() const;
    ::Ice::ObjectPrx ice_facet(const ::std::string&) const;

    ::std::string ice_getAdapterId() const;
    ::Ice::ObjectPrx ice_adapterId(const ::std::string&) const;

    ::Ice::EndpointSeq ice_getEndpoints() const;
    ::Ice::ObjectPrx ice_endpoints(const ::Ice::EndpointSeq&) const;

    ::Ice::Int ice_getLocatorCacheTimeout() const;
    ::Ice::ObjectPrx ice_locatorCacheTimeout(::Ice::Int) const;

    bool ice_isConnectionCached() const;
    ::Ice::ObjectPrx ice_connectionCached(bool) const;

    ::Ice::EndpointSelectionType ice_getEndpointSelection() const;
    ::Ice::ObjectPrx ice_endpointSelection(::Ice::EndpointSelectionType) const;

    bool ice_isSecure() const;
    ::Ice::ObjectPrx ice_secure(bool) const;

    ::Ice::EncodingVersion ice_getEncodingVersion() const;
    ::Ice::ObjectPrx ice_encodingVersion(const ::Ice::EncodingVersion&) const;

    bool ice_isPreferSecure() const;
    ::Ice::ObjectPrx ice_preferSecure(bool) const;

    ::Ice::RouterPrx ice_getRouter() const;
    ::Ice::ObjectPrx ice_router(const ::Ice::RouterPrx&) const;

    ::Ice::LocatorPrx ice_getLocator() const;
    ::Ice::ObjectPrx ice_locator(const ::Ice::LocatorPrx&) const;

    bool ice_isCollocationOptimized() const;
    ::Ice::ObjectPrx ice_collocationOptimized(bool) const;

    ::Ice::Int ice_getInvocationTimeout() const;
    ::Ice::ObjectPrx ice_invocationTimeout(::Ice::Int) const;

    ::Ice::ObjectPrx ice_twoway() const;
    bool ice_isTwoway() const;
    ::Ice::ObjectPrx ice_oneway() const;
    bool ice_isOneway() const;
    ::Ice::ObjectPrx ice_batchOneway() const;
    bool ice_isBatchOneway() const;
    ::Ice::ObjectPrx ice_datagram() const;
    bool ice_isDatagram() const;
    ::Ice::ObjectPrx ice_batchDatagram() const;
    bool ice_isBatchDatagram() const;

    ::Ice::ObjectPrx ice_compress(bool) const;
    ::Ice::ObjectPrx ice_timeout(int) const;

    ::Ice::ObjectPrx ice_connectionId(const ::std::string&) const;
    ::std::string ice_getConnectionId() const;

    ::Ice::ConnectionPtr ice_getConnection()
    {
        return end_ice_getConnection(begin_ice_getConnection());
    }

    ::Ice::AsyncResultPtr begin_ice_getConnection()
    {
        return _iceI_begin_ice_getConnection(::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_getConnection(const ::Ice::CallbackPtr& del,
                                                  const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_getConnection(del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_getConnection(const ::Ice::Callback_Object_ice_getConnectionPtr& del,
                                                  const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_getConnection(del, cookie);
    }

    ::Ice::ConnectionPtr end_ice_getConnection(const ::Ice::AsyncResultPtr&);

    ::Ice::ConnectionPtr ice_getCachedConnection() const;

    void ice_flushBatchRequests()
    {
        return end_ice_flushBatchRequests(begin_ice_flushBatchRequests());
    }

    ::Ice::AsyncResultPtr begin_ice_flushBatchRequests()
    {
        return _iceI_begin_ice_flushBatchRequests(::IceInternal::dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_flushBatchRequests(const ::Ice::CallbackPtr& del,
                                                       const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_flushBatchRequests(del, cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_flushBatchRequests(const ::Ice::Callback_Object_ice_flushBatchRequestsPtr& del,
                                                       const ::Ice::LocalObjectPtr& cookie = 0)
    {
        return _iceI_begin_ice_flushBatchRequests(del, cookie);
    }

    void end_ice_flushBatchRequests(const ::Ice::AsyncResultPtr&);

    const ::IceInternal::ReferencePtr& _getReference() const { return _reference; }

    ::Ice::Int _hash() const;

    void _copyFrom(const ::Ice::ObjectPrx&);

    int _handleException(const ::Ice::Exception&, const ::IceInternal::RequestHandlerPtr&, ::Ice::OperationMode,
                          bool, int&);

    void _checkTwowayOnly(const ::std::string&, bool) const;

    void _end(const ::Ice::AsyncResultPtr&, const std::string&) const;

    ::IceInternal::RequestHandlerPtr _getRequestHandler();
    ::IceInternal::BatchRequestQueuePtr _getBatchRequestQueue();
    ::IceInternal::RequestHandlerPtr _setRequestHandler(const ::IceInternal::RequestHandlerPtr&);
    void _updateRequestHandler(const ::IceInternal::RequestHandlerPtr&, const ::IceInternal::RequestHandlerPtr&);

    void _write(::Ice::OutputStream&) const;

protected:

    virtual Object* _newInstance() const;

private:

    ::Ice::AsyncResultPtr _iceI_begin_ice_isA(const ::std::string&,
                                          const ::Ice::Context&,
                                          const ::IceInternal::CallbackBasePtr&,
                                          const ::Ice::LocalObjectPtr&,
                                          bool = false);

    ::Ice::AsyncResultPtr _iceI_begin_ice_ping(const ::Ice::Context&,
                                           const ::IceInternal::CallbackBasePtr&,
                                           const ::Ice::LocalObjectPtr&,
                                           bool = false);

    ::Ice::AsyncResultPtr _iceI_begin_ice_ids(const ::Ice::Context&,
                                          const ::IceInternal::CallbackBasePtr&,
                                          const ::Ice::LocalObjectPtr&,
                                          bool = false);

    ::Ice::AsyncResultPtr _iceI_begin_ice_id(const ::Ice::Context&,
                                         const ::IceInternal::CallbackBasePtr&,
                                         const ::Ice::LocalObjectPtr&,
                                         bool = false);

    ::Ice::AsyncResultPtr _iceI_begin_ice_invoke(const ::std::string&,
                                             ::Ice::OperationMode,
                                             const ::std::vector< ::Ice::Byte>&,
                                             const ::Ice::Context&,
                                             const ::IceInternal::CallbackBasePtr&,
                                             const ::Ice::LocalObjectPtr&,
                                             bool = false);

    ::Ice::AsyncResultPtr _iceI_begin_ice_invoke(const ::std::string&,
                                             ::Ice::OperationMode,
                                             const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                                             const ::Ice::Context&,
                                             const ::IceInternal::CallbackBasePtr&,
                                             const ::Ice::LocalObjectPtr&,
                                             bool = false);

    ::Ice::AsyncResultPtr _iceI_begin_ice_getConnection(const ::IceInternal::CallbackBasePtr&,
                                                    const ::Ice::LocalObjectPtr&);

    ::Ice::AsyncResultPtr _iceI_begin_ice_flushBatchRequests(const ::IceInternal::CallbackBasePtr&,
                                                         const ::Ice::LocalObjectPtr&);

    void setup(const ::IceInternal::ReferencePtr&);
    friend class ::IceInternal::ProxyFactory;

    ::IceInternal::ReferencePtr _reference;
    ::IceInternal::RequestHandlerPtr _requestHandler;
    ::IceInternal::BatchRequestQueuePtr _batchRequestQueue;
    IceUtil::Mutex _mutex;
};

} }

ICE_API ::std::ostream& operator<<(::std::ostream&, const ::IceProxy::Ice::Object&);

namespace Ice
{

template<typename Prx, typename Base>
class Proxy : public virtual Base
{
public:

    IceInternal::ProxyHandle<Prx> ice_context(const ::Ice::Context& context) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_context(context).get());
    }

    IceInternal::ProxyHandle<Prx> ice_adapterId(const ::std::string& id) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_adapterId(id).get());
    }

    IceInternal::ProxyHandle<Prx> ice_endpoints(const ::Ice::EndpointSeq& endpoints) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_endpoints(endpoints).get());
    }

    IceInternal::ProxyHandle<Prx> ice_locatorCacheTimeout(int timeout) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(timeout).get());
    }

    IceInternal::ProxyHandle<Prx> ice_connectionCached(bool cached) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_connectionCached(cached).get());
    }

    IceInternal::ProxyHandle<Prx> ice_endpointSelection(::Ice::EndpointSelectionType selection) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_endpointSelection(selection).get());
    }

    IceInternal::ProxyHandle<Prx> ice_secure(bool secure) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_secure(secure).get());
    }

    IceInternal::ProxyHandle<Prx> ice_preferSecure(bool preferSecure) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_preferSecure(preferSecure).get());
    }

    IceInternal::ProxyHandle<Prx> ice_router(const ::Ice::RouterPrx& router) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_router(router).get());
    }

    IceInternal::ProxyHandle<Prx> ice_locator(const ::Ice::LocatorPrx& locator) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_locator(locator).get());
    }

    IceInternal::ProxyHandle<Prx> ice_collocationOptimized(bool collocated) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_collocationOptimized(collocated).get());
    }

    IceInternal::ProxyHandle<Prx> ice_invocationTimeout(int timeout) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_invocationTimeout(timeout).get());
    }

    IceInternal::ProxyHandle<Prx> ice_twoway() const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_twoway().get());
    }

    IceInternal::ProxyHandle<Prx> ice_oneway() const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_oneway().get());
    }

    IceInternal::ProxyHandle<Prx> ice_batchOneway() const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    }

    IceInternal::ProxyHandle<Prx> ice_datagram() const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_datagram().get());
    }

    IceInternal::ProxyHandle<Prx> ice_batchDatagram() const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    }

    IceInternal::ProxyHandle<Prx> ice_compress(bool compress) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_compress(compress).get());
    }

    IceInternal::ProxyHandle<Prx> ice_timeout(int timeout) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_timeout(timeout).get());
    }

    IceInternal::ProxyHandle<Prx> ice_connectionId(const ::std::string& id) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_connectionId(id).get());
    }

    IceInternal::ProxyHandle<Prx> ice_encodingVersion(const ::Ice::EncodingVersion& version) const
    {
        return dynamic_cast<Prx*>(::IceProxy::Ice::Object::ice_encodingVersion(version).get());
    }

protected:

    virtual ::IceProxy::Ice::Object* _newInstance() const = 0;
};

ICE_API bool proxyIdentityLess(const ObjectPrx&, const ObjectPrx&);
ICE_API bool proxyIdentityEqual(const ObjectPrx&, const ObjectPrx&);

ICE_API bool proxyIdentityAndFacetLess(const ObjectPrx&, const ObjectPrx&);
ICE_API bool proxyIdentityAndFacetEqual(const ObjectPrx&, const ObjectPrx&);

struct ProxyIdentityLess : std::binary_function<bool, ObjectPrx&, ObjectPrx&>
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
        return proxyIdentityLess(lhs, rhs);
    }
};

struct ProxyIdentityEqual : std::binary_function<bool, ObjectPrx&, ObjectPrx&>
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
        return proxyIdentityEqual(lhs, rhs);
    }
};

struct ProxyIdentityAndFacetLess : std::binary_function<bool, ObjectPrx&, ObjectPrx&>
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
        return proxyIdentityAndFacetLess(lhs, rhs);
    }
};

struct ProxyIdentityAndFacetEqual : std::binary_function<bool, ObjectPrx&, ObjectPrx&>
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
        return proxyIdentityAndFacetEqual(lhs, rhs);
    }
};

}

namespace IceInternal
{

//
// Inline comparison functions for proxies
//
template<typename T, typename U>
inline bool operator==(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    ::IceProxy::Ice::Object* l = lhs._upCast();
    ::IceProxy::Ice::Object* r = rhs._upCast();
    if(l && r)
    {
        return *l == *r;
    }
    else
    {
        return !l && !r;
    }
}

template<typename T, typename U>
inline bool operator!=(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    return !operator==(lhs, rhs);
}

template<typename T, typename U>
inline bool operator<(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    ::IceProxy::Ice::Object* l = lhs._upCast();
    ::IceProxy::Ice::Object* r = rhs._upCast();
    if(l && r)
    {
        return *l < *r;
    }
    else
    {
        return !l && r;
    }
}

template<typename T, typename U>
inline bool operator<=(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    return lhs < rhs || lhs == rhs;
}

template<typename T, typename U>
inline bool operator>(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    return !(lhs < rhs || lhs == rhs);
}

template<typename T, typename U>
inline bool operator>=(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    return !(lhs < rhs);
}


//
// checkedCast and uncheckedCast functions without facet:
//
template<typename P> P
checkedCastImpl(const ::Ice::ObjectPrx& b, const ::Ice::Context& context)
{
    P d = 0;
    if(b.get())
    {
        typedef typename P::element_type T;

        if(b->ice_isA(T::ice_staticId(), context))
        {
            d = new T;
            d->_copyFrom(b);
        }
    }
    return d;
}

template<typename P> P
uncheckedCastImpl(const ::Ice::ObjectPrx& b)
{
    P d = 0;
    if(b)
    {
        typedef typename P::element_type T;

        d = dynamic_cast<T*>(b.get());
        if(!d)
        {
            d = new T;
            d->_copyFrom(b);
        }
    }
    return d;
}

//
// checkedCast and uncheckedCast with facet:
//

//
// Helper with type ID.
//
ICE_API ::Ice::ObjectPrx checkedCastImpl(const ::Ice::ObjectPrx&, const std::string&, const std::string&,
                                            const ::Ice::Context&);

//
// Specializations for P = ::Ice::ObjectPrx
// We have to use inline functions for broken compilers such as VC7.
//

template<> inline ::Ice::ObjectPrx
checkedCastImpl< ::Ice::ObjectPrx>(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context& context)
{
    return checkedCastImpl(b, f, "::Ice::Object", context);
}

template<> inline ::Ice::ObjectPrx
uncheckedCastImpl< ::Ice::ObjectPrx>(const ::Ice::ObjectPrx& b, const std::string& f)
{
    ::Ice::ObjectPrx d = 0;
    if(b)
    {
        d = b->ice_facet(f);
    }
    return d;
}

template<typename P> P
checkedCastImpl(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context& context)
{
    P d = 0;

    typedef typename P::element_type T;
    ::Ice::ObjectPrx bb = checkedCastImpl(b, f, T::ice_staticId(), context);

    if(bb)
    {
        d = new T;
        d->_copyFrom(bb);
    }
    return d;
}

template<typename P> P
uncheckedCastImpl(const ::Ice::ObjectPrx& b, const std::string& f)
{
    P d = 0;
    if(b)
    {
        typedef typename P::element_type T;

        ::Ice::ObjectPrx bb = b->ice_facet(f);
        d = new T;
        d->_copyFrom(bb);
    }
    return d;
}
}

//
// checkedCast and uncheckedCast functions provided in the Ice namespace
//
namespace Ice
{

template<typename P, typename Y> inline P
checkedCast(const ::IceInternal::ProxyHandle<Y>& b, const ::Ice::Context& context = ::Ice::noExplicitContext)
{
    Y* tag = 0;
    return ::IceInternal::checkedCastHelper<typename P::element_type>(b, tag, context);
}

template<typename P, typename Y> inline P
uncheckedCast(const ::IceInternal::ProxyHandle<Y>& b)
{
    Y* tag = 0;
    return ::IceInternal::uncheckedCastHelper<typename P::element_type>(b, tag);
}

template<typename P> inline P
checkedCast(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context& context = ::Ice::noExplicitContext)
{
    return ::IceInternal::checkedCastImpl<P>(b, f, context);
}

template<typename P> inline P
uncheckedCast(const ::Ice::ObjectPrx& b, const std::string& f)
{
    return ::IceInternal::uncheckedCastImpl<P>(b, f);
}

}

namespace IceInternal
{

//
// Base template for operation callbacks.
//
template<class T>
class CallbackNC : public virtual CallbackBase
{
public:

    typedef T callback_type;

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    CallbackNC(const TPtr& instance, Exception excb, Sent sentcb) : _callback(instance), _exception(excb), _sent(sentcb)
    {
    }

    virtual CallbackBasePtr verify(const ::Ice::LocalObjectPtr& cookie)
    {
        if(cookie != 0) // Makes sure begin_ was called without a cookie
        {
            throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "cookie specified for callback without cookie");
        }
        return this;
    }

    virtual void sent(const ::Ice::AsyncResultPtr& result) const
    {
        if(_sent)
        {
            (_callback.get()->*_sent)(result->sentSynchronously());
        }
    }

    virtual bool hasSentCallback() const
    {
        return _sent != 0;
    }

protected:

    void exception(const ::Ice::AsyncResultPtr&, const ::Ice::Exception& ex) const
    {
        if(_exception)
        {
            (_callback.get()->*_exception)(ex);
        }
    }

    TPtr _callback;

private:

    Exception _exception;
    Sent _sent;
};

template<class T, typename CT>
class Callback : public virtual CallbackBase
{
public:

    typedef T callback_type;
    typedef CT cookie_type;

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);

    Callback(const TPtr& instance, Exception excb, Sent sentcb) : _callback(instance), _exception(excb), _sent(sentcb)
    {
    }

    virtual CallbackBasePtr verify(const ::Ice::LocalObjectPtr& cookie)
    {
        if(cookie && !CT::dynamicCast(cookie))
        {
            throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "unexpected cookie type");
        }
        return this;
    }

    virtual void sent(const ::Ice::AsyncResultPtr& result) const
    {
        if(_sent)
        {
            (_callback.get()->*_sent)(result->sentSynchronously(), CT::dynamicCast(result->getCookie()));
        }
    }

    virtual bool hasSentCallback() const
    {
        return _sent != 0;
    }

protected:

    void exception(const ::Ice::AsyncResultPtr& result, const ::Ice::Exception& ex) const
    {
        if(_exception)
        {
            (_callback.get()->*_exception)(ex, CT::dynamicCast(result->getCookie()));
        }
    }

    TPtr _callback;

private:

    Exception _exception;
    Sent _sent;
};

//
// Base class for twoway operation callbacks.
//
template<class T>
class TwowayCallbackNC : public CallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    TwowayCallbackNC(const TPtr& instance, bool cb, Exception excb, Sent sentcb) : CallbackNC<T>(instance, excb, sentcb)
    {
        CallbackBase::checkCallback(instance, cb || excb != 0);
    }
};

template<class T, typename CT>
class TwowayCallback : public Callback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);

    TwowayCallback(const TPtr& instance, bool cb, Exception excb, Sent sentcb) : Callback<T, CT>(instance, excb, sentcb)
    {
        CallbackBase::checkCallback(instance, cb || excb != 0);
    }
};

//
// Base template class for oneway operations callbacks.
//
template<class T>
class OnewayCallbackNC : public CallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);
    typedef void (T::*Response)();

    OnewayCallbackNC(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        CallbackNC<T>(instance, excb, sentcb), _response(cb)
    {
        CallbackBase::checkCallback(instance, cb != 0 || excb != 0);
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        try
        {
            result->getProxy()->_end(result, result->getOperation());
        }
        catch(const ::Ice::Exception& ex)
        {
            CallbackNC<T>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (CallbackNC<T>::_callback.get()->*_response)();
        }
    }

private:

    Response _response;
};

template<class T, typename CT>
class OnewayCallback : public Callback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);
    typedef void (T::*Response)(const CT&);

    OnewayCallback(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        Callback<T, CT>(instance, excb, sentcb), _response(cb)
    {
        CallbackBase::checkCallback(instance, cb != 0 || excb != 0);
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        try
        {
            result->getProxy()->_end(result, result->getOperation());
        }
        catch(const ::Ice::Exception& ex)
        {
            Callback<T, CT>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (Callback<T, CT>::_callback.get()->*_response)(CT::dynamicCast(result->getCookie()));
        }
    }

private:

    Response _response;
};

}

namespace Ice
{

template<class T>
class CallbackNC_Object_ice_isA : public Callback_Object_ice_isA_Base, public ::IceInternal::TwowayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);
    typedef void (T::*Response)(bool);

    CallbackNC_Object_ice_isA(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallbackNC<T>(instance, cb != 0, excb, sentcb), _response(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        bool ret;
        try
        {
            ret = result->getProxy()->end_ice_isA(result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(ret);
        }
    }

private:

    Response _response;
};

template<class T, typename CT>
class Callback_Object_ice_isA : public Callback_Object_ice_isA_Base, public ::IceInternal::TwowayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);
    typedef void (T::*Response)(bool, const CT&);

    Callback_Object_ice_isA(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallback<T, CT>(instance, cb != 0, excb, sentcb), _response(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        bool ret;
        try
        {
            ret = result->getProxy()->end_ice_isA(result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(ret,
                                                                          CT::dynamicCast(result->getCookie()));
        }
    }

private:

    Response _response;
};

template<class T>
class CallbackNC_Object_ice_ping : public Callback_Object_ice_ping_Base, public ::IceInternal::OnewayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);
    typedef void (T::*Response)();

    CallbackNC_Object_ice_ping(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::OnewayCallbackNC<T>(instance, cb, excb, sentcb)
    {
    }
};

template<class T, typename CT>
class Callback_Object_ice_ping : public Callback_Object_ice_ping_Base, public ::IceInternal::OnewayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);
    typedef void (T::*Response)(const CT&);

    Callback_Object_ice_ping(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::OnewayCallback<T, CT>(instance, cb, excb, sentcb)
    {
    }
};

template<class T>
class CallbackNC_Object_ice_ids : public Callback_Object_ice_ids_Base, public ::IceInternal::TwowayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);
    typedef void (T::*Response)(const ::std::vector< ::std::string>&);

    CallbackNC_Object_ice_ids(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallbackNC<T>(instance, cb != 0, excb, sentcb), _response(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::std::vector< ::std::string> ret;
        try
        {
            ret = result->getProxy()->end_ice_ids(result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(ret);
        }
    }

private:

    Response _response;
};

template<class T, typename CT>
class Callback_Object_ice_ids : public Callback_Object_ice_ids_Base, public ::IceInternal::TwowayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);
    typedef void (T::*Response)(const ::std::vector< ::std::string>&, const CT&);

    Callback_Object_ice_ids(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallback<T, CT>(instance, cb != 0, excb, sentcb), _response(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::std::vector< ::std::string> ret;
        try
        {
            ret = result->getProxy()->end_ice_ids(result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(ret,
                                                                          CT::dynamicCast(result->getCookie()));
        }
    }

private:

    Response _response;
};

template<class T>
class CallbackNC_Object_ice_id : public Callback_Object_ice_id_Base, public ::IceInternal::TwowayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);
    typedef void (T::*Response)(const ::std::string&);

    CallbackNC_Object_ice_id(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallbackNC<T>(instance, cb != 0, excb, sentcb), _response(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::std::string ret;
        try
        {
            ret = result->getProxy()->end_ice_id(result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(ret);
        }
    }

private:

    Response _response;
};

template<class T, typename CT>
class Callback_Object_ice_id : public Callback_Object_ice_id_Base, public ::IceInternal::TwowayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);
    typedef void (T::*Response)(const ::std::string&, const CT&);

    Callback_Object_ice_id(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallback<T, CT>(instance, cb != 0, excb, sentcb), _response(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::std::string ret;
        try
        {
            ret = result->getProxy()->end_ice_id(result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(ret,
                                                                          CT::dynamicCast(result->getCookie()));
        }
    }

private:

    Response _response;
};

template<class T>
class CallbackNC_Object_ice_invoke : public Callback_Object_ice_invoke_Base, public ::IceInternal::TwowayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);
    typedef void (T::*Response)(bool, const std::vector< ::Ice::Byte>&);
    typedef void (T::*ResponseArray)(bool, const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&);

    CallbackNC_Object_ice_invoke(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallbackNC<T>(instance, cb != 0, excb, sentcb), _response(cb), _responseArray(0)
    {
    }

    CallbackNC_Object_ice_invoke(const TPtr& instance, ResponseArray cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallbackNC<T>(instance, cb != 0, excb, sentcb), _response(0), _responseArray(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        if(_response)
        {
            bool ok;
            std::vector< ::Ice::Byte> outParams;
            try
            {
                ok = result->getProxy()->end_ice_invoke(outParams, result);
            }
            catch(const ::Ice::Exception& ex)
            {
                ::IceInternal::CallbackNC<T>::exception(result, ex);
                return;
            }
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(ok, outParams);
        }
        else
        {
            bool ok;
            std::pair<const ::Ice::Byte*, const::Ice::Byte*> outParams;
            try
            {
                ok = result->getProxy()->_iceI_end_ice_invoke(outParams, result);
            }
            catch(const ::Ice::Exception& ex)
            {
                ::IceInternal::CallbackNC<T>::exception(result, ex);
                return;
            }
            if(_responseArray)
            {
                (::IceInternal::CallbackNC<T>::_callback.get()->*_responseArray)(ok, outParams);
            }
        }
    }

private:

    Response _response;
    ResponseArray _responseArray;
};

template<class T, typename CT>
class Callback_Object_ice_invoke : public Callback_Object_ice_invoke_Base, public ::IceInternal::TwowayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);
    typedef void (T::*Response)(bool, const std::vector< ::Ice::Byte>&, const CT&);
    typedef void (T::*ResponseArray)(bool, const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&, const CT&);

    Callback_Object_ice_invoke(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallback<T, CT>(instance, cb != 0, excb, sentcb), _response(cb), _responseArray(0)
    {
    }

    Callback_Object_ice_invoke(const TPtr& instance, ResponseArray cb, Exception excb, Sent sentcb) :
        ::IceInternal::TwowayCallback<T, CT>(instance, cb != 0, excb, sentcb), _response(0), _responseArray(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        if(_response)
        {
            bool ok;
            std::vector< ::Ice::Byte> outParams;
            try
            {
                ok = result->getProxy()->end_ice_invoke(outParams, result);
            }
            catch(const ::Ice::Exception& ex)
            {
                ::IceInternal::Callback<T, CT>::exception(result, ex);
                return;
            }
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(ok,
                                                                          outParams,
                                                                          CT::dynamicCast(result->getCookie()));
        }
        else
        {
            bool ok;
            std::pair<const ::Ice::Byte*, const::Ice::Byte*> outParams;
            try
            {
                ok = result->getProxy()->_iceI_end_ice_invoke(outParams, result);
            }
            catch(const ::Ice::Exception& ex)
            {
                ::IceInternal::Callback<T, CT>::exception(result, ex);
                return;
            }
            if(_responseArray)
            {
                (::IceInternal::Callback<T, CT>::_callback.get()->*_responseArray)(ok,
                                                                                   outParams,
                                                                                   CT::dynamicCast(
                                                                                       result->getCookie()));
            }
        }
    }

private:

    Response _response;
    ResponseArray _responseArray;
};

template<class T>
class CallbackNC_Object_ice_getConnection : public Callback_Object_ice_getConnection_Base,
                                            public ::IceInternal::CallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Response)(const ::Ice::ConnectionPtr&);
    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    CallbackNC_Object_ice_getConnection(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::CallbackNC<T>(instance, excb, sentcb), _response(cb)
    {
    }


    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::Ice::ConnectionPtr ret;
        try
        {
            ret = result->getProxy()->end_ice_getConnection(result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(ret);
        }
    }

private:

    Response _response;
};

template<class T, typename CT>
class Callback_Object_ice_getConnection : public Callback_Object_ice_getConnection_Base,
                                          public ::IceInternal::Callback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Response)(const ::Ice::ConnectionPtr&, const CT&);
    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);

    Callback_Object_ice_getConnection(const TPtr& instance, Response cb, Exception excb, Sent sentcb) :
        ::IceInternal::Callback<T, CT>(instance, excb, sentcb), _response(cb)
    {
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::Ice::ConnectionPtr ret;
        try
        {
            ret = result->getProxy()->end_ice_getConnection(result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(ret,
                                                                          CT::dynamicCast(result->getCookie()));
        }
    }

private:

    Response _response;
};

template<class T>
class CallbackNC_Object_ice_flushBatchRequests : public Callback_Object_ice_flushBatchRequests_Base,
                                                 public ::IceInternal::OnewayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    CallbackNC_Object_ice_flushBatchRequests(const TPtr& instance, Exception excb, Sent sentcb) :
        ::IceInternal::OnewayCallbackNC<T>(instance, 0, excb, sentcb)
    {
    }
};

template<class T, typename CT>
class Callback_Object_ice_flushBatchRequests : public Callback_Object_ice_flushBatchRequests_Base,
                                               public ::IceInternal::OnewayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CT&);
    typedef void (T::*Sent)(bool, const CT&);

    Callback_Object_ice_flushBatchRequests(const TPtr& instance, Exception excb, Sent sentcb) :
        ::IceInternal::OnewayCallback<T, CT>(instance, 0, excb, sentcb)
    {
    }
};

template<class T> Callback_Object_ice_isAPtr
newCallback_Object_ice_isA(const IceUtil::Handle<T>& instance,
                           void (T::*cb)(bool),
                           void (T::*excb)(const ::Ice::Exception&),
                           void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_isA<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_isAPtr
newCallback_Object_ice_isA(const IceUtil::Handle<T>& instance,
                           void (T::*cb)(bool, const CT&),
                           void (T::*excb)(const ::Ice::Exception&, const CT&),
                           void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_isA<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_isAPtr
newCallback_Object_ice_isA(const IceUtil::Handle<T>& instance,
                           void (T::*excb)(const ::Ice::Exception&),
                           void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_isA<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_isAPtr
newCallback_Object_ice_isA(const IceUtil::Handle<T>& instance,
                           void (T::*excb)(const ::Ice::Exception&, const CT&),
                           void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_isA<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_isAPtr
newCallback_Object_ice_isA(T* instance,
                           void (T::*cb)(bool),
                           void (T::*excb)(const ::Ice::Exception&),
                           void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_isA<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_isAPtr
newCallback_Object_ice_isA(T* instance,
                           void (T::*cb)(bool, const CT&),
                           void (T::*excb)(const ::Ice::Exception&, const CT&),
                           void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_isA<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_isAPtr
newCallback_Object_ice_isA(T* instance,
                           void (T::*excb)(const ::Ice::Exception&),
                           void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_isA<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_isAPtr
newCallback_Object_ice_isA(T* instance,
                           void (T::*excb)(const ::Ice::Exception&, const CT&),
                           void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_isA<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_pingPtr
newCallback_Object_ice_ping(const IceUtil::Handle<T>& instance,
                            void (T::*cb)(),
                            void (T::*excb)(const ::Ice::Exception&),
                            void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_ping<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_pingPtr
newCallback_Object_ice_ping(const IceUtil::Handle<T>& instance,
                            void (T::*cb)(const CT&),
                            void (T::*excb)(const ::Ice::Exception&, const CT&),
                            void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_ping<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_pingPtr
newCallback_Object_ice_ping(const IceUtil::Handle<T>& instance,
                            void (T::*excb)(const ::Ice::Exception&),
                            void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_ping<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_pingPtr
newCallback_Object_ice_ping(const IceUtil::Handle<T>& instance,
                            void (T::*excb)(const ::Ice::Exception&, const CT&),
                            void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_ping<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_pingPtr
newCallback_Object_ice_ping(T* instance,
                            void (T::*cb)(),
                            void (T::*excb)(const ::Ice::Exception&),
                            void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_ping<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_pingPtr
newCallback_Object_ice_ping(T* instance,
                            void (T::*cb)(const CT&),
                            void (T::*excb)(const ::Ice::Exception&, const CT&),
                            void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_ping<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_pingPtr
newCallback_Object_ice_ping(T* instance,
                            void (T::*excb)(const ::Ice::Exception&),
                            void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_ping<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_pingPtr
newCallback_Object_ice_ping(T* instance,
                            void (T::*excb)(const ::Ice::Exception&, const CT&),
                            void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_ping<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_idsPtr
newCallback_Object_ice_ids(const IceUtil::Handle<T>& instance,
                           void (T::*cb)(const ::std::vector< ::std::string>&),
                           void (T::*excb)(const ::Ice::Exception&),
                           void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_ids<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_idsPtr
newCallback_Object_ice_ids(const IceUtil::Handle<T>& instance,
                           void (T::*cb)(const ::std::vector< ::std::string>&, const CT&),
                           void (T::*excb)(const ::Ice::Exception&, const CT&),
                           void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_ids<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_idsPtr
newCallback_Object_ice_ids(const IceUtil::Handle<T>& instance,
                           void (T::*excb)(const ::Ice::Exception&),
                           void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_ids<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_idsPtr
newCallback_Object_ice_ids(const IceUtil::Handle<T>& instance,
                           void (T::*excb)(const ::Ice::Exception&, const CT&),
                           void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_ids<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_idsPtr
newCallback_Object_ice_ids(T* instance,
                           void (T::*cb)(const ::std::vector< ::std::string>&),
                           void (T::*excb)(const ::Ice::Exception&),
                           void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_ids<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_idsPtr
newCallback_Object_ice_ids(T* instance,
                           void (T::*cb)(const ::std::vector< ::std::string>&, const CT&),
                           void (T::*excb)(const ::Ice::Exception&, const CT&),
                           void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_ids<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_idsPtr
newCallback_Object_ice_ids(T* instance,
                           void (T::*excb)(const ::Ice::Exception&),
                           void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_ids<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_idsPtr
newCallback_Object_ice_ids(T* instance,
                           void (T::*excb)(const ::Ice::Exception&, const CT&),
                           void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_ids<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_idPtr
newCallback_Object_ice_id(const IceUtil::Handle<T>& instance,
                          void (T::*cb)(const ::std::string&),
                          void (T::*excb)(const ::Ice::Exception&),
                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_id<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_idPtr
newCallback_Object_ice_id(const IceUtil::Handle<T>& instance,
                          void (T::*cb)(const ::std::string&, const CT&),
                          void (T::*excb)(const ::Ice::Exception&, const CT&),
                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_id<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_idPtr
newCallback_Object_ice_id(const IceUtil::Handle<T>& instance,
                          void (T::*excb)(const ::Ice::Exception&),
                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_id<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_idPtr
newCallback_Object_ice_id(const IceUtil::Handle<T>& instance,
                          void (T::*excb)(const ::Ice::Exception&, const CT&),
                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_id<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_idPtr
newCallback_Object_ice_id(T* instance,
                          void (T::*cb)(const ::std::string&),
                          void (T::*excb)(const ::Ice::Exception&),
                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_id<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_idPtr
newCallback_Object_ice_id(T* instance,
                          void (T::*cb)(const ::std::string&, const CT&),
                          void (T::*excb)(const ::Ice::Exception&, const CT&),
                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_id<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_idPtr
newCallback_Object_ice_id(T* instance,
                          void (T::*excb)(const ::Ice::Exception&),
                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_id<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_idPtr
newCallback_Object_ice_id(T* instance,
                          void (T::*excb)(const ::Ice::Exception&, const CT&),
                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_id<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(const IceUtil::Handle<T>& instance,
                              void (T::*cb)(bool, const std::vector<Ice::Byte>&),
                              void (T::*excb)(const ::Ice::Exception&),
                              void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_invoke<T>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(const IceUtil::Handle<T>& instance,
                              void (T::*cb)(bool, const std::pair<const Byte*, const Byte*>&),
                              void (T::*excb)(const ::Ice::Exception&),
                              void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_invoke<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(const IceUtil::Handle<T>& instance,
                              void (T::*cb)(bool, const std::vector<Ice::Byte>&, const CT&),
                              void (T::*excb)(const ::Ice::Exception&, const CT&),
                              void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_invoke<T, CT>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(const IceUtil::Handle<T>& instance,
                              void (T::*cb)(bool, const std::pair<const Byte*, const Byte*>&,
                                            const CT&),
                              void (T::*excb)(const ::Ice::Exception&, const CT&),
                              void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_invoke<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(const IceUtil::Handle<T>& instance,
                              void (T::*excb)(const ::Ice::Exception&),
                              void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_invoke<T>(instance, 0, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(const IceUtil::Handle<T>& instance,
                              void (T::*excb)(const ::Ice::Exception&, const CT&),
                              void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_invoke<T, CT>(instance, 0, excb, sentcb);
}

template<class T> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(T* instance,
                              void (T::*cb)(bool, const std::vector<Ice::Byte>&),
                              void (T::*excb)(const ::Ice::Exception&),
                              void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_invoke<T>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(T* instance,
                              void (T::*cb)(bool, const std::pair<const Byte*, const Byte*>&),
                              void (T::*excb)(const ::Ice::Exception&),
                              void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_invoke<T>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(T* instance,
                              void (T::*cb)(bool, const std::vector<Ice::Byte>&, const CT&),
                              void (T::*excb)(const ::Ice::Exception&, const CT&),
                              void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_invoke<T, CT>(instance, cb, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(T* instance,
                              void (T::*cb)(bool, const std::pair<const Byte*, const Byte*>&, const CT&),
                              void (T::*excb)(const ::Ice::Exception&, const CT&),
                              void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_invoke<T, CT>(instance, cb, excb, sentcb);
}

template<class T> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(T* instance,
                              void (T::*excb)(const ::Ice::Exception&),
                              void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_invoke<T>(
        instance, static_cast<void (T::*)(bool, const std::vector<Ice::Byte>&)>(0), excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_invokePtr
newCallback_Object_ice_invoke(T* instance,
                              void (T::*excb)(const ::Ice::Exception&, const CT&),
                              void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_invoke<T, CT>(
        instance, static_cast<void (T::*)(bool, const std::vector<Ice::Byte>&, const CT&)>(0), excb, sentcb);
}

template<class T> Callback_Object_ice_getConnectionPtr
newCallback_Object_ice_getConnection(const IceUtil::Handle<T>& instance,
                                     void (T::*cb)(const ::Ice::ConnectionPtr&),
                                     void (T::*excb)(const ::Ice::Exception&))
{
    return new CallbackNC_Object_ice_getConnection<T>(instance, cb, excb, 0);
}

template<class T, typename CT> Callback_Object_ice_getConnectionPtr
newCallback_Object_ice_getConnection(const IceUtil::Handle<T>& instance,
                                     void (T::*cb)(const ::Ice::ConnectionPtr&, const CT&),
                                     void (T::*excb)(const ::Ice::Exception&, const CT&))
{
    return new Callback_Object_ice_getConnection<T, CT>(instance, cb, excb, 0);
}

template<class T> Callback_Object_ice_getConnectionPtr
newCallback_Object_ice_getConnection(T* instance,
                                     void (T::*cb)(const ::Ice::ConnectionPtr&),
                                     void (T::*excb)(const ::Ice::Exception&))
{
    return new CallbackNC_Object_ice_getConnection<T>(instance, cb, excb, 0);
}

template<class T, typename CT> Callback_Object_ice_getConnectionPtr
newCallback_Object_ice_getConnection(T* instance,
                                     void (T::*cb)(const ::Ice::ConnectionPtr&, const CT&),
                                     void (T::*excb)(const ::Ice::Exception&, const CT&))
{
    return new Callback_Object_ice_getConnection<T, CT>(instance, cb, excb, 0);
}

template<class T> Callback_Object_ice_flushBatchRequestsPtr
newCallback_Object_ice_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                          void (T::*excb)(const ::Ice::Exception&),
                                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_flushBatchRequests<T>(instance, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_flushBatchRequestsPtr
newCallback_Object_ice_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                          void (T::*excb)(const ::Ice::Exception&, const CT&),
                                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

template<class T> Callback_Object_ice_flushBatchRequestsPtr
newCallback_Object_ice_flushBatchRequests(T* instance,
                                          void (T::*excb)(const ::Ice::Exception&),
                                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Object_ice_flushBatchRequests<T>(instance, excb, sentcb);
}

template<class T, typename CT> Callback_Object_ice_flushBatchRequestsPtr
newCallback_Object_ice_flushBatchRequests(T* instance,
                                          void (T::*excb)(const ::Ice::Exception&, const CT&),
                                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Object_ice_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

}
#endif

#endif
