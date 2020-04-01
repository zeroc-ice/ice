//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
//#include <Ice/LocatorF.h> // Can't include LocatorF.h here, otherwise we have cyclic includes
#include <Ice/Context.h>
#include <Ice/Current.h>
#include <Ice/CommunicatorF.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/LocalException.h>
#include <iosfwd>

namespace Ice
{

/** Marker value used to indicate that no explicit context was passed to a proxy invocation. */
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
#if ICE_CPLUSPLUS >= 201402L
            // Move capture with C++14
            _response = [this, response = std::move(response)](bool ok)
#else
            _response = [this, response](bool ok)
#endif
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
#if ICE_CPLUSPLUS >= 201402L
        _response = [&, response = std::move(response)](bool)
#else
        _response = [&, response](bool)
#endif
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
        this->_promise.set_value();
        return false;
    }
};

}

namespace Ice
{

class RouterPrx;
/// \cond INTERNAL
using RouterPrxPtr = ::std::shared_ptr<::Ice::RouterPrx>;
/// \endcond

class LocatorPrx;
/// \cond INTERNAL
using LocatorPrxPtr = ::std::shared_ptr<::Ice::LocatorPrx>;
/// \endcond

class LocalException;
class OutputStream;

/**
 * Base class of all object proxies.
 * \headerfile Ice/Ice.h
 */
class ICE_API ObjectPrx : public ::std::enable_shared_from_this<ObjectPrx>
{
public:

    virtual ~ObjectPrx() = default;

    friend ICE_API bool operator<(const ObjectPrx&, const ObjectPrx&);
    friend ICE_API bool operator==(const ObjectPrx&, const ObjectPrx&);

    /**
     * Obtains the communicator that created this proxy.
     * @return The communicator that created this proxy.
     */
    ::std::shared_ptr<::Ice::Communicator> ice_getCommunicator() const;

    /**
     * Obtains a stringified version of this proxy.
     * @return A stringified proxy.
     */
    ::std::string ice_toString() const;

    /**
     * Tests whether this object supports a specific Slice interface.
     * @param typeId The type ID of the Slice interface to test against.
     * @param context The context map for the invocation.
     * @return true if the target object has the interface
     * specified by id or derives from the interface specified by id.
     */
    bool
    ice_isA(const ::std::string& typeId, const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makePromiseOutgoing<bool>(true, this, &ObjectPrx::_iceI_isA, typeId, context).get();
    }

    /**
     * Tests whether this object supports a specific Slice interface.
     * @param typeId The type ID of the Slice interface to test against.
     * @param response The response callback.
     * @param ex The exception callback.
     * @param sent The sent callback.
     * @param context The context map for the invocation.
     * @return A function that can be called to cancel the invocation locally.
     */
    ::std::function<void()>
    ice_isAAsync(const ::std::string& typeId,
                 ::std::function<void(bool)> response,
                 ::std::function<void(::std::exception_ptr)> ex = nullptr,
                 ::std::function<void(bool)> sent = nullptr,
                 const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makeLambdaOutgoing<bool>(std::move(response), std::move(ex), std::move(sent), this,
                                         &ObjectPrx::_iceI_isA, typeId, context);
    }

    /**
     * Tests whether this object supports a specific Slice interface.
     * @param typeId The type ID of the Slice interface to test against.
     * @param context The context map for the invocation.
     * @return The future object for the invocation.
     */
    template<template<typename> class P = std::promise> auto
    ice_isAAsync(const ::std::string& typeId, const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<bool>>().get_future())
    {
        return _makePromiseOutgoing<bool, P>(false, this, &ObjectPrx::_iceI_isA, typeId, context);
    }

    /// \cond INTERNAL
    void
    _iceI_isA(const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<bool>>&, const ::std::string&, const ::Ice::Context&);
    /// \endcond

