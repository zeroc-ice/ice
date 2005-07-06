// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <NestedI.h>

using namespace std;
using namespace Ice;
using namespace Demo;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Nested.Client.NestedServer";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	fprintf(stderr, "%s: property `%s' not set\n", argv[0], proxyProperty);
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator->stringToProxy(proxy);
    NestedPrx nested = NestedPrx::checkedCast(base);
    if(!nested)
    {
	fprintf(stderr, "%s: invalid proxy\n", argv[0]);
	return EXIT_FAILURE;
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapter("Nested.Client");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy(Ice::stringToIdentity("nestedClient")));
    adapter->add(new NestedI(self), Ice::stringToIdentity("nestedClient"));
    adapter->activate();

    printf("Note: The maximum nesting level is sz * 2, with sz being\n");
    printf("the maximum number of threads in the server thread pool. if\n");
    printf("you specify a value higher than that, the application will\n");
    printf("block or timeout.\n\n");


    char buf[32];
    char* s;
    do
    {
	try
	{
	    printf("enter nesting level or 'x' for exit: ");
	    s = fgets(buf, 32, stdin);

	    if(s != NULL)
	    {
	        int level = atoi(s);
	        if(level > 0)
	        {
		    nested->nestedCall(level, self);
	        }
	    }
	}
	catch(const Exception& ex)
	{
	    fprintf(stderr, "%s\n", ex.toString().c_str());
	}
    }
    while(s != NULL && s[0] != 'x');

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
        fprintf(stderr, "%s\n", ex.toString().c_str());
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
            fprintf(stderr, "%s\n", ex.toString().c_str());
            status = EXIT_FAILURE;
        }
    }

    return status;
}

