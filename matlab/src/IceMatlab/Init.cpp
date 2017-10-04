// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include <Ice/Ice.h>
#include <Ice/RegisterPlugins.h>
#include <IceUtil/Time.h>
#include "icematlab.h"
#include "Util.h"

using namespace std;
using namespace IceMatlab;

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_initialize(mxArray* args, void* propsImpl, void** r)
{
    try
    {
        Ice::StringSeq a;
        getStringList(args, a);

        //
        // Collect InitializationData members.
        //
        Ice::InitializationData id;

        //
        // Properties
        //
        if(propsImpl)
        {
            id.properties = *(reinterpret_cast<shared_ptr<Ice::Properties>*>(propsImpl));
        }

        shared_ptr<Ice::Communicator> c = Ice::initialize(a, id);
        *r = new shared_ptr<Ice::Communicator>(move(c));
        return createResultValue(createStringList(a));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_stringToIdentity(mxArray* s)
{
    try
    {
        Ice::Identity id = Ice::stringToIdentity(getStringFromUTF16(s));
        return createResultValue(createIdentity(id));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_identityToString(mxArray* id, mxArray* mode)
{
    try
    {
        Ice::ToStringMode m = static_cast<Ice::ToStringMode>(getEnumerator(mode, "Ice.ToStringMode"));
        Ice::Identity ident;
        getIdentity(id, ident);
        return createResultValue(createStringFromUTF8(Ice::identityToString(ident, m)));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_stringVersion()
{
    return createResultValue(createStringFromUTF8(ICE_STRING_VERSION));
}

EXPORTED_FUNCTION mxArray*
Ice_intVersion(int* v)
{
    *v = ICE_INT_VERSION;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_currentEncoding()
{
    return createResultValue(createEncodingVersion(Ice::currentEncoding));
}

EXPORTED_FUNCTION mxArray*
Ice_currentProtocol()
{
    return createResultValue(createProtocolVersion(Ice::currentProtocol));
}

EXPORTED_FUNCTION mxArray*
Ice_currentProtocolEncoding()
{
    return createResultValue(createEncodingVersion(Ice::currentProtocolEncoding));
}

//
// This function exists so that mex may be used to compile the library. It is not otherwise needed.
//
void
mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
}

}