    /**
     * Tests whether the target object of this proxy can be reached.
     * @param context The context map for the invocation.
     */
    void
    ice_ping(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        _makePromiseOutgoing<void>(true, this, &ObjectPrx::_iceI_ping, context).get();
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     * @param response The response callback.
     * @param ex The exception callback.
     * @param sent The sent callback.
     * @param context The context map for the invocation.
     * @return A function that can be called to cancel the invocation locally.
     */
    ::std::function<void()>
    ice_pingAsync(::std::function<void()> response,
                  ::std::function<void(::std::exception_ptr)> ex = nullptr,
                  ::std::function<void(bool)> sent = nullptr,
                  const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makeLambdaOutgoing<void>(std::move(response), std::move(ex), std::move(sent), this,
                                         &ObjectPrx::_iceI_ping, context);
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     * @param context The context map for the invocation.
     * @return The future object for the invocation.
     */
    template<template<typename> class P = std::promise>
    auto ice_pingAsync(const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<void>>().get_future())
    {
        return _makePromiseOutgoing<void, P>(false, this, &ObjectPrx::_iceI_ping, context);
    }

    /// \cond INTERNAL
    void
    _iceI_ping(const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<void>>&, const ::Ice::Context&);
    /// \endcond

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     * @param context The context map for the invocation.
     * @return The Slice type IDs of the interfaces supported by the target object, in base-to-derived
     * order. The first element of the returned array is always "::Ice::Object".
     */
    ::std::vector<::std::string>
    ice_ids(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makePromiseOutgoing<::std::vector<::std::string>>(true, this, &ObjectPrx::_iceI_ids, context).get();
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     * @param response The response callback.
     * @param ex The exception callback.
     * @param sent The sent callback.
     * @param context The context map for the invocation.
     * @return A function that can be called to cancel the invocation locally.
     */
    ::std::function<void()>
    ice_idsAsync(::std::function<void(::std::vector<::std::string>)> response,
                 ::std::function<void(::std::exception_ptr)> ex = nullptr,
                 ::std::function<void(bool)> sent = nullptr,
                 const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makeLambdaOutgoing<::std::vector<::std::string>>(std::move(response), std::move(ex), std::move(sent),
                                                                 this, &ObjectPrx::_iceI_ids, context);
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     * @param context The context map for the invocation.
     * @return The future object for the invocation.
     */
    template<template<typename> class P = std::promise> auto
    ice_idsAsync(const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<::std::vector<::std::string>>>().get_future())
    {
        return _makePromiseOutgoing<::std::vector<::std::string>, P>(false, this, &ObjectPrx::_iceI_ids, context);
    }

    /// \cond INTERNAL
    void
    _iceI_ids(const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<::std::vector<::std::string>>>&, const ::Ice::Context&);
    /// \endcond

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     * @param context The context map for the invocation.
     * @return The Slice type ID of the most-derived interface.
     */
    ::std::string
    ice_id(const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makePromiseOutgoing<::std::string>(true, this, &ObjectPrx::_iceI_id, context).get();
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     * @param response The response callback.
     * @param ex The exception callback.
     * @param sent The sent callback.
     * @param context The context map for the invocation.
     * @return A function that can be called to cancel the invocation locally.
     */
    ::std::function<void()>
    ice_idAsync(::std::function<void(::std::string)> response,
                ::std::function<void(::std::exception_ptr)> ex = nullptr,
                ::std::function<void(bool)> sent = nullptr,
                const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return _makeLambdaOutgoing<::std::string>(std::move(response), std::move(ex), std::move(sent), this,
                                                  &ObjectPrx::_iceI_id, context);
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     * @param context The context map for the invocation.
     * @return The future object for the invocation.
     */
    template<template<typename> class P = std::promise>
    auto ice_idAsync(const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<::std::string>>().get_future())
    {
        return _makePromiseOutgoing<::std::string, P>(false, this, &ObjectPrx::_iceI_id, context);
    }

    /// \cond INTERNAL
    void
    _iceI_id(const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<::std::string>>&, const ::Ice::Context&);
    /// \endcond

    /**
     * Returns the Slice type ID associated with this type.
     * @return The Slice type ID.
     */
    static const ::std::string& ice_staticId()
    {
        return ::Ice::Object::ice_staticId();
    }

    /**
     * Invokes an operation dynamically.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams An encapsulation containing the encoded in-parameters for the operation.
     * @param outParams An encapsulation containing the encoded results.
     * @param context The context map for the invocation.
     * @return True if the operation completed successfully, in which case outParams contains
     * the encoded out parameters. False if the operation raised a user exception, in which
     * case outParams contains the encoded user exception. If the operation raises a run-time
     * exception, it throws it directly.
     */
    bool
    ice_invoke(const ::std::string& operation,
               ::Ice::OperationMode mode,
               const ::std::vector<Byte>& inParams,
               ::std::vector<::Ice::Byte>& outParams,
               const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        return ice_invoke(operation, mode, ::IceInternal::makePair(inParams), outParams, context);
    }

    /**
     * Invokes an operation dynamically.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams An encapsulation containing the encoded in-parameters for the operation.
     * @param context The context map for the invocation.
     * @return The future object for the invocation.
     */
    template<template<typename> class P = std::promise> auto
    ice_invokeAsync(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::vector<Byte>& inParams,
                    const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<::Ice::Object::Ice_invokeResult>>().get_future())
    {
        return ice_invokeAsync<P>(operation, mode, ::IceInternal::makePair(inParams), context);
    }

    /**
     * Invokes an operation dynamically.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams An encapsulation containing the encoded in-parameters for the operation.
     * @param response The response callback.
     * @param ex The exception callback.
     * @param sent The sent callback.
     * @param context The context map for the invocation.
     * @return A function that can be called to cancel the invocation locally.
     */
    ::std::function<void()>
    ice_invokeAsync(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::vector<::Ice::Byte>& inParams,
                    ::std::function<void(bool, ::std::vector<::Ice::Byte>)> response,
                    ::std::function<void(::std::exception_ptr)> ex = nullptr,
                    ::std::function<void(bool)> sent = nullptr,
                    const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        using Outgoing = ::IceInternal::InvokeLambdaOutgoing<::Ice::Object::Ice_invokeResult>;
        ::std::function<void(::Ice::Object::Ice_invokeResult&&)> r;
        if(response)
        {
#if ICE_CPLUSPLUS >= 201402L
            r = [response = std::move(response)](::Ice::Object::Ice_invokeResult&& result)
#else
            r = [response](::Ice::Object::Ice_invokeResult&& result)
#endif
            {
                response(result.returnValue, std::move(result.outParams));
            };
        }
        auto outAsync = ::std::make_shared<Outgoing>(shared_from_this(), std::move(r), std::move(ex), std::move(sent));
        outAsync->invoke(operation, mode, ::IceInternal::makePair(inParams), context);
        return [outAsync]() { outAsync->cancel(); };
    }

    /**
     * Invokes an operation dynamically.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams An encapsulation containing the encoded in-parameters for the operation.
     * @param outParams An encapsulation containing the encoded results.
     * @param context The context map for the invocation.
     * @return True if the operation completed successfully, in which case outParams contains
     * the encoded out parameters. False if the operation raised a user exception, in which
     * case outParams contains the encoded user exception. If the operation raises a run-time
     * exception, it throws it directly.
     */
    bool
    ice_invoke(const ::std::string& operation,
               ::Ice::OperationMode mode,
               const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
               ::std::vector<::Ice::Byte>& outParams,
               const ::Ice::Context& context = ::Ice::noExplicitContext)
    {
        using Outgoing = ::IceInternal::InvokePromiseOutgoing<
            ::std::promise<::Ice::Object::Ice_invokeResult>, ::Ice::Object::Ice_invokeResult>;
        auto outAsync = ::std::make_shared<Outgoing>(shared_from_this(), true);
        outAsync->invoke(operation, mode, inParams, context);
        auto result = outAsync->getFuture().get();
        outParams.swap(result.outParams);
        return result.returnValue;
    }

    /**
     * Invokes an operation dynamically.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams An encapsulation containing the encoded in-parameters for the operation.
     * @param context The context map for the invocation.
     * @return The future object for the invocation.
     */
    template<template<typename> class P = std::promise> auto
    ice_invokeAsync(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                    const ::Ice::Context& context = ::Ice::noExplicitContext)
        -> decltype(std::declval<P<::Ice::Object::Ice_invokeResult>>().get_future())
    {
        using Outgoing =
            ::IceInternal::InvokePromiseOutgoing<P<::Ice::Object::Ice_invokeResult>, ::Ice::Object::Ice_invokeResult>;
        auto outAsync = ::std::make_shared<Outgoing>(shared_from_this(), false);
        outAsync->invoke(operation, mode, inParams, context);
        return outAsync->getFuture();
    }

    /**
     * Invokes an operation dynamically.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams An encapsulation containing the encoded in-parameters for the operation.
     * @param response The response callback.
     * @param ex The exception callback.
     * @param sent The sent callback.
     * @param context The context map for the invocation.
     * @return A function that can be called to cancel the invocation locally.
     */
    ::std::function<void()>
    ice_invokeAsync(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
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
#if ICE_CPLUSPLUS >= 201402L
            r = [response = std::move(response)](Result&& result)
#else
            r = [response](Result&& result)
#endif
            {
                response(::std::get<0>(result), ::std::move(::std::get<1>(result)));
            };
        }
        auto outAsync = ::std::make_shared<Outgoing>(shared_from_this(), std::move(r), std::move(ex), std::move(sent));
        outAsync->invoke(operation, mode, inParams, context);
        return [outAsync]() { outAsync->cancel(); };
    }

    /**
     * Obtains the identity embedded in this proxy.
     * @return The identity of the target object.
     */
    ::Ice::Identity ice_getIdentity() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the identity.
     * @param id The identity for the new proxy.
     * @return A proxy with the new identity.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_identity(const ::Ice::Identity& id) const;

    /**
     * Obtains the per-proxy context for this proxy.
     * @return The per-proxy context.
     */
    ::Ice::Context ice_getContext() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the per-proxy context.
     * @param context The context for the new proxy.
     * @return A proxy with the new per-proxy context.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_context(const ::Ice::Context& context) const;

    /**
     * Obtains the facet for this proxy.
     * @return The facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
     */
    const ::std::string& ice_getFacet() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the facet.
     * @param facet The facet for the new proxy.
     * @return A proxy with the new facet.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_facet(const ::std::string& facet) const;

    /**
     * Obtains the adapter ID for this proxy.
     * @return The adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
     */
    ::std::string ice_getAdapterId() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the adapter ID.
     * @param id The adapter ID for the new proxy.
     * @return A proxy with the new adapter ID.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_adapterId(const ::std::string& id) const;

    /**
     * Obtains the endpoints used by this proxy.
     * @return The endpoints used by this proxy.
     */
    ::Ice::EndpointSeq ice_getEndpoints() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the endpoints.
     * @param endpoints The endpoints for the new proxy.
     * @return A proxy with the new endpoints.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_endpoints(const ::Ice::EndpointSeq& endpoints) const;

    /**
     * Obtains the locator cache timeout of this proxy.
     * @return The locator cache timeout value (in seconds).
     */
    ::Ice::Int ice_getLocatorCacheTimeout() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the locator cache timeout.
     * @param timeout The new locator cache timeout (in seconds).
     * @return A proxy with the new timeout.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_locatorCacheTimeout(::Ice::Int timeout) const;

    /**
     * Determines whether this proxy caches connections.
     * @return True if this proxy caches connections, false otherwise.
     */
    bool ice_isConnectionCached() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for connection caching.
     * @param b True if the new proxy should cache connections, false otherwise.
     * @return A proxy with the specified caching policy.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_connectionCached(bool b) const;

    /**
     * Obtains the endpoint selection policy for this proxy (randomly or ordered).
     * @return The endpoint selection policy.
     */
    ::Ice::EndpointSelectionType ice_getEndpointSelection() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the endpoint selection policy.
     * @param type The new endpoint selection policy.
     * @return A proxy with the specified endpoint selection policy.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_endpointSelection(::Ice::EndpointSelectionType type) const;

    /**
     * Determines whether this proxy uses only secure endpoints.
     * @return True if this proxy communicates only via secure endpoints, false otherwise.
     */
    bool ice_isSecure() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for how it selects endpoints.
     * @param b If true, only endpoints that use a secure transport are used by the new proxy.
     * If false, the returned proxy uses both secure and insecure endpoints.
     * @return A proxy with the specified security policy.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_secure(bool b) const;

    /**
     * Obtains the encoding version used to marshal request parameters.
     * @return The encoding version.
     */
    ::Ice::EncodingVersion ice_getEncodingVersion() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the encoding used to marshal
     * parameters.
     * @param version The encoding version to use to marshal request parameters.
     * @return A proxy with the specified encoding version.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_encodingVersion(const ::Ice::EncodingVersion& version) const;

    /**
     * Determines whether this proxy prefers secure endpoints.
     * @return True if the proxy always attempts to invoke via secure endpoints before it
     * attempts to use insecure endpoints, false otherwise.
     */
    bool ice_isPreferSecure() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for its endpoint selection policy.
     * @param b If true, the new proxy will use secure endpoints for invocations and only use
     * insecure endpoints if an invocation cannot be made via secure endpoints. If false, the
     * proxy prefers insecure endpoints to secure ones.
     * @return A proxy with the specified selection policy.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_preferSecure(bool b) const;

    /**
     * Obtains the router for this proxy.
     * @return The router for the proxy. If no router is configured for the proxy, the return value
     * is nil.
     */
    ::std::shared_ptr<::Ice::RouterPrx> ice_getRouter() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the router.
     * @param router The router for the new proxy.
     * @return A proxy with the specified router.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_router(const ::std::shared_ptr<::Ice::RouterPrx>& router) const;

    /**
     * Obtains the locator for this proxy.
     * @return The locator for this proxy. If no locator is configured, the return value is nil.
     */
    ::std::shared_ptr<::Ice::LocatorPrx> ice_getLocator() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the locator.
     * @param locator The locator for the new proxy.
     * @return A proxy with the specified locator.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_locator(const ::std::shared_ptr<::Ice::LocatorPrx>& locator) const;

    /**
     * Determines whether this proxy uses collocation optimization.
     * @return True if the proxy uses collocation optimization, false otherwise.
     */
    bool ice_isCollocationOptimized() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for collocation optimization.
     * @param b True if the new proxy enables collocation optimization, false otherwise.
     * @return A proxy with the specified collocation optimization.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_collocationOptimized(bool b) const;

    /**
     * Obtains the invocation timeout of this proxy.
     * @return The invocation timeout value (in milliseconds).
     */
    ::Ice::Int ice_getInvocationTimeout() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for the invocation timeout.
     * @param timeout The new invocation timeout (in milliseconds).
     * @return A proxy with the new timeout.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_invocationTimeout(::Ice::Int timeout) const;

    /**
     * Obtains a proxy that is identical to this proxy, but uses twoway invocations.
     * @return A proxy that uses twoway invocations.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_twoway() const;

    /**
     * Determines whether this proxy uses twoway invocations.
     * @return True if this proxy uses twoway invocations, false otherwise.
     */
    bool ice_isTwoway() const;

    /**
     * Obtains a proxy that is identical to this proxy, but uses oneway invocations.
     * @return A proxy that uses oneway invocations.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_oneway() const;

    /**
     * Determines whether this proxy uses oneway invocations.
     * @return True if this proxy uses oneway invocations, false otherwise.
     */
    bool ice_isOneway() const;

    /**
     * Obtains a proxy that is identical to this proxy, but uses batch oneway invocations.
     * @return A proxy that uses batch oneway invocations.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_batchOneway() const;

    /**
     * Determines whether this proxy uses batch oneway invocations.
     * @return True if this proxy uses batch oneway invocations, false otherwise.
     */
    bool ice_isBatchOneway() const;

    /**
     * Obtains a proxy that is identical to this proxy, but uses datagram invocations.
     * @return A proxy that uses datagram invocations.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_datagram() const;

    /**
     * Determines whether this proxy uses datagram invocations.
     * @return True if this proxy uses datagram invocations, false otherwise.
     */
    bool ice_isDatagram() const;

    /**
     * Obtains a proxy that is identical to this proxy, but uses batch datagram invocations.
     * @return A proxy that uses batch datagram invocations.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_batchDatagram() const;

    /**
     * Determines whether this proxy uses batch datagram invocations.
     * @return True if this proxy uses batch datagram invocations, false otherwise.
     */
    bool ice_isBatchDatagram() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for its compression setting which
     * overrides the compression setting from the proxy endpoints.
     * @param b True enables compression for the new proxy, false disables compression.
     * @return A proxy with the specified compression override setting.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_compress(bool b) const;

    /**
     * Obtains the compression override setting of this proxy.
     * @return The compression override setting. If nullopt is returned, no override is set. Otherwise, true
     * if compression is enabled, false otherwise.
     */
    ::Ice::optional<bool> ice_getCompress() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for its connection timeout setting
     * which overrides the timeot setting from the proxy endpoints.
     * @param timeout The connection timeout override for the proxy (in milliseconds).
     * @return A proxy with the specified timeout override.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_timeout(int timeout) const;

    /**
     * Obtains the timeout override of this proxy.
     * @return The timeout override. If nullopt is returned, no override is set. Otherwise, returns
     * the timeout override value.
     */
    ::Ice::optional<int> ice_getTimeout() const;

    /**
     * Obtains a proxy that is identical to this proxy, except for its connection ID.
     * @param id The connection ID for the new proxy. An empty string removes the
     * connection ID.
     * @return A proxy with the specified connection ID.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_connectionId(const ::std::string& id) const;

    /**
     * Obtains the connection ID of this proxy.
     * @return The connection ID.
     */
    ::std::string ice_getConnectionId() const;

    /**
     * Obtains a proxy that is identical to this proxy, except it's a fixed proxy bound
     * the given connection.
     * @param connection The fixed proxy connection.
     * @return A fixed proxy bound to the given connection.
     */
    ::std::shared_ptr<::Ice::ObjectPrx> ice_fixed(const ::std::shared_ptr<::Ice::Connection>& connection) const;

    /**
     * Determines whether this proxy is a fixed proxy.
     * @return True if this proxy is a fixed proxy, false otherwise.
     */
    bool ice_isFixed() const;

    /**
     * Obtains the Connection for this proxy. If the proxy does not yet have an established connection,
     * it first attempts to create a connection.
     * @return The connection for this proxy.
     */
    ::std::shared_ptr<::Ice::Connection>
    ice_getConnection()
    {
        return ice_getConnectionAsync().get();
    }

    /**
     * Obtains the Connection for this proxy. If the proxy does not yet have an established connection,
     * it first attempts to create a connection.
     * @param response The response callback.
     * @param ex The exception callback.
     * @param sent The sent callback.
     * @return A function that can be called to cancel the invocation locally.
     */
    ::std::function<void()>
    ice_getConnectionAsync(::std::function<void(::std::shared_ptr<::Ice::Connection>)> response,
                           ::std::function<void(::std::exception_ptr)> ex = nullptr,
                           ::std::function<void(bool)> sent = nullptr)
    {
        using LambdaOutgoing = ::IceInternal::ProxyGetConnectionLambda;
        auto outAsync = ::std::make_shared<LambdaOutgoing>(shared_from_this(), std::move(response), std::move(ex), std::move(sent));
        _iceI_getConnection(outAsync);
        return [outAsync]() { outAsync->cancel(); };
    }

    /**
     * Obtains the Connection for this proxy. If the proxy does not yet have an established connection,
     * it first attempts to create a connection.
     * @return The future object for the invocation.
     */
    template<template<typename> class P = std::promise> auto
    ice_getConnectionAsync() -> decltype(std::declval<P<::std::shared_ptr<::Ice::Connection>>>().get_future())
    {
        using PromiseOutgoing = ::IceInternal::ProxyGetConnectionPromise<P<::std::shared_ptr<::Ice::Connection>>>;
        auto outAsync = ::std::make_shared<PromiseOutgoing>(shared_from_this());
        _iceI_getConnection(outAsync);
        return outAsync->getFuture();
    }

    /// \cond INTERNAL
    void _iceI_getConnection(const ::std::shared_ptr<::IceInternal::ProxyGetConnection>&);
    /// \endcond

    /**
     * Obtains the cached Connection for this proxy. If the proxy does not yet have an established
     * connection, it does not attempt to create a connection.
     * @return The cached connection for this proxy, or nil if the proxy does not have
     * an established connection.
     */
    ::std::shared_ptr<::Ice::Connection> ice_getCachedConnection() const;

    /**
     * Flushes any pending batched requests for this communicator. The call blocks until the flush is complete.
     */
    void ice_flushBatchRequests()
    {
        return ice_flushBatchRequestsAsync().get();
    }

    /**
     * Flushes asynchronously any pending batched requests for this communicator.
     * @param ex The exception callback.
     * @param sent The sent callback.
     * @return A function that can be called to cancel the invocation locally.
     */
    std::function<void()>
    ice_flushBatchRequestsAsync(::std::function<void(::std::exception_ptr)> ex,
                                ::std::function<void(bool)> sent = nullptr)
    {
        using LambdaOutgoing = ::IceInternal::ProxyFlushBatchLambda;
        auto outAsync = ::std::make_shared<LambdaOutgoing>(shared_from_this(), std::move(ex), std::move(sent));
        _iceI_flushBatchRequests(outAsync);
        return [outAsync]() { outAsync->cancel(); };
    }

    /**
     * Flushes asynchronously any pending batched requests for this communicator.
     * @return The future object for the invocation.
     */
    template<template<typename> class P = std::promise> auto
    ice_flushBatchRequestsAsync() -> decltype(std::declval<P<void>>().get_future())
    {
        using PromiseOutgoing = ::IceInternal::ProxyFlushBatchPromise<P<void>>;
        auto outAsync = ::std::make_shared<PromiseOutgoing>(shared_from_this());
        _iceI_flushBatchRequests(outAsync);
        return outAsync->getFuture();
    }

    /// \cond INTERNAL
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
    /// \endcond

protected:

    /// \cond INTERNAL
    template<typename R, template<typename> class P = ::std::promise, typename Obj, typename Fn, typename... Args>
    auto _makePromiseOutgoing(bool sync, Obj obj, Fn fn, Args&&... args)
        -> decltype(std::declval<P<R>>().get_future())
    {
        auto outAsync = ::std::make_shared<::IceInternal::PromiseOutgoing<P<R>, R>>(shared_from_this(), sync);
        (obj->*fn)(outAsync, std::forward<Args>(args)...);
        return outAsync->getFuture();
    }

    template<typename R, typename Re, typename E, typename S, typename Obj, typename Fn, typename... Args>
    ::std::function<void()> _makeLambdaOutgoing(Re r, E e, S s, Obj obj, Fn fn, Args&&... args)
    {
        auto outAsync = ::std::make_shared<::IceInternal::LambdaOutgoing<R>>(shared_from_this(),
                                                                             std::move(r), std::move(e), std::move(s));
        (obj->*fn)(outAsync, std::forward<Args>(args)...);
        return [outAsync]() { outAsync->cancel(); };
    }

    virtual ::std::shared_ptr<ObjectPrx> _newInstance() const;
    ObjectPrx() = default;
    friend ::std::shared_ptr<ObjectPrx> IceInternal::createProxy<ObjectPrx>();
    /// \endcond

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

/**
 * Helper template that supplies proxy factory functions.
 * \headerfile Ice/Ice.h
 */
template<typename Prx, typename... Bases>
class Proxy : public virtual Bases...
{
public:

    /**
     * Obtains a proxy that is identical to this proxy, except for the per-proxy context.
     * @param context The context for the new proxy.
     * @return A proxy with the new per-proxy context.
     */
    ::std::shared_ptr<Prx> ice_context(const ::Ice::Context& context) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_context(context));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for the adapter ID.
     * @param id The adapter ID for the new proxy.
     * @return A proxy with the new adapter ID.
     */
    ::std::shared_ptr<Prx> ice_adapterId(const ::std::string& id) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_adapterId(id));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for the endpoints.
     * @param endpoints The endpoints for the new proxy.
     * @return A proxy with the new endpoints.
     */
    ::std::shared_ptr<Prx> ice_endpoints(const ::Ice::EndpointSeq& endpoints) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_endpoints(endpoints));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for the locator cache timeout.
     * @param timeout The new locator cache timeout (in seconds).
     * @return A proxy with the new timeout.
     */
    ::std::shared_ptr<Prx> ice_locatorCacheTimeout(int timeout) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_locatorCacheTimeout(timeout));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for connection caching.
     * @param b True if the new proxy should cache connections, false otherwise.
     * @return A proxy with the specified caching policy.
     */
    ::std::shared_ptr<Prx> ice_connectionCached(bool b) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_connectionCached(b));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for the endpoint selection policy.
     * @param type The new endpoint selection policy.
     * @return A proxy with the specified endpoint selection policy.
     */
    ::std::shared_ptr<Prx> ice_endpointSelection(::Ice::EndpointSelectionType type) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_endpointSelection(type));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for how it selects endpoints.
     * @param b If true, only endpoints that use a secure transport are used by the new proxy.
     * If false, the returned proxy uses both secure and insecure endpoints.
     * @return A proxy with the specified security policy.
     */
    ::std::shared_ptr<Prx> ice_secure(bool b) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_secure(b));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for its endpoint selection policy.
     * @param b If true, the new proxy will use secure endpoints for invocations and only use
     * insecure endpoints if an invocation cannot be made via secure endpoints. If false, the
     * proxy prefers insecure endpoints to secure ones.
     * @return A proxy with the specified selection policy.
     */
    ::std::shared_ptr<Prx> ice_preferSecure(bool b) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_preferSecure(b));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for the router.
     * @param router The router for the new proxy.
     * @return A proxy with the specified router.
     */
    ::std::shared_ptr<Prx> ice_router(const ::std::shared_ptr<::Ice::RouterPrx>& router) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_router(router));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for the locator.
     * @param locator The locator for the new proxy.
     * @return A proxy with the specified locator.
     */
    ::std::shared_ptr<Prx> ice_locator(const ::std::shared_ptr<::Ice::LocatorPrx>& locator) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_locator(locator));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for collocation optimization.
     * @param b True if the new proxy enables collocation optimization, false otherwise.
     * @return A proxy with the specified collocation optimization.
     */
    ::std::shared_ptr<Prx> ice_collocationOptimized(bool b) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_collocationOptimized(b));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for the invocation timeout.
     * @param timeout The new invocation timeout (in milliseconds).
     * @return A proxy with the new timeout.
     */
    ::std::shared_ptr<Prx> ice_invocationTimeout(int timeout) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_invocationTimeout(timeout));
    }

    /**
     * Obtains a proxy that is identical to this proxy, but uses twoway invocations.
     * @return A proxy that uses twoway invocations.
     */
    ::std::shared_ptr<Prx> ice_twoway() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_twoway());
    }

    /**
     * Obtains a proxy that is identical to this proxy, but uses oneway invocations.
     * @return A proxy that uses oneway invocations.
     */
    ::std::shared_ptr<Prx> ice_oneway() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_oneway());
    }

    /**
     * Obtains a proxy that is identical to this proxy, but uses batch oneway invocations.
     * @return A proxy that uses batch oneway invocations.
     */
    ::std::shared_ptr<Prx> ice_batchOneway() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_batchOneway());
    }

    /**
     * Obtains a proxy that is identical to this proxy, but uses datagram invocations.
     * @return A proxy that uses datagram invocations.
     */
    ::std::shared_ptr<Prx> ice_datagram() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_datagram());
    }

    /**
     * Obtains a proxy that is identical to this proxy, but uses batch datagram invocations.
     * @return A proxy that uses batch datagram invocations.
     */
    ::std::shared_ptr<Prx> ice_batchDatagram() const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_batchDatagram());
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for its compression setting which
     * overrides the compression setting from the proxy endpoints.
     * @param b True enables compression for the new proxy, false disables compression.
     * @return A proxy with the specified compression override setting.
     */
    ::std::shared_ptr<Prx> ice_compress(bool b) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_compress(b));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for its connection timeout setting
     * which overrides the timeot setting from the proxy endpoints.
     * @param timeout The connection timeout override for the proxy (in milliseconds).
     * @return A proxy with the specified timeout override.
     */
    ::std::shared_ptr<Prx> ice_timeout(int timeout) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_timeout(timeout));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for its connection ID.
     * @param id The connection ID for the new proxy. An empty string removes the
     * connection ID.
     * @return A proxy with the specified connection ID.
     */
    ::std::shared_ptr<Prx> ice_connectionId(const ::std::string& id) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_connectionId(id));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except it's a fixed proxy bound
     * the given connection.
     * @param connection The fixed proxy connection.
     * @return A fixed proxy bound to the given connection.
     */
    ::std::shared_ptr<Prx> ice_fixed(const ::std::shared_ptr<::Ice::Connection>& connection) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_fixed(connection));
    }

    /**
     * Obtains a proxy that is identical to this proxy, except for the encoding used to marshal
     * parameters.
     * @param version The encoding version to use to marshal request parameters.
     * @return A proxy with the specified encoding version.
     */
    ::std::shared_ptr<Prx> ice_encodingVersion(const ::Ice::EncodingVersion& version) const
    {
        return ::std::dynamic_pointer_cast<Prx>(ObjectPrx::ice_encodingVersion(version));
    }

