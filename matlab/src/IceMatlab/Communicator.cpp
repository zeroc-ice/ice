// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include <Ice/Communicator.h>
#include <Ice/Proxy.h>
#include "icematlab.h"
#include "Communicator.h"
#include "Future.h"
#include "Logger.h"
#include "ObjectPrx.h"
#include "Util.h"

#define DEREF(x) (*(reinterpret_cast<shared_ptr<Ice::Communicator>*>(x)))
#define SELF DEREF(self)

using namespace std;
using namespace IceMatlab;

shared_ptr<Ice::Communicator>
IceMatlab::getCommunicator(void* p)
{
    return DEREF(p);
}

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_Communicator_unref(void* self)
{
    delete &SELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_destroy(void* self)
{
    try
    {
        SELF->destroy();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_destroyAsync(void* self, void** future)
{
    *future = 0;
    auto c = SELF;
    auto f = make_shared<SimpleFuture>();

    thread t([c, f]
        {
            try
            {
                c->destroy();
                f->done();
            }
            catch(const std::exception&)
            {
                f->exception(current_exception());
            }
        });
    t.detach();
    *future = new shared_ptr<SimpleFuture>(move(f));
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_stringToProxy(void* self, const char* s, void** proxy)
{
    try
    {
        auto p = SELF->stringToProxy(s);
        if(p)
        {
            *proxy = createProxy(p);
        }
        else
        {
            *proxy = 0;
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_proxyToString(void* self, void* proxy)
{
    assert(proxy);
    try
    {
        auto p = getProxy(proxy);
        return createResultValue(createStringFromUTF8(SELF->proxyToString(p)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_propertyToProxy(void* self, const char* prop, void** proxy)
{
    try
    {
        auto p = SELF->propertyToProxy(prop);
        if(p)
        {
            *proxy = createProxy(p);
        }
        else
        {
            *proxy = 0;
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_proxyToProperty(void* self, void* proxy, const char* prop)
{
    assert(proxy);
    try
    {
        auto p = getProxy(proxy);
        auto d = SELF->proxyToProperty(p, prop);
        return createResultValue(createStringMap(d));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_identityToString(void* self, mxArray* id)
{
    try
    {
        Ice::Identity ident;
        getIdentity(id, ident);
        return createResultValue(createStringFromUTF8(SELF->identityToString(ident)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_getProperties(void* self, void** props)
{
    try
    {
        auto p = SELF->getProperties();
        *props = new shared_ptr<Ice::Properties>(move(p));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_getLogger(void* self, void** logger)
{
    try
    {
        auto l = SELF->getLogger();
        *logger = createLogger(l);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_getDefaultRouter(void* self, void** proxy)
{
    try
    {
        auto p = SELF->getDefaultRouter();
        if(p)
        {
            *proxy = createProxy(p);
        }
        else
        {
            *proxy = 0;
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_setDefaultRouter(void* self, void* proxy)
{
    try
    {
        shared_ptr<Ice::RouterPrx> p;
        if(proxy)
        {
            p = Ice::uncheckedCast<Ice::RouterPrx>(getProxy(proxy));
        }
        SELF->setDefaultRouter(p);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_getDefaultLocator(void* self, void** proxy)
{
    try
    {
        auto p = SELF->getDefaultLocator();
        if(p)
        {
            *proxy = createProxy(p);
        }
        else
        {
            *proxy = 0;
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_setDefaultLocator(void* self, void* proxy)
{
    try
    {
        shared_ptr<Ice::LocatorPrx> p;
        if(proxy)
        {
            p = Ice::uncheckedCast<Ice::LocatorPrx>(getProxy(proxy));
        }
        SELF->setDefaultLocator(p);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_flushBatchRequests(void* self, mxArray* mode)
{
    try
    {
        auto m = static_cast<Ice::CompressBatch>(getEnumerator(mode, "Ice.CompressBatch"));
        SELF->flushBatchRequests(m);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_flushBatchRequestsAsync(void* self, mxArray* mode, void** future)
{
    *future = 0;
    auto f = make_shared<SimpleFuture>();

    try
    {
        auto m = static_cast<Ice::CompressBatch>(getEnumerator(mode, "Ice.CompressBatch"));
        function<void()> token = SELF->flushBatchRequestsAsync(
            m,
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            [f](bool /*sentSynchronously*/)
            {
                f->done();
            });
        f->token(token);
        *future = new shared_ptr<SimpleFuture>(move(f));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

}
