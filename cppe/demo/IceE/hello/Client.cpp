// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

void
menu()
{
    printf("usage:\n");
    printf("t: send greeting as twoway\n");
    printf("o: send greeting as oneway\n");
    printf("O: send greeting as batch oneway\n");
    printf("f: flush all batch requests\n");
    printf("T: set a timeout\n");
    printf("s: shutdown server\n");
    printf("x: exit\n");
    printf("?: help\n");
}

int
run(int argc, char* argv[], const IceE::CommunicatorPtr& communicator)
{
    IceE::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Hello.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	fprintf(stderr, "%s: property `%s' not set\n", argv[0], proxyProperty);
	return EXIT_FAILURE;
    }

    IceE::ObjectPrx base = communicator->stringToProxy(proxy);
    HelloPrx twoway = HelloPrx::checkedCast(base->ice_twoway()->ice_timeout(-1));
    if(!twoway)
    {
	fprintf(stderr, "%s: invalid proxy\n", argv[0]);
	return EXIT_FAILURE;
    }
    HelloPrx oneway = HelloPrx::uncheckedCast(twoway->ice_oneway());
    HelloPrx batchOneway = HelloPrx::uncheckedCast(twoway->ice_batchOneway());

    int timeout = -1;

    menu();

    char c;
    do
    {
	try
	{
	    printf("==> ");
	    do
	    {
	        c = getchar();
	    }
	    while(c != EOF && c == '\n');
	    if(c == 't')
	    {
		twoway->sayHello();
	    }
	    else if(c == 'o')
	    {
		oneway->sayHello();
	    }
	    else if(c == 'O')
	    {
		batchOneway->sayHello();
	    }
	    else if(c == 'f')
	    {
		communicator->flushBatchRequests();
	    }
	    else if(c == 'T')
	    {
		if(timeout == -1)
		{
		    timeout = 2000;
		}
		else
		{
		    timeout = -1;
		}
		
		twoway = HelloPrx::uncheckedCast(twoway->ice_timeout(timeout));
		oneway = HelloPrx::uncheckedCast(oneway->ice_timeout(timeout));
		batchOneway = HelloPrx::uncheckedCast(batchOneway->ice_timeout(timeout));
		
		if(timeout == -1)
		{
		    printf("timeout is now switched off\n");
		}
		else
		{
		    printf("timeout is now set to 2000ms\n");
		}
	    }
	    else if(c == 's')
	    {
		twoway->shutdown();
	    }
	    else if(c == 'x')
	    {
		// Nothing to do
	    }
	    else if(c == '?')
	    {
		menu();
	    }
	    else
	    {
		printf("unknown command `%c'\n", c);
		menu();
	    }
	}
	catch(const IceE::Exception& ex)
	{
	    fprintf(stderr, "%s\n", ex.toString().c_str());
	}
    }
    while(c != EOF && c != 'x');

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