protected:

    /// \cond INTERNAL
    virtual ::std::shared_ptr<ObjectPrx> _newInstance() const = 0;
    /// \endcond
};

ICE_API ::std::ostream& operator<<(::std::ostream&, const ::Ice::ObjectPrx&);

/**
 * Compares the object identities of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity in lhs compares less than the identity in rhs, false otherwise.
 */
ICE_API bool proxyIdentityLess(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs);

/**
 * Compares the object identities of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity in lhs compares equal to the identity in rhs, false otherwise.
 */
ICE_API bool proxyIdentityEqual(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs);

/**
 * Compares the object identities and facets of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity and facet in lhs compare less than the identity and facet
 * in rhs, false otherwise.
 */
ICE_API bool proxyIdentityAndFacetLess(const ::std::shared_ptr<ObjectPrx>& lhs,
                                       const ::std::shared_ptr<ObjectPrx>& rhs);

/**
 * Compares the object identities and facets of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity and facet in lhs compare equal to the identity and facet
 * in rhs, false otherwise.
 */
ICE_API bool proxyIdentityAndFacetEqual(const ::std::shared_ptr<ObjectPrx>& lhs,
                                        const ::std::shared_ptr<ObjectPrx>& rhs);

/**
 * A functor that compares the object identities of two proxies. Evaluates true if the identity in lhs
 * compares less than the identity in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */
