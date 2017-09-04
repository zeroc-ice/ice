// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include <Ice/Properties.h>
#include <Ice/Initialize.h>
#include "icematlab.h"
#include "Util.h"

#define SELF (*(reinterpret_cast<shared_ptr<Ice::Properties>*>(self)))

using namespace std;
using namespace IceMatlab;

extern "C"
{

//
// Implementation note: We need to pass the implementation object from MATLAB code for the defaults argument.
// If we passed the wrapper object instead, there is no way to extract the void pointer from the mxArray object.
//
EXPORTED_FUNCTION mxArray*
Ice_createProperties(mxArray* args, void* defaultsImpl, void** r)
{
    try
    {
        vector<string> a;
        getStringList(args, a);
        shared_ptr<Ice::Properties> def;
        if(defaultsImpl)
        {
            def = *(reinterpret_cast<shared_ptr<Ice::Properties>*>(defaultsImpl));
        }
        shared_ptr<Ice::Properties> props = Ice::createProperties(a, def);
        *r = new shared_ptr<Ice::Properties>(props);
        return createResultValue(createStringList(a));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties__release(void* self)
{
    delete &SELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_getProperty(void* self, const char* key)
{
    try
    {
        return createResultValue(createStringFromUTF8(SELF->getProperty(key)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_getPropertyWithDefault(void* self, const char* key, const char* dflt)
{
    try
    {
        return createResultValue(createStringFromUTF8(SELF->getPropertyWithDefault(key, dflt)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_getPropertyAsInt(void* self, const char* key, int* r)
{
    try
    {
        *r = SELF->getPropertyAsInt(key);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_getPropertyAsIntWithDefault(void* self, const char* key, int dflt, int* r)
{
    try
    {
        *r = SELF->getPropertyAsIntWithDefault(key, dflt);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_getPropertyAsList(void* self, const char* key)
{
    try
    {
        Ice::StringSeq l = SELF->getPropertyAsList(key);
        return createResultValue(createStringList(l));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_getPropertyAsListWithDefault(void* self, const char* key, mxArray* dflt)
{
    try
    {
        Ice::StringSeq d;
        getStringList(dflt, d);
        Ice::StringSeq l = SELF->getPropertyAsListWithDefault(key, d);
        return createResultValue(createStringList(l));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_getPropertiesForPrefix(void* self, const char* prefix)
{
    try
    {
        Ice::PropertyDict d = SELF->getPropertiesForPrefix(prefix);
        return createResultValue(createStringMap(d));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_setProperty(void* self, const char* key, const char* value)
{
    try
    {
        SELF->setProperty(key, value);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_getCommandLineOptions(void* self)
{
    try
    {
        Ice::StringSeq opts = SELF->getCommandLineOptions();
        return createResultValue(createStringList(opts));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_parseCommandLineOptions(void* self, const char* prefix, mxArray* options)
{
    try
    {
        Ice::StringSeq opts;
        getStringList(options, opts);
        Ice::StringSeq rem = SELF->parseCommandLineOptions(prefix, opts);
        return createResultValue(createStringList(rem));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_parseIceCommandLineOptions(void* self, mxArray* options)
{
    try
    {
        Ice::StringSeq opts;
        getStringList(options, opts);
        Ice::StringSeq rem = SELF->parseIceCommandLineOptions(opts);
        return createResultValue(createStringList(rem));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_load(void* self, const char* file)
{
    try
    {
        SELF->load(file);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_Properties_clone(void* self, void** r)
{
    try
    {
        shared_ptr<Ice::Properties> c = SELF->clone();
        *r = new shared_ptr<Ice::Properties>(c);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

}
