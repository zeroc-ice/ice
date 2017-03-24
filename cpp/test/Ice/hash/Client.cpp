// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    Ice::registerIceSSL(false);
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
    id.properties->setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL");
    id.properties->setProperty("IceSSL.Keychain", "client.keychain");
    id.properties->setProperty("IceSSL.KeychainPassword", "password");

    Ice::CommunicatorPtr communicator = Ice::initialize(id);
    for(i = 0; proxyCollisions < maxCollisions && i < maxIterations; ++i)
    {
        ostringstream os;
        os << i << ":tcp -p " << IceUtilInternal::random(65536) << " -t 10" << IceUtilInternal::random(1000000)
                << ":udp -p " << IceUtilInternal::random(65536) << " -h " << IceUtilInternal::random(100);

        Ice::ObjectPrxPtr obj = communicator->stringToProxy(os.str());
        Ice::EndpointSeq endpoints = obj->ice_getEndpoints();
        if(!seenProxy.insert(make_pair(obj->_hash(), obj)).second)
        {
            ++proxyCollisions;
        }
        test(obj->_hash() == obj->_hash());
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
    proxyMap["prx1"] = prx1->_hash();
    proxyMap["prx2"] = prx2->_hash();
    proxyMap["prx3"] = prx3->_hash();
    proxyMap["prx4"] = prx4->_hash();
    proxyMap["prx5"] = prx5->_hash();
    proxyMap["prx6"] = prx6->_hash();
    proxyMap["prx7"] = prx7->_hash();
    proxyMap["prx8"] = prx8->_hash();
    proxyMap["prx9"] = prx9->_hash();
    proxyMap["prx10"] = prx10->_hash();

    test( communicator->stringToProxy("Glacier2/router:tcp -p 10010")->_hash() == proxyMap["prx1"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -p 10011")->_hash() == proxyMap["prx2"]);
    test( communicator->stringToProxy("Glacier2/router:udp -p 10012")->_hash() == proxyMap["prx3"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010")->_hash() == proxyMap["prx4"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011")->_hash() == proxyMap["prx5"]);
    test( communicator->stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012")->_hash() == proxyMap["prx6"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -p 10010 -t 10000")->_hash() == proxyMap["prx7"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -p 10011 -t 10000")->_hash() == proxyMap["prx8"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000")->_hash() == proxyMap["prx9"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000")->_hash() == proxyMap["prx10"]);

    cerr << "ok" << endl;

    if(communicator)
    {
        communicator->destroy();
    }
    return EXIT_SUCCESS;
}
