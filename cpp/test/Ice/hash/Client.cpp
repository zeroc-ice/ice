// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Random.h>
#include <Test.h>
#include <TestCommon.h>

#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;
using namespace Test;

DEFINE_TEST("client")

int main(int argc, char** argv)
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif
    cout << "testing proxy hash algorithm collisions... " << flush;
    map<Ice::Int, Ice::ObjectPrxPtr> seenProxy;
    map<Ice::Int, Ice::EndpointPtr> seenEndpoint;
    unsigned int proxyCollisions = 0;
    unsigned int i = 0;
    unsigned int maxCollisions = 10;
    unsigned int maxIterations = 10000;

    Ice::InitializationData id;
    id.properties = Ice::createProperties(argc, argv);
#if !defined(ICE_OS_WINRT) && TARGET_OS_IPHONE==0
    //
    // In Ice for WinRT IceSSL is part of Ice core.
    //
    id.properties->setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL");
    id.properties->setProperty("IceSSL.Keychain", "client.keychain");
    id.properties->setProperty("IceSSL.KeychainPassword", "password");
#endif
    Ice::CommunicatorPtr communicator = Ice::initialize(id);
    for(i = 0; proxyCollisions < maxCollisions && i < maxIterations; ++i)
    {
        ostringstream os;
        os << i << ":tcp -p " << IceUtilInternal::random(65536) << " -t 10" << IceUtilInternal::random(1000000)
                << ":udp -p " << IceUtilInternal::random(65536) << " -h " << IceUtilInternal::random(100);

        Ice::ObjectPrxPtr obj = communicator->stringToProxy(os.str());
        Ice::EndpointSeq endpoints = obj->ice_getEndpoints();
        if(!seenProxy.insert(make_pair(obj->iceHash(), obj)).second)
        {
            ++proxyCollisions;
        }
        test(obj->iceHash() == obj->iceHash());
    }
    test(proxyCollisions < maxCollisions);

    //
    // Check the same proxy produce the same hash, even when we recreate the proxy.
    //
    Ice::ObjectPrxPtr prx1 = communicator->stringToProxy("Glacier2/router:tcp -p 10010");
    Ice::ObjectPrxPtr prx2 = communicator->stringToProxy("Glacier2/router:ssl -p 10011");
    Ice::ObjectPrxPtr prx3 = communicator->stringToProxy("Glacier2/router:udp -p 10012");
    Ice::ObjectPrxPtr prx4 = communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010");
    Ice::ObjectPrxPtr prx5 = communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011");
    Ice::ObjectPrxPtr prx6 = communicator->stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012");
    Ice::ObjectPrxPtr prx7 = communicator->stringToProxy("Glacier2/router:tcp -p 10010 -t 10000");
    Ice::ObjectPrxPtr prx8 = communicator->stringToProxy("Glacier2/router:ssl -p 10011 -t 10000");
    Ice::ObjectPrxPtr prx9 = communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000");
    Ice::ObjectPrxPtr prx10 = communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000");

    map<string, int> proxyMap;
    proxyMap["prx1"] = prx1->iceHash();
    proxyMap["prx2"] = prx2->iceHash();
    proxyMap["prx3"] = prx3->iceHash();
    proxyMap["prx4"] = prx4->iceHash();
    proxyMap["prx5"] = prx5->iceHash();
    proxyMap["prx6"] = prx6->iceHash();
    proxyMap["prx7"] = prx7->iceHash();
    proxyMap["prx8"] = prx8->iceHash();
    proxyMap["prx9"] = prx9->iceHash();
    proxyMap["prx10"] = prx10->iceHash();

    test( communicator->stringToProxy("Glacier2/router:tcp -p 10010")->iceHash() == proxyMap["prx1"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -p 10011")->iceHash() == proxyMap["prx2"]);
    test( communicator->stringToProxy("Glacier2/router:udp -p 10012")->iceHash() == proxyMap["prx3"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010")->iceHash() == proxyMap["prx4"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011")->iceHash() == proxyMap["prx5"]);
    test( communicator->stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012")->iceHash() == proxyMap["prx6"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -p 10010 -t 10000")->iceHash() == proxyMap["prx7"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -p 10011 -t 10000")->iceHash() == proxyMap["prx8"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000")->iceHash() == proxyMap["prx9"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000")->iceHash() == proxyMap["prx10"]);

    cerr << "ok" << endl;

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
        }
    }
    return EXIT_SUCCESS;
}