#if (ICE_CPLUSPLUS >= 201703L)
struct ProxyIdentityLess
#else
struct ProxyIdentityLess : std::binary_function<bool, ::std::shared_ptr<ObjectPrx>&, ::std::shared_ptr<ObjectPrx>&>
#endif
{
    bool operator()(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs) const
    {
        return proxyIdentityLess(lhs, rhs);
    }
};

/**
 * A functor that compares the object identities of two proxies. Evaluates true if the identity in lhs
 * compares equal to the identity in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */
#if (ICE_CPLUSPLUS >= 201703L)
struct ProxyIdentityEqual
#else
struct ProxyIdentityEqual : std::binary_function<bool, ::std::shared_ptr<ObjectPrx>&, ::std::shared_ptr<ObjectPrx>&>
#endif
{
    bool operator()(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs) const
    {
        return proxyIdentityEqual(lhs, rhs);
    }
};

/**
 * A functor that compares the object identities and facets of two proxies. Evaluates true if the identity
 * and facet in lhs compare less than the identity and facet in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */
#if (ICE_CPLUSPLUS >= 201703L)
struct ProxyIdentityAndFacetLess
#else
struct ProxyIdentityAndFacetLess : std::binary_function<bool, ::std::shared_ptr<ObjectPrx>&, ::std::shared_ptr<ObjectPrx>&>
#endif
{
    bool operator()(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs) const
    {
        return proxyIdentityAndFacetLess(lhs, rhs);
    }
};

