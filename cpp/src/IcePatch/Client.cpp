// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <IcePatch/Node.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

namespace IcePatch
{

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
    void printNode(const NodePrx&, int);
};

};

void
IcePatch::Client::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
IcePatch::Client::run(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
	if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage();
	    return EXIT_SUCCESS;
	}
	else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else
	{
	    cerr << appName() << ": unknown option `" << argv[i] << "'" << endl;
	    usage();
	    return EXIT_FAILURE;
	}
    }

    PropertiesPtr properties = communicator()->getProperties();
    
    //
    // Get the working directory and change to this directory.
    //
    const char* directoryProperty = "IcePatch.Directory";
    string directory = properties->getProperty(directoryProperty);
    if (!directory.empty())
    {
	if (chdir(directory.c_str()) == -1)
	{
	    cerr << appName() << ": can't change to directory `" << directory << "': " << strerror(errno) << endl;
	    return EXIT_FAILURE;
	}
    }

    //
    // Get the IcePatch endpoints.
    //
    const char* endpointsProperty = "IcePatch.Endpoints";
    string endpoints = properties->getProperty(endpointsProperty);
    if (endpoints.empty())
    {
	cerr << appName() << ": property `" << endpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }

    //
    // Display node structure.
    //
    ObjectPrx topObj = communicator()->stringToProxy("IcePatch/.:" + endpoints);
    NodePrx top = NodePrx::checkedCast(topObj);
    printNode(top, 0);

    return EXIT_SUCCESS;
}

void
IcePatch::Client::printNode(const NodePrx& node, int ident)
{
    int i;

    for (i = 0; i < ident; ++i)
    {
	if (i == ident - 1)
	{
	    cout << '+';
	}
	else
	{
	    cout << ' ';
	}
    }
    
    cout << node->ice_getIdentity().name << endl;

    DirectoryPrx directory = DirectoryPrx::checkedCast(node);
    if (directory)
    {
	Nodes nodes = directory->getNodes();
	for (i = 0; i < static_cast<int>(nodes.size()); ++i)
	{
	    printNode(nodes[i], ident + 1);
	}
    }
}

int
main(int argc, char* argv[])
{
    addArgumentPrefix("IcePatch");
    Client app;
    return app.main(argc, argv);
}
