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
#include <Freeze/Freeze.h>
#include <Ice/Application.h>
#include <IcePack/Registry.h>

#include <Ice/Xerces.h>
#include <xercesc/util/PlatformUtils.hpp>

using namespace std;
using namespace IcePack;

class RegistryServer : public Ice::Application
{
public:

    void usage();
    virtual int run(int argc, char* argv[]);

};

int
main(int argc, char* argv[])
{
    try
    {
	ICE_XERCES_NS XMLPlatformUtils::Initialize();
    }
    catch(const ICE_XERCES_NS XMLException& e)
    {
	cout << e.getMessage() << endl;
	return EXIT_FAILURE;
    }

    RegistryServer app;
    int rc = app.main(argc, argv);

    ICE_XERCES_NS XMLPlatformUtils::Terminate();

    return rc;
}

void
RegistryServer::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"--nowarn             Don't print any security warnings.\n"
	;
}

int
RegistryServer::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();    
    
    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("IcePack", args);
    args = properties->parseCommandLineOptions("Freeze", args);
    Ice::stringSeqToArgs(args, argc, argv);

    bool nowarn = false;
    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage();
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[i], "--nowarn") == 0)
	{
	    nowarn = true;
	}
	else
	{
	    cerr << appName() << ": unknown option `" << argv[i] << "'" << endl;
	    usage();
	    return EXIT_FAILURE;
	}
    }
    
    Registry registry(communicator());
    if(!registry.start(nowarn, false))
    {
	return EXIT_FAILURE;
    }

    shutdownOnInterrupt();
    communicator()->waitForShutdown();
    ignoreInterrupt();
    
    return EXIT_SUCCESS;
}

