// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/ios/iAPMatch.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

namespace
{
    // A communicator with the iAP endpoint factory registered, so iAP endpoint strings can be parsed.
    Ice::CommunicatorPtr createIAPCommunicator()
    {
        Ice::InitializationData initData;
        initData.pluginFactories = {Ice::iapPluginFactory()};
        return Ice::initialize(initData);
    }

    Ice::EndpointPtr parseEndpoint(const Ice::CommunicatorPtr& communicator, const string& endpoint)
    {
        optional<Ice::ObjectPrx> prx = communicator->stringToProxy("dummy:" + endpoint);
        test(prx);
        Ice::EndpointSeq endpoints = prx->ice_getEndpoints();
        test(endpoints.size() == 1);
        return endpoints[0];
    }

    void testEndpointParsing()
    {
        cout << "testing iAP endpoint parsing... " << flush;
        Ice::CommunicatorHolder ich{createIAPCommunicator()};
        Ice::CommunicatorPtr communicator = ich.communicator();

        {
            // All options round-trip through the string form.
            Ice::EndpointPtr endpoint =
                parseEndpoint(communicator, "iap -p com.zeroc.ice -m Acme -o Model3 -n HeadUnit -t 1500 -z");
            string s = endpoint->toString();
            test(s.find("-p com.zeroc.ice") != string::npos);
            test(s.find("-m Acme") != string::npos);
            test(s.find("-o Model3") != string::npos);
            test(s.find("-n HeadUnit") != string::npos);
            test(s.find("-t 1500") != string::npos);
            test(s.find("-z") != string::npos);

            auto info = dynamic_pointer_cast<Ice::IAPEndpointInfo>(endpoint->getInfo());
            test(info);
            test(info->manufacturer == "Acme");
            test(info->modelNumber == "Model3");
            test(info->name == "HeadUnit");
        }

        {
            // A bare endpoint has no filters set.
            Ice::EndpointPtr endpoint = parseEndpoint(communicator, "iap");
            auto info = dynamic_pointer_cast<Ice::IAPEndpointInfo>(endpoint->getInfo());
            test(info);
            test(info->manufacturer.empty());
            test(info->modelNumber.empty());
            test(info->name.empty());
        }

        {
            // -t infinite is accepted and produces no timeout option in the string form.
            Ice::EndpointPtr endpoint = parseEndpoint(communicator, "iap -t infinite");
            test(endpoint->toString().find("-t ") == string::npos);
        }

        // Invalid endpoints must be rejected with a ParseException.
        const string invalid[] = {
            "iap -t abc", // non-numeric timeout
            "iap -t 0",   // timeout < 1
            "iap -m",     // missing argument
            "iap -o",
            "iap -n",
            "iap -p",
            "iap -z arg", // -z takes no argument
        };
        for (const string& endpoint : invalid)
        {
            try
            {
                communicator->stringToProxy("dummy:" + endpoint);
                test(false);
            }
            catch (const Ice::ParseException&)
            {
                // expected
            }
        }
        cout << "ok" << endl;
    }

    void testAccessoryMatching()
    {
        cout << "testing iAP accessory matching... " << flush;
        using IceObjC::iAPMatches;
        const vector<string> proto{"com.zeroc.ice"};

        // Empty filters match any connected accessory that advertises the protocol.
        test(iAPMatches("", "", "", "com.zeroc.ice", true, "Acme", "Model3", "HeadUnit", proto));
        // A disconnected accessory never matches.
        test(!iAPMatches("", "", "", "com.zeroc.ice", false, "Acme", "Model3", "HeadUnit", proto));
        // The protocol must be advertised by the accessory.
        test(!iAPMatches("", "", "", "com.zeroc.ice", true, "Acme", "Model3", "HeadUnit", {"com.other"}));
        // Manufacturer / modelNumber / name filters must match when set.
        test(iAPMatches("Acme", "", "", "com.zeroc.ice", true, "Acme", "Model3", "HeadUnit", proto));
        test(!iAPMatches("Other", "", "", "com.zeroc.ice", true, "Acme", "Model3", "HeadUnit", proto));
        test(!iAPMatches("", "Model4", "", "com.zeroc.ice", true, "Acme", "Model3", "HeadUnit", proto));
        test(!iAPMatches("", "", "Other", "com.zeroc.ice", true, "Acme", "Model3", "HeadUnit", proto));
        // All filters set and matching.
        test(iAPMatches("Acme", "Model3", "HeadUnit", "com.zeroc.ice", true, "Acme", "Model3", "HeadUnit", proto));
        cout << "ok" << endl;
    }
}

void
allTests(Test::TestHelper* helper)
{
    testEndpointParsing();
    testAccessoryMatching();

    // Defined in Transceiver.mm; exercises the transceiver I/O state machine over a fake stream pair.
    void allTestsTransceiver(const Ice::CommunicatorPtr&);
    allTestsTransceiver(helper->communicator());
}
