// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


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
        else if(strncmp(argv[i], "--", 2) != 0)
        {
            cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
            usage();
            return EXIT_FAILURE;
        }
    }

    ServiceManagerI* serviceManagerImpl = new ServiceManagerI(this, argc, argv);
    ServiceManagerPtr serviceManager = serviceManagerImpl;
    return serviceManagerImpl->run();
}
