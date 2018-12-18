// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Properties.h>
#include <Ice/Initialize.h>
#include "ice.h"
#include "Util.h"

using namespace std;
using namespace IceMatlab;

extern "C"
{

//
// Implementation note: We need to pass the implementation object from MATLAB code for the defaults argument.
// If we passed the wrapper object instead, there is no way to extract the void pointer from the mxArray object.
//
mxArray*
Ice_createProperties(mxArray* args, void* defaultsImpl, void** r)
{
    try
    {
        vector<string> a;
        getStringList(args, a);
        shared_ptr<Ice::Properties> def;
        if(defaultsImpl)
        {
            def = deref<Ice::Properties>(defaultsImpl);
        }
        auto props = Ice::createProperties(a, def);
        *r = new shared_ptr<Ice::Properties>(move(props));
        return createResultValue(createStringList(a));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_unref(void* self)
{
    delete reinterpret_cast<shared_ptr<Ice::Properties>*>(self);
    return 0;
}

mxArray*
Ice_Properties_getProperty(void* self, const char* key)
{
    try
    {
        return createResultValue(createStringFromUTF8(deref<Ice::Properties>(self)->getProperty(key)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_getPropertyWithDefault(void* self, const char* key, const char* dflt)
{
    try
    {
        return createResultValue(createStringFromUTF8(deref<Ice::Properties>(self)->getPropertyWithDefault(key, dflt)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_getPropertyAsInt(void* self, const char* key, int* r)
{
    try
    {
        *r = deref<Ice::Properties>(self)->getPropertyAsInt(key);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Properties_getPropertyAsIntWithDefault(void* self, const char* key, int dflt, int* r)
{
    try
    {
        *r = deref<Ice::Properties>(self)->getPropertyAsIntWithDefault(key, dflt);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Properties_getPropertyAsList(void* self, const char* key)
{
    try
    {
        auto l = deref<Ice::Properties>(self)->getPropertyAsList(key);
        return createResultValue(createStringList(l));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_getPropertyAsListWithDefault(void* self, const char* key, mxArray* dflt)
{
    try
    {
        Ice::StringSeq d;
        getStringList(dflt, d);
        Ice::StringSeq l = deref<Ice::Properties>(self)->getPropertyAsListWithDefault(key, d);
        return createResultValue(createStringList(l));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_getPropertiesForPrefix(void* self, const char* prefix)
{
    try
    {
        auto d = deref<Ice::Properties>(self)->getPropertiesForPrefix(prefix);
        return createResultValue(createStringMap(d));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_setProperty(void* self, const char* key, const char* value)
{
    try
    {
        deref<Ice::Properties>(self)->setProperty(key, value);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Properties_getCommandLineOptions(void* self)
{
    try
    {
        auto opts = deref<Ice::Properties>(self)->getCommandLineOptions();
        return createResultValue(createStringList(opts));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_parseCommandLineOptions(void* self, const char* prefix, mxArray* options)
{
    try
    {
        Ice::StringSeq opts;
        getStringList(options, opts);
        Ice::StringSeq rem = deref<Ice::Properties>(self)->parseCommandLineOptions(prefix, opts);
        return createResultValue(createStringList(rem));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_parseIceCommandLineOptions(void* self, mxArray* options)
{
    try
    {
        Ice::StringSeq opts;
        getStringList(options, opts);
        Ice::StringSeq rem = deref<Ice::Properties>(self)->parseIceCommandLineOptions(opts);
        return createResultValue(createStringList(rem));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_Properties_load(void* self, const char* file)
{
    try
    {
        deref<Ice::Properties>(self)->load(file);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_Properties_clone(void* self, void** r)
{
    try
    {
        *r = new shared_ptr<Ice::Properties>(deref<Ice::Properties>(self)->clone());
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

}
