//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Ice/RegisterPlugins.h>
#include <IceUtil/Time.h>
#include "ice.h"
#include "Util.h"

using namespace std;
using namespace IceMatlab;

namespace
{

class Init
{
public:

    Init()
    {
        Ice::registerIceWS(true);
        Ice::registerIceSSL(false);
        Ice::registerIceDiscovery(false);
        Ice::registerIceLocatorDiscovery(false);
    }
};

Init init;
}

extern "C"
{

mxArray*
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
            id.properties = deref<Ice::Properties>(propsImpl);
        }

        *r = new shared_ptr<Ice::Communicator>(Ice::initialize(a, id));
        return createResultValue(createStringList(a));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_stringToIdentity(mxArray* s)
{
    try
    {
        return createResultValue(createIdentity(Ice::stringToIdentity(getStringFromUTF16(s))));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
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
}

mxArray*
Ice_stringVersion()
{
    return createResultValue(createStringFromUTF8(ICE_STRING_VERSION));
}

mxArray*
Ice_intVersion(int* v)
{
    *v = ICE_INT_VERSION;
    return 0;
}

mxArray*
Ice_currentEncoding()
{
    return createResultValue(createEncodingVersion(Ice::currentEncoding));
}

mxArray*
Ice_currentProtocol()
{
    return createResultValue(createProtocolVersion(Ice::currentProtocol));
}

mxArray*
Ice_currentProtocolEncoding()
{
    return createResultValue(createEncodingVersion(Ice::currentProtocolEncoding));
}

//
// This function exists so that mex may be used to compile the library. It is not otherwise needed.
//
void
mexFunction(int /*nlhs*/, mxArray * /*plhs*/[], int /*nrhs*/, const mxArray* /*prhs*/[])
{
}

}
