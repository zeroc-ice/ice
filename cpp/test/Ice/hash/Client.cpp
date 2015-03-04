// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Random.h>
#include <Test.h>
#include <TestCommon.h>

#ifdef _MSC_VER
// For 'Ice::Object::ice_getHash': was declared deprecated
#pragma warning( disable : 4996 )
#endif

#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;
using namespace Test;

DEFINE_TEST("client")

int main(int argc, char** argv)
{
    cout << "testing proxy & endpoint hash algorithm collisions... " << flush;
    map<Ice::Int, Ice::ObjectPrx> seenProxy;
    map<Ice::Int, Ice::EndpointPtr> seenEndpoint;
    unsigned int proxyCollisions = 0;
    unsigned int endpointCollisions = 0;
    unsigned int i = 0;
    unsigned int maxCollisions = 10;
    unsigned int maxIterations = 10000;

    Ice::InitializationData id;
    id.properties = Ice::createProperties(argc, argv);
#ifndef ICE_OS_WINRT
    //
    // In Ice for WinRT IceSSL is part of Ice core.
    //
    id.properties->setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL");
#endif
    Ice::CommunicatorPtr communicator = Ice::initialize(id);
    for(i = 0; proxyCollisions < maxCollisions && 
               endpointCollisions < maxCollisions  && 
               i < maxIterations; ++i)
    {
        ostringstream os;
        os << i << ":tcp -p " << IceUtilInternal::random(65536) << " -t 10" << IceUtilInternal::random(1000000)
                << ":udp -p " << IceUtilInternal::random(65536) << " -h " << IceUtilInternal::random(100);
                
        Ice::ObjectPrx obj = communicator->stringToProxy(os.str());
        Ice::EndpointSeq endpoints = obj->ice_getEndpoints();
        if(!seenProxy.insert(make_pair(obj->ice_getHash(), obj)).second)
        {
            ++proxyCollisions;
        }
        test(obj->ice_getHash() == obj->ice_getHash());
        
        for(Ice::EndpointSeq::const_iterator j = endpoints.begin(); j != endpoints.end(); ++j)
        {
            Ice::EndpointPtr endpoint = (*j);
            if(!seenEndpoint.insert(make_pair(endpoint->ice_getHash(), endpoint)).second)
            {
                if(endpoint == seenEndpoint[endpoint->ice_getHash()])
                {
                    continue; // Same object
                }
                ++endpointCollisions;
            }
            test(endpoint->ice_getHash() == endpoint->ice_getHash());
        }
    }
    test(proxyCollisions < maxCollisions);
    test(endpointCollisions < maxCollisions);
    
    //
    // Check the same proxy produce the same hash, even when we recreate the proxy.
    //
    Ice::ObjectPrx prx1 = communicator->stringToProxy("Glacier2/router:tcp -p 10010");
    Ice::ObjectPrx prx2 = communicator->stringToProxy("Glacier2/router:ssl -p 10011");
    Ice::ObjectPrx prx3 = communicator->stringToProxy("Glacier2/router:udp -p 10012");
    Ice::ObjectPrx prx4 = communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010");
    Ice::ObjectPrx prx5 = communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011");
    Ice::ObjectPrx prx6 = communicator->stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012");
    Ice::ObjectPrx prx7 = communicator->stringToProxy("Glacier2/router:tcp -p 10010 -t 10000");
    Ice::ObjectPrx prx8 = communicator->stringToProxy("Glacier2/router:ssl -p 10011 -t 10000");
    Ice::ObjectPrx prx9 = communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000");
    Ice::ObjectPrx prx10 = communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000");

    map<string, int> proxyMap;
    proxyMap["prx1"] = prx1->ice_getHash();
    proxyMap["prx2"] = prx2->ice_getHash();
    proxyMap["prx3"] = prx3->ice_getHash();
    proxyMap["prx4"] = prx4->ice_getHash();
    proxyMap["prx5"] = prx5->ice_getHash();
    proxyMap["prx6"] = prx6->ice_getHash();
    proxyMap["prx7"] = prx7->ice_getHash();
    proxyMap["prx8"] = prx8->ice_getHash();
    proxyMap["prx9"] = prx9->ice_getHash();
    proxyMap["prx10"] = prx10->ice_getHash();

    test( communicator->stringToProxy("Glacier2/router:tcp -p 10010")->ice_getHash() == proxyMap["prx1"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -p 10011")->ice_getHash() == proxyMap["prx2"]);
    test( communicator->stringToProxy("Glacier2/router:udp -p 10012")->ice_getHash() == proxyMap["prx3"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010")->ice_getHash() == proxyMap["prx4"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011")->ice_getHash() == proxyMap["prx5"]);
    test( communicator->stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012")->ice_getHash() == proxyMap["prx6"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -p 10010 -t 10000")->ice_getHash() == proxyMap["prx7"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -p 10011 -t 10000")->ice_getHash() == proxyMap["prx8"]);
    test( communicator->stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000")->ice_getHash() == proxyMap["prx9"]);
    test( communicator->stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000")->ice_getHash() == proxyMap["prx10"]);
    
    cerr << "ok" << endl;
    
    cout << "testing objects hash algorithm collisions... " << flush;
    unsigned int objectCollisions = 0;
    map<Ice::Int, Ice::ObjectPtr> seenObject;
    for(i = 0; objectCollisions < maxCollisions && i < maxIterations; ++i)
    {
        Ice::ObjectPtr obj = new Point;
        if(!seenObject.insert(make_pair(obj->ice_getHash(), obj)).second)
        {
            ++objectCollisions;
        }
        test(obj->ice_getHash() == obj->ice_getHash());
    }
    test(objectCollisions < maxCollisions);

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