/**
 * A functor that compares the object identities and facets of two proxies. Evaluates true if the identity
 * and facet in lhs compare equal to the identity and facet in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */
#if (ICE_CPLUSPLUS >= 201703L)
struct ProxyIdentityAndFacetEqual
#else
struct ProxyIdentityAndFacetEqual : std::binary_function<bool, ::std::shared_ptr<ObjectPrx>&, ::std::shared_ptr<ObjectPrx>&>
#endif
{
    bool operator()(const ::std::shared_ptr<ObjectPrx>& lhs, const ::std::shared_ptr<ObjectPrx>& rhs) const
    {
        return proxyIdentityAndFacetEqual(lhs, rhs);
    }
};

/**
 * Downcasts a proxy without confirming the target object's type via a remote invocation.
 * @param b The target proxy.
 * @return A proxy with the requested type.
 */
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

/**
 * Downcasts a proxy without confirming the target object's type via a remote invocation.
 * @param b The target proxy.
 * @param f A facet name.
 * @return A proxy with the requested type and facet.
 */
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

/**
 * Downcasts a proxy after confirming the target object's type via a remote invocation.
 * @param b The target proxy.
 * @param context The context map for the invocation.
 * @return A proxy with the requested type, or nil if the target proxy is nil or the target
 * object does not support the requested type.
 */
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

/**
 * Downcasts a proxy after confirming the target object's type via a remote invocation.
 * @param b The target proxy.
 * @param f A facet name.
 * @param context The context map for the invocation.
 * @return A proxy with the requested type and facet, or nil if the target proxy is nil or the target
 * object does not support the requested type.
 */
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

}

#endif
