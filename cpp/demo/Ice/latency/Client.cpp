// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Latency.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* refProperty = "Latency.Ping";
    std::string ref = properties->getProperty(refProperty);
    if (ref.empty())
    {
	cerr << argv[0] << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    PingPrx ping = PingPrx::checkedCast(base);
    if (!ping)
    {
	cerr << argv[0] << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    // Initial ping to setup the connection.
    ping->ice_ping();

    IceUtil::Time tm = IceUtil::Time::now();

    const int repetitions = 100000;
    cout << "pinging server " << repetitions << " times (this may take a while)" << endl;
    for (int i = 0; i < repetitions; ++i)
    {
	ping->ice_ping();
    }

    tm = IceUtil::Time::now() - tm;

    double total = tm / 1000.0;
    double perPing = total / repetitions;
    
    cout << "time for " << repetitions << " pings: " << total  << "ms" << endl;
    cout << "time per ping: " << perPing << "ms" << endl;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createProperties(argc, argv);
        properties->load("config");
	communicator = Ice::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
