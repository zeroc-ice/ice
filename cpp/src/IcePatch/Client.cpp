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
    void printNode(const NodePrx&);
    void printNodes(const Nodes&, const string&);
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
    printNode(top);

    return EXIT_SUCCESS;
}

void
IcePatch::Client::printNode(const NodePrx& node)
{
    string name = node->ice_getIdentity().name;
    string::size_type pos = name.rfind('/');
    if (pos != string::npos)
    {
	name.erase(0, pos + 1);
    }

    cout << name << endl;
    
    DirectoryPrx directory = DirectoryPrx::checkedCast(node);
    if (directory)
    {
	printNodes(directory->getNodes(), "");
    }
}

void
IcePatch::Client::printNodes(const Nodes& nodes, const string& indent)
{
    for (unsigned int i = 0; i < nodes.size(); ++i)
    {
	string name = nodes[i]->ice_getIdentity().name;
	string::size_type pos = name.rfind('/');
	if (pos != string::npos)
	{
	    name.erase(0, pos + 1);
	}
    
	cout << indent << "+-" << name << endl;
	
	DirectoryPrx directory = DirectoryPrx::checkedCast(nodes[i]);
	if (directory)
	{
	    string newIndent;
	    if (i < nodes.size() - 1)
	    {
		newIndent = indent + "| ";
	    }
	    else
	    {
		newIndent = indent + "  ";
	    }

	    printNodes(directory->getNodes(), newIndent);
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
