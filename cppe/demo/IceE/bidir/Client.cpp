// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/UUID.h>
#include <IceE/IceE.h>
#include <Callback.h>

using namespace std;
using namespace Ice;
using namespace Demo;

class CallbackReceiverI : public CallbackReceiver
{
public:

    virtual void
    callback(Int num, const Current&)
    {
	printf("received callback #%d\n", num);
    }
};

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

    CallbackSenderPrx server = CallbackSenderPrx::checkedCast(communicator->stringToProxy(proxy));
    if(!server)
    {
	fprintf(stderr, "%s: invalid proxy\n", argv[0]);
	return EXIT_FAILURE;
    }

    ObjectAdapterPtr adapter = communicator->createObjectAdapter("Callback.Client");
    Identity ident;
    ident.name = IceUtil::generateUUID();
    ident.category = "";
    adapter->add(new CallbackReceiverI, ident);
    adapter->activate();
    server->ice_connection()->setAdapter(adapter);
    server->addClient(ident);
    communicator->waitForShutdown();

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
