// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Throughput.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Throughput.Throughput";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(proxy);
    ThroughputPrx throughput = ThroughputPrx::checkedCast(base);
    if(!throughput)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    // Initial ping to setup the connection.
    throughput->ice_ping();

    IceUtil::Time tsec = IceUtil::Time::now();

    const int repetitions = 100;

    cout << "sending and receiving " << repetitions << " sequences of size " << seqSize
	 << " (this may take a while)" << endl;
    ByteSeq seq(seqSize, 0);
    for(int i = 0; i < repetitions; ++i)
    {
	throughput->echoByteSeq(seq);
    }

    tsec = IceUtil::Time::now() - tsec;

    double tmsec = tsec * 1000.0L;
    
    cout << "time for " << repetitions << " sequences: " << tmsec  << "ms" << endl;
    cout << "time per sequence: " << tmsec / repetitions << "ms" << endl;

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

    if(communicator)
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
