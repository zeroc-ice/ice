// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Single.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();

    StringSeq args = argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("IceStorm", args);
    stringSeqToArgs(args, argc, argv);

    const char* managerReferenceProperty = "IceStorm.TopicManager";
    string managerReference = properties->getProperty(managerReferenceProperty);
    if(managerReference.empty())
    {
	cerr << argv[0] << ": property `" << managerReferenceProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator->stringToProxy(managerReference);
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(base);
    if(!manager)
    {
	cerr << argv[0] << ": `" << managerReference << "' is not running" << endl;
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
    // a Single object
    //
    ObjectPrx obj = topic->getPublisher();
    obj = obj->ice_oneway();
    SinglePrx single = SinglePrx::uncheckedCast(obj);

    for(int i = 0; i < 10; ++i)
    {
	single->event();
    }

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;

    try
    {
	communicator = initialize(argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
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
	catch(const Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
