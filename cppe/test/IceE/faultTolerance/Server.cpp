// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/IceE.h>
#include <TestApplication.h>
#include <TestI.h>

using namespace std;

void
usage(const char* n)
{
    tprintf("Usage: %s port\n", n);
}

class FaultToleranceTestApplication : public TestApplication
{
public:

    FaultToleranceTestApplication() :
        TestApplication("fault server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        loadConfig(initData.properties);
	initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        int port = 0;
        for(int i = 1; i < argc; ++i)
        {
	    if(argv[i][0] == '-')
	    {
	        tprintf("%s: unknown option `%s'\n", argv[0], argv[i]);
	        usage(argv[0]);
	        return EXIT_FAILURE;
	    }

	    if(port > 0)
	    {
	        tprintf("%s: only one port can be specified\n", argv[0]);
	        usage(argv[0]);
	        return EXIT_FAILURE;
	    }

	    port = atoi(argv[i]);
        }

        if(port <= 0)
        {
	    tprintf("%s: no port specified\n", argv[0]);
	    usage(argv[0]);
	    return EXIT_FAILURE;
        }

        char buf[32];
        sprintf(buf, "default -p %d", port);
        communicator()->getProperties()->setProperty("TestAdapter.Endpoints", buf);
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new TestI(adapter);
        adapter->add(object, communicator()->stringToIdentity("test"));
        adapter->activate();
        communicator()->waitForShutdown();
        return EXIT_SUCCESS;
    }
};

int
main(int argc, char** argv)
{
    FaultToleranceTestApplication app;
    return app.main(argc, argv);
}
