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
#include <Ice/OutputStream.h>
#include "icematlab.h"
#include "ObjectPrx.h"
#include "Util.h"

#define SELF (*(reinterpret_cast<shared_ptr<Ice::Communicator>*>(self)))

using namespace std;
using namespace IceMatlab;

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_Communicator__release(void* self)
{
    delete &SELF;
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
Ice_Communicator_stringToProxy(void* self, const char* s, void** proxy)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> p = SELF->stringToProxy(s);
        if(p)
        {
            *proxy = new shared_ptr<Ice::ObjectPrx>(p);
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
Ice_Communicator_propertyToProxy(void* self, const char* prop, void** proxy)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> p = SELF->propertyToProxy(prop);
        if(p)
        {
            *proxy = new shared_ptr<Ice::ObjectPrx>(p);
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
        shared_ptr<Ice::ObjectPrx> p = getProxy(proxy);
        Ice::PropertyDict d = SELF->proxyToProperty(p, prop);
        return createResultValue(createStringMap(d));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_proxyToString(void* self, void* proxy)
{
    assert(proxy);
    try
    {
        shared_ptr<Ice::ObjectPrx> p = getProxy(proxy);
        return createResultValue(createStringFromUTF8(SELF->proxyToString(p)));
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
        shared_ptr<Ice::Properties> p = SELF->getProperties();
        *props = new shared_ptr<Ice::Properties>(p);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Communicator_createOutputStream(void* self, mxArray* encoding, void** stream)
{
    Ice::EncodingVersion v;
    getEncodingVersion(encoding, v);
    *stream = new Ice::OutputStream(SELF, v);
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

}
