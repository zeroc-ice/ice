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
#include <IceStorm/IceStorm.h>
#include <Single.h>

using namespace Ice;
using namespace IceStorm;
using namespace std;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();

    const char* managerEndpointsProperty = "IceStorm.TopicManager.Endpoints";
    string managerEndpoints = properties->getProperty(managerEndpointsProperty);
    if (managerEndpoints.empty())
    {
	cerr << argv[0] << ": " << managerEndpointsProperty << " is not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator->stringToProxy("TopicManager:" + managerEndpoints);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if (!manager)
    {
	cerr << argv[0] << ": `" << managerEndpoints << "' is not running" << endl;
	return EXIT_FAILURE;
    }

    TopicPrx topic;
    try
    {
	topic = manager->retrieve("single");
    }
    catch(const NoSuchTopic& e)
    {
	cerr << argv[0] << ": NoSuchTopic: " << e.name << endl;
	return EXIT_FAILURE;
	
    }
    assert(topic);

    //
    // Get a publisher object, create a oneway proxy and then cast to
    // a Clock object
    //
    ObjectPrx obj = topic->getPublisher();
    obj = obj->ice_oneway();
    SinglePrx single = SinglePrx::uncheckedCast(obj);

    for (int i = 0; i < 10; ++i)
	single->event();

    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	addArgumentPrefix("IceStorm");
	communicator = Ice::initialize(argc, argv);
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
