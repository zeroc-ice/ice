// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
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
    PropertiesPtr properties = communicator()->getProperties();

    StringSeq args = argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("IceBox", args);
    stringSeqToArgs(args, argc, argv);

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

    const char* managerEndpointsProperty = "IceBox.ServiceManager.Endpoints";
    string managerEndpoints = properties->getProperty(managerEndpointsProperty);
    if(managerEndpoints.empty())
    {
        cerr << appName() << ": property `" << managerEndpointsProperty << "' is not set" << endl;
        return EXIT_FAILURE;
    }

    ObjectPrx base = communicator()->stringToProxy("ServiceManager:" + managerEndpoints);
    IceBox::ServiceManagerPrx manager = IceBox::ServiceManagerPrx::checkedCast(base);
    if(!manager)
    {
        cerr << appName() << ": `" << managerEndpoints << "' is not running" << endl;
        return EXIT_FAILURE;
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
