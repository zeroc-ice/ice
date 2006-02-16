// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Latency.h>

#include <iostream>

using namespace std;
using namespace Demo;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    bool oneway = false;
    bool batch = false;
    bool twoway = false;
    int i;
    for(i = 0; i < argc; i++)
    {
	if(strcmp(argv[i], "oneway") == 0)
	{
	    oneway = true;
	}
	else if(strcmp(argv[i], "batch") == 0)
	{
	    batch = true;
	}
	else if(strcmp(argv[i], "twoway") == 0)
	{
	    twoway = true;
	}
    }
    if(!oneway && !twoway && !batch)
    {
	twoway = true;
    }

    int repetitions = 0;
    if(twoway)
    {
	repetitions = 100000;
    }
    else if(oneway)
    {
	repetitions = 500000;
    }
    else if(batch)
    {
	repetitions = 1000000;
    }

    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Latency.Latency";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(proxy);
    LatencyPrx latency = LatencyPrx::uncheckedCast(base);
    if(!latency)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    LatencyPrx onewayprx = LatencyPrx::uncheckedCast(base->ice_oneway());
    LatencyPrx batchprx = LatencyPrx::uncheckedCast(base->ice_batchOneway());

    // Initial ping to setup the connection.
    latency->ping();

    IceUtil::Time tm = IceUtil::Time::now();

    for(i = 0; i < repetitions; ++i)
    {
        if(batch)
	{
            if(i != 0 && i % 100 == 0)
	    {
	       batchprx->ice_connection()->flushBatchRequests();
	    }
	}

	if(twoway)
	{
	    latency->ping();
	}
	else if(oneway)
	{
	    onewayprx->ping();
	}
	else if(batch)
	{
	    batchprx->ping();
	}
    }

    if(oneway || batch)
    {
        if(batch)
	{
            batchprx->ice_connection()->flushBatchRequests();
        }
        latency->ping();
    }

    tm = IceUtil::Time::now() - tm;

    latency->shutdown();

    cout << tm.toMilliSecondsDouble() / repetitions << endl;
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createProperties();
        properties->load("config");
	communicator = Ice::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex.ice_name() << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex.ice_name() << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
