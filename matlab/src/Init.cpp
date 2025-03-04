// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Ice/RegisterPlugins.h"
#include "Util.h"
#include "ice.h"

// Link with IceDiscovery and IceLocatorDiscovery on Windows
#if defined(_MSC_VER) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("IceDiscovery"))
#    pragma comment(lib, ICE_LIBNAME("IceLocatorDiscovery"))
#endif

using namespace std;
using namespace IceMatlab;

namespace
{
    class Init
    {
    public:
        Init()
        {
            Ice::registerIceDiscovery(false);
            Ice::registerIceLocatorDiscovery(false);
        }
    };

    Init init;
}

extern "C"
{
    mxArray* Ice_initialize(mxArray* args, void* propsImpl, void** r)
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
            if (propsImpl)
            {
                id.properties = deref<Ice::Properties>(propsImpl);
            }

            if (!id.properties)
            {
                id.properties = Ice::createProperties();
            }

            // Always accept cycles in MATLAB
            id.properties->setProperty("Ice.AcceptClassCycles", "1");

            *r = new shared_ptr<Ice::Communicator>(Ice::initialize(a, id));
            return createResultValue(createStringList(a));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_stringToIdentity(mxArray* s)
    {
        try
        {
            return createResultValue(createIdentity(Ice::stringToIdentity(getStringFromUTF16(s))));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_identityToString(mxArray* id, mxArray* mode)
    {
        try
        {
            Ice::ToStringMode m = static_cast<Ice::ToStringMode>(getEnumerator(mode, "Ice.ToStringMode"));
            Ice::Identity ident;
            getIdentity(id, ident);
            return createResultValue(createStringFromUTF8(Ice::identityToString(ident, m)));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_stringVersion() { return createResultValue(createStringFromUTF8(ICE_STRING_VERSION)); }

    mxArray* Ice_intVersion(int* v)
    {
        *v = ICE_INT_VERSION;
        return 0;
    }

    mxArray* Ice_currentEncoding() { return createResultValue(createEncodingVersion(Ice::currentEncoding)); }

    mxArray* Ice_currentProtocol() { return createResultValue(createProtocolVersion(Ice::currentProtocol)); }

    mxArray* Ice_currentProtocolEncoding()
    {
        return createResultValue(createEncodingVersion(Ice::currentProtocolEncoding));
    }

    //
    // This function exists so that mex may be used to compile the library. It is not otherwise needed.
    //
    void mexFunction(int /*nlhs*/, mxArray* /*plhs*/[], int /*nrhs*/, const mxArray* /*prhs*/[]) {}
}
