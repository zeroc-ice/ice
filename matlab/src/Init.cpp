// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceDiscovery/IceDiscovery.h"
#include "IceLocatorDiscovery/IceLocatorDiscovery.h"
#include "Util.h"
#include "ice.h"

using namespace std;
using namespace IceMatlab;

extern "C"
{
    mxArray* Ice_initialize(mxArray* args, void* propsImpl, void** r)
    {
        try
        {
            Ice::StringSeq a;
            getStringList(args, a);

            // We add a first argument of 'matlab-client'. It will become the ProgramName unless Ice.ProgramName is set
            // explicitly.
            a.insert(a.begin(), "matlab-client");

            // Create the C++ InitializationData object.
            Ice::InitializationData initData;

            if (propsImpl)
            {
                initData.properties = deref<Ice::Properties>(propsImpl);
            }

            if (!initData.properties)
            {
                initData.properties = Ice::createProperties();
            }

            // Always accept cycles in MATLAB.
            initData.properties->setProperty("Ice.AcceptClassCycles", "1");

            // Add IceDiscovery/IceLocatorDiscovery if these plug-ins are configured via Ice.Plugin.name.
            if (!initData.properties->getIceProperty("Ice.Plugin.IceDiscovery").empty())
            {
                initData.pluginFactories.push_back(IceDiscovery::discoveryPluginFactory());
            }

            if (!initData.properties->getIceProperty("Ice.Plugin.IceLocatorDiscovery").empty())
            {
                initData.pluginFactories.push_back(IceLocatorDiscovery::locatorDiscoveryPluginFactory());
            }

            *r = new shared_ptr<Ice::Communicator>(Ice::initialize(a, initData));

            // Remove the first argument, 'matlab-client'.
            a.erase(a.begin());

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

    //
    // This function exists so that mex may be used to compile the library. It is not otherwise needed.
    //
    void mexFunction(int /*nlhs*/, mxArray* /*plhs*/[], int /*nrhs*/, const mxArray* /*prhs*/[]) {}
}
