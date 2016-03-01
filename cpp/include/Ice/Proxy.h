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
#include <Ice/ObjectF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ReferenceF.h>
#include <Ice/BatchRequestQueueF.h>
#include <Ice/AsyncResult.h>
//#include <Ice/RouterF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
//#include <Ice/LocatorF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
#include <Ice/Current.h>
#include <Ice/StreamF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ObserverHelper.h>
#include <iosfwd>

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

namespace IceInternal
{

class Outgoing;

}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Router> RouterPrx;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Locator> LocatorPrx;

class LocalException;

ICE_API void ice_writeObjectPrx(const ::Ice::OutputStreamPtr&, const ObjectPrx&);
ICE_API void ice_readObjectPrx(const ::Ice::InputStreamPtr&, ObjectPrx&);

class Callback_Object_ice_isA_Base : virtual public ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_isA_Base> Callback_Object_ice_isAPtr;

class Callback_Object_ice_ping_Base : virtual public ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_ping_Base> Callback_Object_ice_pingPtr;

class Callback_Object_ice_ids_Base : virtual public ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_ids_Base> Callback_Object_ice_idsPtr;

class Callback_Object_ice_id_Base : virtual public ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_id_Base> Callback_Object_ice_idPtr;

class Callback_Object_ice_invoke_Base : virtual public ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_invoke_Base> Callback_Object_ice_invokePtr;

class Callback_Object_ice_flushBatchRequests_Base : virtual public ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_flushBatchRequests_Base> Callback_Object_ice_flushBatchRequestsPtr;

class Callback_Object_ice_getConnection_Base : virtual public ::IceInternal::CallbackBase { };
typedef ::IceUtil::Handle< Callback_Object_ice_getConnection_Base> Callback_Object_ice_getConnectionPtr;

}

#ifdef ICE_CPP11
namespace IceInternal
{

class ICE_API Cpp11FnCallbackNC : public CallbackBase
{
public:

    Cpp11FnCallbackNC(const ::std::function<void (const ::Ice::Exception&)>&,
                      const ::std::function<void (bool)>&);

    virtual CallbackBasePtr verify(const ::Ice::LocalObjectPtr&);

    virtual void sent(const ::Ice::AsyncResultPtr&) const;

    virtual bool hasSentCallback() const;

protected:

    void exception(const ::Ice::AsyncResultPtr&, const ::Ice::Exception& ex) const;

    ::std::function<void (const ::Ice::Exception&)> _exception;
    ::std::function<void (bool)> _sent;
};

class ICE_API Cpp11FnOnewayCallbackNC : public Cpp11FnCallbackNC
{
public:

    Cpp11FnOnewayCallbackNC(const ::std::function<void ()>&,
                            const ::std::function<void (const ::Ice::Exception&)>&,
                            const ::std::function<void (bool)>&);

    virtual void
    completed(const ::Ice::AsyncResultPtr&) const;

private:

    ::std::function<void ()> _cb;
};

}
#endif

namespace IceProxy { namespace Ice
{

class ICE_API Object : public ::IceUtil::Shared
{
public:

    bool operator==(const Object&) const;
    bool operator!=(const Object&) const;
    bool operator<(const Object&) const;

    ::Ice::CommunicatorPtr ice_getCommunicator() const;

    ::std::string ice_toString() const;

    bool ice_isA(const ::std::string& typeId)
    {
        return ice_isA(typeId, 0);
    }
    bool ice_isA(const ::std::string& typeId, const ::Ice::Context& context)
    {
        return ice_isA(typeId, &context);
    }

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr
    begin_ice_isA(const ::std::string& typeId,
                  const ::Ice::Context& ctx,
                  const ::IceInternal::Function<void (bool)>& response,
                  const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                  ::IceInternal::Function<void (const ::Ice::Exception&)>(),
                  const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_isA(typeId, &ctx, response, exception, sent);
    }

    ::Ice::AsyncResultPtr
    begin_ice_isA(const ::std::string& typeId,
                  const ::IceInternal::Function<void (bool)>& response,
                  const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                  ::IceInternal::Function<void (const ::Ice::Exception&)>(),
                  const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_isA(typeId, 0, response, exception, sent);
    }

