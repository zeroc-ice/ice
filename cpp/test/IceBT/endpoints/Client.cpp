// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceBT/IceBT.h"
#include "TestHelper.h"

using namespace std;

namespace
{
    // Walk the underlying chain (e.g. bts -> bt) to find the IceBT endpoint info.
    IceBT::EndpointInfoPtr getBTEndpointInfo(const Ice::EndpointInfoPtr& info)
    {
        for (Ice::EndpointInfoPtr p = info; p; p = p->underlying)
        {
            if (auto bt = dynamic_pointer_cast<IceBT::EndpointInfo>(p))
            {
                return bt;
            }
        }
        return nullptr;
    }
}

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);

    // Register the IceBT plug-in but do NOT initialize it. Initializing the plug-in connects to the
    // Bluetooth daemon over the D-Bus system bus, which requires Bluetooth services we don't have on
    // CI. The bt/bts endpoint factories are registered when the plug-in is *constructed* (before
    // initialization), so we can parse endpoints without a live Bluetooth stack.
    //
    // We only add IceBT here: Ice always registers the built-in TCP and SSL plug-in factories itself,
    // so the SSL endpoint factory needed to parse a bts (SSL-over-Bluetooth) endpoint is present
    // without us adding it.
    initData.properties->setProperty("Ice.InitPlugins", "0");
    initData.pluginFactories = {IceBT::btPluginFactory()};

    Ice::CommunicatorHolder holder{initialize(std::move(initData))};
    const Ice::CommunicatorPtr& communicator = holder.communicator();

    const string addr = "01:23:45:67:89:AB";
    const string uuid = "5ec1a9e6-8f7a-4f3a-9b2c-1234567890ab";

    cout << "testing bt endpoint parsing... " << flush;
    {
        // The address is quoted because ':' is the proxy endpoint separator.
        Ice::ObjectPrx prx{communicator, "dummy:bt -a \"" + addr + "\" -u " + uuid};

        Ice::EndpointSeq endpoints = prx->ice_getEndpoints();
        test(endpoints.size() == 1);

        // A bt endpoint's info is directly an IceBT::EndpointInfo (no wrapper).
        IceBT::EndpointInfoPtr info = dynamic_pointer_cast<IceBT::EndpointInfo>(endpoints[0]->getInfo());
        test(info);
        test(info->addr == addr);
        test(info->uuid == uuid);
        test(!info->secure());

        // The stringified proxy must round-trip back to an equivalent proxy.
        Ice::ObjectPrx prx2{communicator, prx->ice_toString()};
        test(prx2 == prx);
    }
    cout << "ok" << endl;

    cout << "testing bts (SSL over Bluetooth) endpoint parsing... " << flush;
    {
        Ice::ObjectPrx prx{communicator, "dummy:bts -a \"" + addr + "\" -u " + uuid};

        Ice::EndpointSeq endpoints = prx->ice_getEndpoints();
        test(endpoints.size() == 1);

        // A bts endpoint layers SSL over Bluetooth: the top-level info is the SSL wrapper, and the
        // IceBT::EndpointInfo is its underlying info.
        Ice::EndpointInfoPtr info = endpoints[0]->getInfo();
        test(!dynamic_pointer_cast<IceBT::EndpointInfo>(info));
        IceBT::EndpointInfoPtr btInfo = getBTEndpointInfo(info);
        test(btInfo);
        test(btInfo->addr == addr);
        test(btInfo->uuid == uuid);

        // The stringified proxy must round-trip back to an equivalent proxy.
        Ice::ObjectPrx prx2{communicator, prx->ice_toString()};
        test(prx2 == prx);
    }
    cout << "ok" << endl;

    cout << "testing malformed bt endpoints are rejected... " << flush;
    {
        // Invalid device address. -u is supplied so the address is what fails: a proxy endpoint
        // without -u is rejected for the missing UUID before the address is ever validated.
        try
        {
            Ice::ObjectPrx bad{communicator, "dummy:bt -a not-an-address -u 8ce255c0-200a-11e0-ac64-0800200c9a66"};
            test(false);
        }
        catch (const Ice::ParseException&)
        {
        }

        // The channel option (-c) is only valid for object adapter endpoints, not proxy endpoints.
        try
        {
            Ice::ObjectPrx bad{communicator, "dummy:bt -a \"" + addr + "\" -u " + uuid + " -c 1"};
            test(false);
        }
        catch (const Ice::ParseException&)
        {
        }
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
