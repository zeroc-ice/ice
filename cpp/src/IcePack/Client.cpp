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
#include <IcePack/Parser.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IcePack;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] [file...]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"-DNAME               Define NAME as 1.\n"
	"-DNAME=DEF           Define NAME as DEF.\n"
	"-UNAME               Remove any definition for NAME.\n"
	"-IDIR                Put DIR in the include file search path.\n"
	"-e COMMANDS          Execute COMMANDS.\n"
	"-d, --debug          Print debug messages.\n"
	;
}

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string cpp("cpp");
    string commands;
    bool debug = false;

    int idx = 1;
    while (idx < argc)
    {
	if (strncmp(argv[idx], "-I", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if (strncmp(argv[idx], "-D", 2) == 0 || strncmp(argv[idx], "-U", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if (strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	else if (strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else if (strcmp(argv[idx], "-e") == 0)
	{
	    if (idx + 1 >= argc)
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
	    
	    commands += argv[idx + 1];
	    commands += ';';

	    for (int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if (strcmp(argv[idx], "-d") == 0 || strcmp(argv[idx], "--debug") == 0)
	{
	    debug = true;
	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
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

    if (argc >= 2 && !commands.empty())
    {
	cerr << argv[0] << ": `-e' option cannot be used if input files are given" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    PropertiesPtr properties = communicator->getProperties();

    string adminEndpoints = properties->getProperty("Ice.Adapter.Admin.Endpoints");
    if (adminEndpoints.length() == 0)
    {
	cerr << argv[0] << ": `Ice.Adapter.Admin.Endpoints' property is not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx adminBase = communicator->stringToProxy("admin:" + adminEndpoints);
    AdminPrx admin = AdminPrx::checkedCast(adminBase);
    if (!admin)
    {
	cerr << argv[0] << ": `" << adminEndpoints << "' are no valid administrative endpoints" << endl;
	return EXIT_FAILURE;
    }

    ParserPtr parser = Parser::createParser(communicator, admin);
    int status = EXIT_SUCCESS;

    if (argc < 2) // No files given
    {
	if (!commands.empty()) // Commands were given
	{
	    int parseStatus = parser->parse(commands, debug);
	    if (parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	}
	else // No commands, let's use standard input
	{
	    int parseStatus = parser->parse(stdin, debug);
	    if (parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	}
    }
    else // Process files given on the command line
    {
	for (idx = 1 ; idx < argc ; ++idx)
	{
	    ifstream test(argv[idx]);
	    if (!test)
	    {
		cerr << argv[0] << ": can't open `" << argv[idx] << "' for reading: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	    }
	    test.close();
	    
	    string cmd = cpp + " " + argv[idx];
#ifdef WIN32
	    FILE* cppHandle = _popen(cmd.c_str(), "r");
#else
	    FILE* cppHandle = popen(cmd.c_str(), "r");
#endif
	    if (cppHandle == NULL)
	    {
		cerr << argv[0] << ": can't run C++ preprocessor: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	    }
	    
	    int parseStatus = parser->parse(cppHandle, debug);
	    
#ifdef WIN32
	    _pclose(cppHandle);
#else
	    pclose(cppHandle);
#endif

	    if (parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	}
    }

    return status;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;

    try
    {
	communicator = initialize(argc, argv);
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
