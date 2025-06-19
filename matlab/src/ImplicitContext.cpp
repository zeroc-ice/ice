// Copyright (c) ZeroC, Inc.

#include "Ice/ImplicitContext.h"
#include "Util.h"
#include "ice.h"

using namespace std;
using namespace IceMatlab;

extern "C"
{
    mxArray* Ice_ImplicitContext_unref(void* self)
    {
        delete reinterpret_cast<shared_ptr<Ice::ImplicitContext>*>(self);
        return 0;
    }

    mxArray* Ice_ImplicitContext_getContext(void* self)
    {
        try
        {
            return createResultValue(createContext(deref<Ice::ImplicitContext>(self)->getContext()));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_ImplicitContext_setContext(void* self, mxArray* newContext)
    {
        // Caller makes sure it's a configured dictionary string -> string.
        assert(!mxIsEmpty(newContext));
        try
        {
            Ice::Context ctx;
            getContext(newContext, ctx);
            deref<Ice::ImplicitContext>(self)->setContext(ctx);
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ImplicitContext_containsKey(void* self, mxArray* key)
    {
        try
        {
            return createResultValue(
                createBool(deref<Ice::ImplicitContext>(self)->containsKey(getStringFromUTF16(key))));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_ImplicitContext_get(void* self, mxArray* key)
    {
        try
        {
            return createResultValue(
                createStringFromUTF8(deref<Ice::ImplicitContext>(self)->get(getStringFromUTF16(key))));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_ImplicitContext_put(void* self, mxArray* key, mxArray* value)
    {
        try
        {
            string k = getStringFromUTF16(key);
            string v = getStringFromUTF16(value);
            return createResultValue(createStringFromUTF8(deref<Ice::ImplicitContext>(self)->put(k, v)));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_ImplicitContext_remove(void* self, mxArray* key)
    {
        try
        {
            string k = getStringFromUTF16(key);
            return createResultValue(createStringFromUTF8(deref<Ice::ImplicitContext>(self)->remove(k)));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }
}
