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
#ifdef WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

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

    timeval tv1;
#ifdef WIN32
    struct _timeb tb1;
    _ftime(&tb1);
    tv1.tv_sec = tb1.time;
    tv1.tv_usec = tb1.millitm * 1000;
#else
    gettimeofday(&tv1, 0);
#endif

    const int repetitions = 100000;
    cout << "pinging server " << repetitions << " times (this may take a while)" << endl;
    for (int i = 0; i < repetitions; ++i)
    {
	ping->ice_ping();
    }

    timeval tv2;
#ifdef WIN32
    struct _timeb tb2;
    _ftime(&tb2);
    tv2.tv_sec = tb2.time;
    tv2.tv_usec = tb2.millitm * 1000;
#else
    gettimeofday(&tv2, 0);
#endif

    timeval tv;

    tv.tv_sec = tv2.tv_sec - tv1.tv_sec;
    tv.tv_usec = tv2.tv_usec - tv1.tv_usec;

    tv.tv_sec += tv.tv_usec / 1000000;
    tv.tv_usec = tv.tv_usec % 1000000;

    if (tv.tv_usec < 0)
    {
        tv.tv_usec += 1000000;
        tv.tv_sec -= 1;
    }

    double total = (tv.tv_sec * 1000000.0 + tv.tv_usec) / 1000.0;
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
	Ice::PropertiesPtr properties = Ice::createPropertiesFromFile(argc, argv, "config");
	communicator = Ice::initializeWithProperties(properties);
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
