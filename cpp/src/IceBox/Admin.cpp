// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Ice/SliceChecksums.h>
#include <IceBox/IceBox.h>

using namespace std;
using namespace Ice;

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Client app;
    int rc = app.main(argc, argv);

    return rc;
}

void
Client::usage()
{
    cerr << "Usage: " << appName() << " [options] [command...]\n";
    cerr <<        
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "\n"
        "Commands:\n"
        "shutdown             Shutdown the server.\n"
        ;
}

int
Client::run(int argc, char* argv[])
{
    vector<string> commands;

    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
        {
            usage();
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
        {
            cout << ICE_STRING_VERSION << endl;
            return EXIT_SUCCESS;
        }
        else if(argv[idx][0] == '-')
        {
            cerr << appName() << ": unknown option `" << argv[idx] << "'" << endl;
            usage();
            return EXIT_FAILURE;
        }
        else
        {
            commands.push_back(argv[idx]);
            ++idx;
        }
    }

    if(commands.empty())
    {
        usage();
        return EXIT_SUCCESS;
    }

    PropertiesPtr properties = communicator()->getProperties();
    string managerIdentity = properties->getPropertyWithDefault("IceBox.ServiceManager.Identity", "ServiceManager");

    string managerProxy;

    if(properties->getProperty("Ice.Default.Locator").empty())
    {
	string managerEndpoints = properties->getProperty("IceBox.ServiceManager.Endpoints");
	if(managerEndpoints.empty())
	{
	    cerr << appName() << ": property `IceBox.ServiceManager.Endpoints' is not set" << endl;
	    return EXIT_FAILURE;
	}

	managerProxy = managerIdentity + ":" + managerEndpoints;
    }
    else
    {
	string managerAdapterId = properties->getProperty("IceBox.ServiceManager.AdapterId");
	if(managerAdapterId.empty())
	{
	    cerr << appName() << ": property `IceBox.ServiceManager.AdapterId' is not set" << endl;
	    return EXIT_FAILURE;
	}

	managerProxy = managerIdentity + "@" + managerAdapterId;
    }

    ObjectPrx base = communicator()->stringToProxy(managerProxy);
    IceBox::ServiceManagerPrx manager = IceBox::ServiceManagerPrx::checkedCast(base);
    if(!manager)
    {
        cerr << appName() << ": `" << managerProxy << "' is not running" << endl;
        return EXIT_FAILURE;
    }

    Ice::SliceChecksumDict serverChecksums = manager->getSliceChecksums();
    Ice::SliceChecksumDict localChecksums = Ice::sliceChecksums();
    for(Ice::SliceChecksumDict::const_iterator p = localChecksums.begin(); p != localChecksums.end(); ++p)
    {
        Ice::SliceChecksumDict::const_iterator q = serverChecksums.find(p->first);
        if(q == serverChecksums.end())
        {
            cerr << appName() << ": server is using unknown Slice type `" << q->first << "'" << endl;
        }
        else if(p->second != q->second)
        {
            cerr << appName() << ": server is using a different Slice definition of `" << q->first << "'" << endl;
        }
    }

    vector<string>::const_iterator r;
    for(r = commands.begin(); r != commands.end(); ++r)
    {
        if((*r) == "shutdown")
        {
            manager->shutdown();
        }
        else
        {
            cerr << appName() << ": unknown command `" << *r << "'" << endl;
            usage();
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
