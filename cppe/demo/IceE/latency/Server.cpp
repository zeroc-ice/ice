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

class PingI : public Ping
{
public:
    // no methods.
};

int
run(int argc, char* argv[], const IceE::CommunicatorPtr& communicator)
{
    IceE::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Latency");
    IceE::ObjectPtr object = new PingI;
    adapter->add(object, IceE::stringToIdentity("ping"));
    adapter->activate();
    communicator->waitForShutdown();
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
