// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "ice.h"
#include <Ice/ImplicitContext.h>
#include "Util.h"

using namespace std;
using namespace IceMatlab;

extern "C"
{

mxArray*
Ice_ImplicitContext_unref(void* self)
{
    delete reinterpret_cast<shared_ptr<Ice::ImplicitContext>*>(self);
    return 0;
}

mxArray*
Ice_ImplicitContext_getContext(void* self)
{
    try
    {
        return createResultValue(createStringMap(deref<Ice::ImplicitContext>(self)->getContext()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_ImplicitContext_setContext(void* self, mxArray* newContext)
{
    try
    {
        map<string, string> ctx;
        getStringMap(newContext, ctx);
        deref<Ice::ImplicitContext>(self)->setContext(ctx);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ImplicitContext_containsKey(void* self, mxArray* key)
{
    try
    {
        return createResultValue(createBool(deref<Ice::ImplicitContext>(self)->containsKey(getStringFromUTF16(key))));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_ImplicitContext_get(void* self, mxArray* key)
{
    try
    {
        return createResultValue(createStringFromUTF8(deref<Ice::ImplicitContext>(self)->get(getStringFromUTF16(key))));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_ImplicitContext_put(void* self, mxArray* key, mxArray* value)
{
    try
    {
        string k = getStringFromUTF16(key);
        string v = getStringFromUTF16(value);
        return createResultValue(createStringFromUTF8(deref<Ice::ImplicitContext>(self)->put(k, v)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_ImplicitContext_remove(void* self, mxArray* key)
{
    try
    {
        string k = getStringFromUTF16(key);
        return createResultValue(createStringFromUTF8(deref<Ice::ImplicitContext>(self)->remove(k)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

}
