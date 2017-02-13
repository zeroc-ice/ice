// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Options.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <TestCommon.h>
#include <Single.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace Test;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    IceUtilInternal::Options opts;
    opts.addOpt("", "cycle");

    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        return EXIT_FAILURE;
    }

    PropertiesPtr properties = communicator->getProperties();
    const char* managerProxyProperty = "IceStormAdmin.TopicManager.Default";
    string managerProxy = properties->getProperty(managerProxyProperty);
    if(managerProxy.empty())
    {
        cerr << argv[0] << ": property `" << managerProxyProperty << "' is not set" << endl;
        return EXIT_FAILURE;
    }

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator->stringToProxy(managerProxy));
    if(!manager)
    {
        cerr << argv[0] << ": `" << managerProxy << "' is not running" << endl;
        return EXIT_FAILURE;
    }

    TopicPrx topic;
    while(true)
    {
        try
        {
            topic = manager->retrieve("single");
            break;
        }
        // This can happen if the replica group loses the majority
        // during retrieve. In this case we retry.
        catch(const Ice::UnknownException&)
        {
            continue;
        }
        catch(const IceStorm::NoSuchTopic& e)
        {
            cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
            return EXIT_FAILURE;
        }
    }
    assert(topic);

    //
    // Get a publisher object, create a twoway proxy and then cast to
    // a Single object.
    //
    if(opts.isSet("cycle"))
    {
        Ice::ObjectPrx prx = topic->getPublisher()->ice_twoway();
        vector<SinglePrx> single;
        Ice::EndpointSeq endpoints = prx->ice_getEndpoints();
        for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
        {
            if((*p)->toString().substr(0, 3) != "udp")
            {
                Ice::EndpointSeq e;
                e.push_back(*p);
                single.push_back(SinglePrx::uncheckedCast(prx->ice_endpoints(e)));
            }
        }
        if(single.size() <= 1)
        {
            cerr << argv[0] << ": Not enough endpoints in publisher proxy" << endl;
            return EXIT_FAILURE;
        }
        int which = 0;
        for(int i = 0; i < 1000; ++i)
        {
            single[which]->event(i);
            which = (which + 1) % static_cast<int>(single.size());
        }
    }
    else
    {
        SinglePrx single = SinglePrx::uncheckedCast(topic->getPublisher()->ice_twoway());
        for(int i = 0; i < 1000; ++i)
        {
            single->event(i);
        }
    }

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;
    InitializationData initData = getTestInitData(argc, argv);
    try
    {
        communicator = initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;
}
