// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <IceBox/ServiceManagerI.h>

using namespace std;
using namespace Ice;
using namespace IceBox;

namespace IceBox
{

class Server : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

}

int
main(int argc, char* argv[])
{
    Server server;
    return server.main(argc, argv);
}

void
Server::usage()
{
    cerr << "Usage: " << appName() << " [options] --Ice.Config=<file>\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
Server::run(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    
    vector<string> args;
    try
    {
    	args = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
        cerr << e.reason << endl;
	usage();
	return EXIT_FAILURE;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage();
	return EXIT_SUCCESS;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	cout << ICE_STRING_VERSION << endl;
	return EXIT_SUCCESS;
    }

    if(!args.empty())
    {
	usage();
	return EXIT_FAILURE;
    }

    ServiceManagerI* serviceManagerImpl = new ServiceManagerI(this, argc, argv);
    ServiceManagerPtr serviceManager = serviceManagerImpl;
    return serviceManagerImpl->run();
}
