// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include "icematlab.h"
#include "Logger.h"
#include "Util.h"

#define DEREF(x) (*(reinterpret_cast<shared_ptr<Ice::Logger>*>(x)))
#define SELF DEREF(self)

using namespace std;
using namespace IceMatlab;

void*
IceMatlab::createLogger(shared_ptr<Ice::Logger> p)
{
    return new shared_ptr<Ice::Logger>(p);
}

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_Logger__release(void* self)
{
    delete &SELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Logger_print(void* self, mxArray* message)
{
    try
    {
        SELF->print(getStringFromUTF16(message));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Logger_trace(void* self, mxArray* category, mxArray* message)
{
    try
    {
        SELF->trace(getStringFromUTF16(category), getStringFromUTF16(message));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Logger_warning(void* self, mxArray* message)
{
    try
    {
        SELF->warning(getStringFromUTF16(message));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Logger_error(void* self, mxArray* message)
{
    try
    {
        SELF->error(getStringFromUTF16(message));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Logger_getPrefix(void* self)
{
    try
    {
        return createResultValue(createStringFromUTF8(SELF->getPrefix()));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Logger_cloneWithPrefix(void* self, mxArray* prefix, void** newLogger)
{
    try
    {
        shared_ptr<Ice::Logger> l = SELF->cloneWithPrefix(getStringFromUTF16(prefix));
        *newLogger = l.get() == SELF.get() ? 0 : new shared_ptr<Ice::Logger>(l);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

}
