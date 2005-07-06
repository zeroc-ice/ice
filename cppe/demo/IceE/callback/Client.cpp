// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Callback.h>

using namespace std;
using namespace Ice;
using namespace Demo;

class CallbackReceiverI : public CallbackReceiver
{
public:

    virtual void callback(const Current&)
    {
	printf("received callback\n");
    }
};


void
menu()
{
    printf("usage:\n");
    printf("t: send callback as twoway\n");
    printf("o: send callback as oneway\n");
    printf("O: send callback as batch oneway\n");
    printf("f: flush all batch requests\n");
    printf("s: shutdown server\n");
    printf("x: exit\n");
    printf("?: help\n");
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "Callback.Client.CallbackServer";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	fprintf(stderr, "%s: property `%s' not set\n", argv[0], proxyProperty);
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator->stringToProxy(proxy);
    CallbackPrx twoway = CallbackPrx::checkedCast(base->ice_twoway()->ice_timeout(-1));
    if(!twoway)
    {
	fprintf(stderr, "%s: invalid proxy\n", argv[0]);
	return EXIT_FAILURE;
    }
    CallbackPrx oneway = CallbackPrx::uncheckedCast(twoway->ice_oneway());
    CallbackPrx batchOneway = CallbackPrx::uncheckedCast(twoway->ice_batchOneway());
    
    ObjectAdapterPtr adapter = communicator->createObjectAdapter("Callback.Client");
    adapter->add(new CallbackReceiverI, stringToIdentity("callbackReceiver"));
    adapter->activate();

    CallbackReceiverPrx twowayR = CallbackReceiverPrx::uncheckedCast(
	adapter->createProxy(stringToIdentity("callbackReceiver")));
    CallbackReceiverPrx onewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_oneway());

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
		twoway->initiateCallback(twowayR);
	    }
	    else if(c == 'o')
	    {
		oneway->initiateCallback(onewayR);
	    }
	    else if(c == 'O')
	    {
		batchOneway->initiateCallback(onewayR);
	    }
	    else if(c == 'f')
	    {
		communicator->flushBatchRequests();
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
	catch(const Exception& ex)
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
