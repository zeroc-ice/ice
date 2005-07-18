// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestApplication.h>
#include <Test.h>

using namespace std;

void
usage(const char* n)
{
    tprintf("Usage: %s port...\n", n);
}

class FaultTestApplication : public TestApplication
{
public:

    FaultTestApplication() :
        TestApplication("fault client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
	Ice::PropertiesPtr properties = Ice::getDefaultProperties(argc, argv);

	//
	// This test aborts servers, so we don't want warnings.
	//
	properties->setProperty("IceE.Warn.Connections", "0");

        loadConfig(properties);
        setCommunicator(Ice::initializeWithProperties(argc, argv, properties));

        vector<int> ports;
        for(int i = 1; i < argc; ++i)
        {
	    if(argv[i][0] == '-')
	    {
	        tprintf("%s: unknown option `%s'\n", argv[0], argv[i]);
	        usage(argv[0]);
	        return EXIT_FAILURE;
	    }

	    ports.push_back(atoi(argv[i]));
        }

        if(ports.empty())
        {
	    tprintf("%s: no ports specified\n", argv[0]);
	    usage(argv[0]);
	    return EXIT_FAILURE;
        }

        try
        {
	    void allTests(const Ice::CommunicatorPtr&, const vector<int>&);
	    allTests(communicator(), ports);
        }
        catch(const Ice::Exception& ex)
        {
	    tprintf("%s\n", ex.toString().c_str());
	    test(false);
        }

        return EXIT_SUCCESS;
    }
};

int
main(int argc, char** argv)
{
    FaultTestApplication app;
    return app.main(argc, argv);
}