    ::Ice::AsyncResultPtr
    begin_ice_isA(const ::std::string& typeId,
                  const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
                  const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent =
                  ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
    {
        return begin_ice_isA(typeId, 0, ::Ice::newCallback(completed, sent), 0);
    }

    ::Ice::AsyncResultPtr
    begin_ice_isA(const ::std::string& typeId,
                  const ::Ice::Context& ctx,
                  const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
                  const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent =
                  ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
    {
        return begin_ice_isA(typeId, &ctx, ::Ice::newCallback(completed, sent), 0);
    }
#endif

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId)
    {
        return begin_ice_isA(typeId, 0, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId, const ::Ice::Context& __ctx)
    {
        return begin_ice_isA(typeId, &__ctx, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::CallbackPtr& __del,
                                        const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_isA(typeId, 0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::Context& __ctx,
                                        const ::Ice::CallbackPtr& __del,
                                        const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_isA(typeId, &__ctx, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::Callback_Object_ice_isAPtr& __del,
                                        const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_isA(typeId, 0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string& typeId,
                                        const ::Ice::Context& __ctx,
                                        const ::Ice::Callback_Object_ice_isAPtr& __del,
                                        const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_isA(typeId, &__ctx, __del, __cookie);
    }

    bool end_ice_isA(const ::Ice::AsyncResultPtr&);

    void ice_ping()
    {
        ice_ping(0);
    }
    void ice_ping(const ::Ice::Context& context)
    {
        ice_ping(&context);
    }

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr
    begin_ice_ping(const ::IceInternal::Function<void ()>& response,
                   const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                   ::IceInternal::Function<void (const ::Ice::Exception&)>(),
                   const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_ping(0, response, exception, sent);
    }

    ::Ice::AsyncResultPtr
    begin_ice_ping(const ::Ice::Context& ctx,
                   const ::IceInternal::Function<void ()>& response,
                   const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                   ::IceInternal::Function<void (const ::Ice::Exception&)>(),
                   const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_ping(&ctx, response, exception, sent);
    }

    ::Ice::AsyncResultPtr
    begin_ice_ping(const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
                   const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent =
                   ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
    {
        return begin_ice_ping(0, ::Ice::newCallback(completed, sent), 0);
    }

    ::Ice::AsyncResultPtr
    begin_ice_ping(const ::Ice::Context& ctx,
                   const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
                   const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent =
                   ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
    {
        return begin_ice_ping(&ctx, ::Ice::newCallback(completed, sent), 0);
    }
#endif

    ::Ice::AsyncResultPtr begin_ice_ping()
    {
        return begin_ice_ping(0, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Context& __ctx)
    {
        return begin_ice_ping(&__ctx, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::CallbackPtr& __del,
                                         const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_ping(0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Context& __ctx, const ::Ice::CallbackPtr& __del,
                                         const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_ping(&__ctx, __del, __cookie);
    }


    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Callback_Object_ice_pingPtr& __del,
                                         const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_ping(0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Context& __ctx, const ::Ice::Callback_Object_ice_pingPtr& __del,
                                         const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_ping(&__ctx, __del, __cookie);
    }

    void end_ice_ping(const ::Ice::AsyncResultPtr&);

    ::std::vector< ::std::string> ice_ids()
    {
        return ice_ids(0);
    }
    ::std::vector< ::std::string> ice_ids(const ::Ice::Context& context)
    {
        return ice_ids(&context);
    }

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr
    begin_ice_ids(const ::IceInternal::Function<void (const ::std::vector< ::std::string>&)>& response,
                  const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                  ::IceInternal::Function<void (const ::Ice::Exception&)>(),
                  const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_ids(0, response, exception, sent);
    }

    ::Ice::AsyncResultPtr
    begin_ice_ids(const ::Ice::Context& ctx,
                  const ::IceInternal::Function<void (const ::std::vector< ::std::string>&)>& response,
                  const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                                                            ::IceInternal::Function<void (const ::Ice::Exception&)>(),
                  const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_ids(&ctx, response, exception, sent);
    }

    ::Ice::AsyncResultPtr
    begin_ice_ids(const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
                  const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent =
                  ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
    {
        return begin_ice_ids(0, ::Ice::newCallback(completed, sent), 0);
    }

    ::Ice::AsyncResultPtr
    begin_ice_ids(const ::Ice::Context& ctx,
                  const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
                  const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent =
                  ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
    {
        return begin_ice_ids(&ctx, ::Ice::newCallback(completed, sent), 0);
    }
#endif

    ::Ice::AsyncResultPtr begin_ice_ids()
    {
        return begin_ice_ids(0, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Context& __ctx)
    {
        return begin_ice_ids(&__ctx, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::CallbackPtr& __del,
                                        const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_ids(0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Context& __ctx,
                                        const ::Ice::CallbackPtr& __del,
                                        const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_ids(&__ctx, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Callback_Object_ice_idsPtr& __del,
                                        const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_ids(0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Context& __ctx,
                                        const ::Ice::Callback_Object_ice_idsPtr& __del,
                                        const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_ids(&__ctx, __del, __cookie);
    }

    ::std::vector< ::std::string> end_ice_ids(const ::Ice::AsyncResultPtr&);

    ::std::string ice_id()
    {
        return ice_id(0);
    }
    ::std::string ice_id(const ::Ice::Context& context)
    {
        return ice_id(&context);
    }

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr
    begin_ice_id(const ::IceInternal::Function<void (const ::std::string&)>& response,
                 const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                 ::IceInternal::Function<void (const ::Ice::Exception&)>(),
                 const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_id(0, response, exception, sent);
    }

    ::Ice::AsyncResultPtr
    begin_ice_id(const ::Ice::Context& ctx,
                 const ::IceInternal::Function<void (const ::std::string&)>& response,
                 const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                 ::IceInternal::Function<void (const ::Ice::Exception&)>(),
                 const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_id(&ctx, response, exception, sent);
    }

    ::Ice::AsyncResultPtr
    begin_ice_id(const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
                 const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent =
                 ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
    {
        return begin_ice_id(0, ::Ice::newCallback(completed, sent), 0);
    }

    ::Ice::AsyncResultPtr
    begin_ice_id(const ::Ice::Context& ctx,
                 const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
                 const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent =
                 ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
    {
        return begin_ice_id(&ctx, ::Ice::newCallback(completed, sent), 0);
    }
#endif

    ::Ice::AsyncResultPtr begin_ice_id()
    {
        return begin_ice_id(0, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Context& __ctx)
    {
        return begin_ice_id(&__ctx, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::CallbackPtr& __del,
                                       const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_id(0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Context& __ctx,
                                       const ::Ice::CallbackPtr& __del,
                                       const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_id(&__ctx, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Callback_Object_ice_idPtr& __del,
                                       const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_id(0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Context& __ctx,
                                       const ::Ice::Callback_Object_ice_idPtr& __del,
                                       const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_id(&__ctx, __del, __cookie);
    }

    ::std::string end_ice_id(const ::Ice::AsyncResultPtr&);


    static const ::std::string& ice_staticId()
    {
        return ::Ice::Object::ice_staticId();
    }


    // Returns true if ok, false if user exception.
    bool ice_invoke(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::vector< ::Ice::Byte>& inParams,
                    ::std::vector< ::Ice::Byte>& outParams)
    {
        return ice_invoke(operation, mode, inParams, outParams, 0);
    }

    bool ice_invoke(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::vector< ::Ice::Byte>& inParams,
                    ::std::vector< ::Ice::Byte>& outParams,
                    const ::Ice::Context& context)
    {
        return ice_invoke(operation, mode, inParams, outParams, &context);
    }

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr begin_ice_invoke(
        const ::std::string& operation,
        ::Ice::OperationMode mode,
        const ::std::vector< ::Ice::Byte>& inParams,
        const ::IceInternal::Function<void (bool, const ::std::vector< ::Ice::Byte>&)>& response,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
           ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_invoke(operation, mode, inParams, 0, response, exception, sent);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(
        const ::std::string& operation,
        ::Ice::OperationMode mode,
        const ::std::vector< ::Ice::Byte>& inParams,
        const ::Ice::Context& ctx,
        const ::IceInternal::Function<void (bool, const ::std::vector< ::Ice::Byte>&)>& response,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
           ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_invoke(operation, mode, inParams, &ctx, response, exception, sent);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(
            const ::std::string& operation,
            ::Ice::OperationMode mode,
            const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
            const ::IceInternal::Function<void (bool, const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&)>& response,
            const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
                                                            ::IceInternal::Function<void (const ::Ice::Exception&)>(),
            const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_invoke(operation, mode, inParams, 0, response, exception, sent);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(
        const ::std::string& operation,
        ::Ice::OperationMode mode,
        const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
        const ::Ice::Context& ctx,
        const ::IceInternal::Function<void (bool, const ::std::pair<const ::Ice::Byte*,
                                                                    const ::Ice::Byte*>&)>& response,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
        ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_ice_invoke(operation, mode, inParams, &ctx, response, exception, sent);
    }
#endif

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams)
    {
        return begin_ice_invoke(operation, mode, inParams, 0, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::Context& __ctx)
    {
        return begin_ice_invoke(operation, mode, inParams, &__ctx, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::CallbackPtr& __del,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, 0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::Context& __ctx,
                                           const ::Ice::CallbackPtr& __del,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, &__ctx, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::Callback_Object_ice_invokePtr& __del,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, 0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::vector< ::Ice::Byte>& inParams,
                                           const ::Ice::Context& __ctx,
                                           const ::Ice::Callback_Object_ice_invokePtr& __del,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, &__ctx, __del, __cookie);
    }

    bool end_ice_invoke(::std::vector< ::Ice::Byte>&, const ::Ice::AsyncResultPtr&);

    bool ice_invoke(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                    ::std::vector< ::Ice::Byte>& outParams)
    {
        return ice_invoke(operation, mode, inParams, outParams, 0);
    }

    bool ice_invoke(const ::std::string& operation,
                    ::Ice::OperationMode mode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                    ::std::vector< ::Ice::Byte>& outParams,
                    const ::Ice::Context& context)
    {
        return ice_invoke(operation, mode, inParams, outParams, &context);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams)
    {
        return begin_ice_invoke(operation, mode, inParams, 0, ::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::Context& __ctx,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, &__ctx, ::IceInternal::__dummyCallback, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::CallbackPtr& __del,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, 0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::Context& __ctx,
                                           const ::Ice::CallbackPtr& __del,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, &__ctx, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::Callback_Object_ice_invokePtr& __del,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, 0, __del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string& operation,
                                           ::Ice::OperationMode mode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                                           const ::Ice::Context& __ctx,
                                           const ::Ice::Callback_Object_ice_invokePtr& __del,
                                           const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_invoke(operation, mode, inParams, &__ctx, __del, __cookie);
    }

    bool ___end_ice_invoke(::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&, const ::Ice::AsyncResultPtr&);

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

    ::Ice::ConnectionPtr ice_getConnection();

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr begin_ice_getConnection(
        const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed)
    {
        return begin_ice_getConnectionInternal(::Ice::newCallback(completed, 0), 0);
    }

    ::Ice::AsyncResultPtr begin_ice_getConnection(
        const ::IceInternal::Function<void (const ::Ice::ConnectionPtr&)>& response,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
            ::IceInternal::Function<void (const ::Ice::Exception&)>());
#endif

    ::Ice::AsyncResultPtr begin_ice_getConnection()
    {
        return begin_ice_getConnectionInternal(::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_getConnection(const ::Ice::CallbackPtr& __del,
                                                  const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_getConnectionInternal(__del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_getConnection(const ::Ice::Callback_Object_ice_getConnectionPtr& __del,
                                                  const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_getConnectionInternal(__del, __cookie);
    }

    ::Ice::ConnectionPtr end_ice_getConnection(const ::Ice::AsyncResultPtr&);

    ::Ice::ConnectionPtr ice_getCachedConnection() const;

    void ice_flushBatchRequests();

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr begin_ice_flushBatchRequests(
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception,
        const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return begin_ice_flushBatchRequestsInternal(
            new ::IceInternal::Cpp11FnOnewayCallbackNC(nullptr, exception, sent), 0);
    }
#endif

    ::Ice::AsyncResultPtr begin_ice_flushBatchRequests()
    {
        return begin_ice_flushBatchRequestsInternal(::IceInternal::__dummyCallback, 0);
    }

    ::Ice::AsyncResultPtr begin_ice_flushBatchRequests(const ::Ice::CallbackPtr& __del,
                                                       const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_flushBatchRequestsInternal(__del, __cookie);
    }

    ::Ice::AsyncResultPtr begin_ice_flushBatchRequests(const ::Ice::Callback_Object_ice_flushBatchRequestsPtr& __del,
                                                       const ::Ice::LocalObjectPtr& __cookie = 0)
    {
        return begin_ice_flushBatchRequestsInternal(__del, __cookie);
    }

    void end_ice_flushBatchRequests(const ::Ice::AsyncResultPtr&);

    const ::IceInternal::ReferencePtr& __reference() const { return _reference; }

    ::Ice::Int __hash() const;

    void __copyFrom(const ::Ice::ObjectPrx&);

    int __handleException(const ::Ice::Exception&, const ::IceInternal::RequestHandlerPtr&, ::Ice::OperationMode,
                          bool, int&);

    void __checkTwowayOnly(const ::std::string&) const;
    void __checkAsyncTwowayOnly(const ::std::string&) const;

    void __invoke(::IceInternal::Outgoing&) const;
    void __end(const ::Ice::AsyncResultPtr&, const std::string&) const;

    ::IceInternal::RequestHandlerPtr __getRequestHandler();
    ::IceInternal::BatchRequestQueuePtr __getBatchRequestQueue();
    ::IceInternal::RequestHandlerPtr __setRequestHandler(const ::IceInternal::RequestHandlerPtr&);
    void __updateRequestHandler(const ::IceInternal::RequestHandlerPtr&, const ::IceInternal::RequestHandlerPtr&);

protected:

    virtual Object* __newInstance() const;

private:

    bool ice_isA(const ::std::string&, const ::Ice::Context*);
    ::Ice::AsyncResultPtr begin_ice_isA(const ::std::string&,
                                        const ::Ice::Context*,
                                        const ::IceInternal::CallbackBasePtr&,
                                        const ::Ice::LocalObjectPtr&);

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr __begin_ice_isA(
        const ::std::string&,
        const ::Ice::Context*,
        const ::IceInternal::Function<void (bool)>&,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& =
           ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& = ::IceInternal::Function<void (bool)>());
#endif


    void ice_ping(const ::Ice::Context*);
    ::Ice::AsyncResultPtr begin_ice_ping(const ::Ice::Context*,
                                         const ::IceInternal::CallbackBasePtr&,
                                         const ::Ice::LocalObjectPtr&);

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr __begin_ice_ping(
        const ::Ice::Context* ctx,
        const ::IceInternal::Function<void ()>& response,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception =
        ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return begin_ice_ping(ctx, new ::IceInternal::Cpp11FnOnewayCallbackNC(response, exception, sent), 0);
    }
#endif

    ::std::vector< ::std::string> ice_ids(const ::Ice::Context*);
    ::Ice::AsyncResultPtr begin_ice_ids(const ::Ice::Context*,
                                        const ::IceInternal::CallbackBasePtr&,
                                        const ::Ice::LocalObjectPtr&);

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr __begin_ice_ids(
        const ::Ice::Context*,
        const ::IceInternal::Function<void (const ::std::vector< ::std::string>&)>&,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& =
           ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& =
           ::IceInternal::Function<void (bool)>());
#endif

    ::std::string ice_id(const ::Ice::Context*);
    ::Ice::AsyncResultPtr begin_ice_id(const ::Ice::Context*,
                                       const ::IceInternal::CallbackBasePtr&,
                                       const ::Ice::LocalObjectPtr&);
#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr __begin_ice_id(
        const ::Ice::Context*,
        const ::IceInternal::Function<void (const ::std::string&)>&,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& =
           ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& sent =
           ::IceInternal::Function<void (bool)>());
#endif

    bool ice_invoke(const ::std::string&,
                    ::Ice::OperationMode,
                    const ::std::vector< ::Ice::Byte>&,
                    ::std::vector< ::Ice::Byte>&,
                    const ::Ice::Context*);

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string&,
                                           ::Ice::OperationMode,
                                           const ::std::vector< ::Ice::Byte>&,
                                           const ::Ice::Context*,
                                           const ::IceInternal::CallbackBasePtr&,
                                           const ::Ice::LocalObjectPtr&);

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr __begin_ice_invoke(
        const ::std::string&,
        ::Ice::OperationMode,
        const ::std::vector< ::Ice::Byte>&,
        const ::Ice::Context*,
        const ::IceInternal::Function<void (bool, const ::std::vector< ::Ice::Byte>&)>&,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& =
           ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& = ::IceInternal::Function<void (bool)>());

#endif

    bool ice_invoke(const ::std::string&,
                    ::Ice::OperationMode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                    ::std::vector< ::Ice::Byte>&,
                    const ::Ice::Context*);

    ::Ice::AsyncResultPtr begin_ice_invoke(const ::std::string&,
                                           ::Ice::OperationMode,
                                           const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                                           const ::Ice::Context*,
                                           const ::IceInternal::CallbackBasePtr&,
                                           const ::Ice::LocalObjectPtr&);

#ifdef ICE_CPP11
    ::Ice::AsyncResultPtr __begin_ice_invoke(
        const ::std::string&,
        ::Ice::OperationMode,
        const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
        const ::Ice::Context*,
        const ::IceInternal::Function<void (bool, const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&)>&,
        const ::IceInternal::Function<void (const ::Ice::Exception&)>& =
           ::IceInternal::Function<void (const ::Ice::Exception&)>(),
        const ::IceInternal::Function<void (bool)>& = ::IceInternal::Function<void (bool)>());
#endif

    ::Ice::AsyncResultPtr begin_ice_getConnectionInternal(const ::IceInternal::CallbackBasePtr&,
                                                          const ::Ice::LocalObjectPtr&);

    ::Ice::AsyncResultPtr begin_ice_flushBatchRequestsInternal(const ::IceInternal::CallbackBasePtr&,
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
    ::IceProxy::Ice::Object* l = lhs.__upCast();
    ::IceProxy::Ice::Object* r = rhs.__upCast();
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
    ::IceProxy::Ice::Object* l = lhs.__upCast();
    ::IceProxy::Ice::Object* r = rhs.__upCast();
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
checkedCastImpl(const ::Ice::ObjectPrx& b, const ::Ice::Context* context)
{
    P d = 0;
    if(b.get())
    {
        typedef typename P::element_type T;

        d = dynamic_cast<T*>(b.get());
        if(!d && (context == 0 ?
                  b->ice_isA(T::ice_staticId()) :
                  b->ice_isA(T::ice_staticId(), *context)))
        {
            d = new T;
            d->__copyFrom(b);
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
            d->__copyFrom(b);
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
                                         const ::Ice::Context*);

//
// Specializations for P = ::Ice::ObjectPrx
// We have to use inline functions for broken compilers such as VC7.
//

template<> inline ::Ice::ObjectPrx
checkedCastImpl< ::Ice::ObjectPrx>(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context* context)
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
checkedCastImpl(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context* context)
{
    P d = 0;

    typedef typename P::element_type T;
    ::Ice::ObjectPrx bb = checkedCastImpl(b, f, T::ice_staticId(), context);

    if(bb)
    {
        d = new T;
        d->__copyFrom(bb);
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
        d->__copyFrom(bb);
    }
    return d;
}
}

//
// checkedCast and uncheckedCast functions provided in the global namespace
//

template<typename P, typename Y> inline P
checkedCast(const ::IceInternal::ProxyHandle<Y>& b)
{
    Y* tag = 0;
    Ice::Context* ctx = 0;
    return ::IceInternal::checkedCastHelper<typename P::element_type>(b, tag, ctx);
}

template<typename P, typename Y> inline P
checkedCast(const ::IceInternal::ProxyHandle<Y>& b, const ::Ice::Context& context)
{
    Y* tag = 0;
    return ::IceInternal::checkedCastHelper<typename P::element_type>(b, tag, &context);
}

template<typename P, typename Y> inline P
uncheckedCast(const ::IceInternal::ProxyHandle<Y>& b)
{
    Y* tag = 0;
    return ::IceInternal::uncheckedCastHelper<typename P::element_type>(b, tag);
}

template<typename P> inline P
checkedCast(const ::Ice::ObjectPrx& b, const std::string& f)
{
    Ice::Context* ctx = 0;
    return ::IceInternal::checkedCastImpl<P>(b, f, ctx);
}

template<typename P> inline P
checkedCast(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context& context)
{
    return ::IceInternal::checkedCastImpl<P>(b, f, &context);
}

template<typename P> inline P
uncheckedCast(const ::Ice::ObjectPrx& b, const std::string& f)
{
    return ::IceInternal::uncheckedCastImpl<P>(b, f);
}

namespace IceInternal
{

//
// Base template for operation callbacks.
//
template<class T>
class CallbackNC : virtual public CallbackBase
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
class Callback : virtual public CallbackBase
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
            result->getProxy()->__end(result, result->getOperation());
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
            result->getProxy()->__end(result, result->getOperation());
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        bool __ret;
        try
        {
            __ret = __result->getProxy()->end_ice_isA(__result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(__result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(__ret);
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        bool __ret;
        try
        {
            __ret = __result->getProxy()->end_ice_isA(__result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(__result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(__ret,
                                                                          CT::dynamicCast(__result->getCookie()));
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::std::vector< ::std::string> __ret;
        try
        {
            __ret = __result->getProxy()->end_ice_ids(__result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(__result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(__ret);
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::std::vector< ::std::string> __ret;
        try
        {
            __ret = __result->getProxy()->end_ice_ids(__result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(__result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(__ret,
                                                                          CT::dynamicCast(__result->getCookie()));
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::std::string __ret;
        try
        {
            __ret = __result->getProxy()->end_ice_id(__result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(__result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(__ret);
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::std::string __ret;
        try
        {
            __ret = __result->getProxy()->end_ice_id(__result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(__result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(__ret,
                                                                          CT::dynamicCast(__result->getCookie()));
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        if(_response)
        {
            bool __ok;
            std::vector< ::Ice::Byte> outParams;
            try
            {
                __ok = __result->getProxy()->end_ice_invoke(outParams, __result);
            }
            catch(const ::Ice::Exception& ex)
            {
                ::IceInternal::CallbackNC<T>::exception(__result, ex);
                return;
            }
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(__ok, outParams);
        }
        else
        {
            bool __ok;
            std::pair<const ::Ice::Byte*, const::Ice::Byte*> outParams;
            try
            {
                __ok = __result->getProxy()->___end_ice_invoke(outParams, __result);
            }
            catch(const ::Ice::Exception& ex)
            {
                ::IceInternal::CallbackNC<T>::exception(__result, ex);
                return;
            }
            if(_responseArray)
            {
                (::IceInternal::CallbackNC<T>::_callback.get()->*_responseArray)(__ok, outParams);
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        if(_response)
        {
            bool __ok;
            std::vector< ::Ice::Byte> outParams;
            try
            {
                __ok = __result->getProxy()->end_ice_invoke(outParams, __result);
            }
            catch(const ::Ice::Exception& ex)
            {
                ::IceInternal::Callback<T, CT>::exception(__result, ex);
                return;
            }
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(__ok,
                                                                          outParams,
                                                                          CT::dynamicCast(__result->getCookie()));
        }
        else
        {
            bool __ok;
            std::pair<const ::Ice::Byte*, const::Ice::Byte*> outParams;
            try
            {
                __ok = __result->getProxy()->___end_ice_invoke(outParams, __result);
            }
            catch(const ::Ice::Exception& ex)
            {
                ::IceInternal::Callback<T, CT>::exception(__result, ex);
                return;
            }
            if(_responseArray)
            {
                (::IceInternal::Callback<T, CT>::_callback.get()->*_responseArray)(__ok,
                                                                                   outParams,
                                                                                   CT::dynamicCast(
                                                                                       __result->getCookie()));
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


    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::Ice::ConnectionPtr __ret;
        try
        {
            __ret = __result->getProxy()->end_ice_getConnection(__result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(__result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::CallbackNC<T>::_callback.get()->*_response)(__ret);
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

    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::Ice::ConnectionPtr __ret;
        try
        {
            __ret = __result->getProxy()->end_ice_getConnection(__result);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(__result, ex);
            return;
        }
        if(_response)
        {
            (::IceInternal::Callback<T, CT>::_callback.get()->*_response)(__ret,
                                                                          CT::dynamicCast(__result->getCookie()));
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
