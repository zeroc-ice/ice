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
    RegistryServer app;
    int rc = app.main(argc, argv);

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

