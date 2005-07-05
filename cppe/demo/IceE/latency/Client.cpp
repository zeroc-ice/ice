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

using namespace std;
using namespace Demo;

int
run(int argc, char* argv[], const IceE::CommunicatorPtr& communicator)
{
    IceE::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Latency.Ping";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	fprintf(stderr, "%s: property `%s' not set\n", argv[0], proxyProperty);
	return EXIT_FAILURE;
    }

    IceE::ObjectPrx base = communicator->stringToProxy(proxy);
    PingPrx ping = PingPrx::checkedCast(base);
    if(!ping)
    {
	fprintf(stderr, "%s: invalid proxy\n", argv[0]);
	return EXIT_FAILURE;
    }

    // Initial ping to setup the connection.
    ping->ice_ping();

    IceE::Time tm = IceE::Time::now();

    const int repetitions = 100000;
    printf("pinging server %d times (this may take a while)\n", repetitions);
    for(int i = 0; i < repetitions; ++i)
    {
	ping->ice_ping();
    }

    tm = IceE::Time::now() - tm;

    printf("time for %d pings: %fms\n", repetitions, tm.toMicroSeconds() / 1000.0);
    printf("time per ping: %fms\n", (tm / repetitions).toMicroSeconds() / 1000.0);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    IceE::CommunicatorPtr communicator;

    try
    {
	IceE::PropertiesPtr properties = IceE::createProperties();
        properties->load("config");
	communicator = IceE::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const IceE::Exception& ex)
    {
	fprintf(stderr, "%s\n", ex.toString().c_str());
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const IceE::Exception& ex)
	{
	    fprintf(stderr, "%s\n", ex.toString().c_str());
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
