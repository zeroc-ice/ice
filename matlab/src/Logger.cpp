// Copyright (c) ZeroC, Inc.

#include "Ice/Logger.h"
#include "Util.h"
#include "ice.h"

using namespace std;
using namespace IceMatlab;

extern "C"
{
    mxArray* Ice_Logger_unref(void* self)
    {
        delete reinterpret_cast<shared_ptr<Ice::Logger>*>(self);
        return 0;
    }

    mxArray* Ice_Logger_print(void* self, mxArray* message)
    {
        try
        {
            deref<Ice::Logger>(self)->print(getStringFromUTF16(message));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_Logger_trace(void* self, mxArray* category, mxArray* message)
    {
        try
        {
            deref<Ice::Logger>(self)->trace(getStringFromUTF16(category), getStringFromUTF16(message));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_Logger_warning(void* self, mxArray* message)
    {
        try
        {
            deref<Ice::Logger>(self)->warning(getStringFromUTF16(message));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_Logger_error(void* self, mxArray* message)
    {
        try
        {
            deref<Ice::Logger>(self)->error(getStringFromUTF16(message));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_Logger_getPrefix(void* self)
    {
        try
        {
            return createResultValue(createStringFromUTF8(deref<Ice::Logger>(self)->getPrefix()));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_Logger_cloneWithPrefix(void* self, mxArray* prefix, void** r)
    {
        try
        {
            auto logger = deref<Ice::Logger>(self);
            auto newLogger = logger->cloneWithPrefix(getStringFromUTF16(prefix));
            *r = newLogger == logger ? 0 : new shared_ptr<Ice::Logger>(move(newLogger));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }
}
