// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <AdminI.h>
#include <Forward.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] add|remove reference...\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"--admin ENDPOINTS    Use ENDPOINTS as administrative endpoints.\n"
	;
}

int
run(int argc, char* argv[], CommunicatorPtr communicator)
{

    PropertiesPtr properties = communicator->getProperties();

    string adminEndpoints = properties->getProperty("Ice.Adapter.Admin.Endpoints");
    if(adminEndpoints.length() == 0)
    {
	cerr << argv[0] << ": `Ice.Adapter.Admin.Endpoints' property is not set" << endl;
	return EXIT_FAILURE;
    }

    if (argc < 2)
    {
	cerr << argv[0] << ": no command" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string cmd = argv[1];

    if (argc < 3)
    {
	cerr << argv[0] << ": no reference" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx adminBase = communicator->stringToProxy("admin:" + adminEndpoints);
    AdminPrx admin = AdminPrx::checkedCast(adminBase);
    if (!admin)
    {
	cerr << argv[0] << ": `" << adminEndpoints << "' are no valid administrative endpoints" << endl;
	return EXIT_FAILURE;
    }

    if (cmd == "add")
    {
	for (int i = 2; i < argc; ++i)
	{
	    ServerDescriptionPtr desc = new ServerDescription;
	    desc->object = communicator->stringToProxy(argv[i]);
	    admin->add(desc);
	}
    }
    else if (cmd == "remove")
    {
	for (int i = 2; i < argc; ++i)
	{
	    admin->remove(communicator->stringToProxy(argv[i]));
	}
    }
    else
    {
	cerr << argv[0] << ": invalid command `" << cmd << "'" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    PropertiesPtr properties = getDefaultProperties();

    int idx = 1;
    while (idx < argc)
    {
	if (strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	else if (strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else if (strcmp(argv[idx], "--admin") == 0)
	{
	    if (idx + 1 >= argc)
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
	    
	    properties->setProperty("Ice.Adapter.Admin.Endpoints", argv[idx + 1]);
	    for (int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if (argv[idx][0] == '-')
	{
	    cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}
	else
	{
	    ++idx;
	}
    }

    int status;
    CommunicatorPtr communicator;

    try
    {
	communicator = initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const LocalException& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const LocalException& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
