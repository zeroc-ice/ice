// **********************************************************************
//
// Copyright (c) 2002
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

#include <Ice/Application.h>
#include <Yellow/Parser.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace Yellow;

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
    return app.main(argc, argv);
}

void
Client::usage()
{
    cerr << "Usage: " << appName() << " [options] [file...]\n";
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
        "-s, --service name   the service name (defaults to Yellow).\n"
	;
}

int
Client::run(int argc, char* argv[])
{
    string cpp("cpp");
    string commands;
    bool debug = false;
    string service("Yellow");

    PropertiesPtr properties = communicator()->getProperties();

    StringSeq args = argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("Yellow", args);
    stringSeqToArgs(args, argc, argv);

    int idx = 1;
    while(idx < argc)
    {
	if(strncmp(argv[idx], "-I", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strncmp(argv[idx], "-D", 2) == 0 || strncmp(argv[idx], "-U", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage();
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[idx], "-e") == 0)
	{
	    if(idx + 1 >= argc)
            {
		cerr << appName() << ": argument expected for`" << argv[idx] << "'" << endl;
		usage();
		return EXIT_FAILURE;
            }
	    
	    commands += argv[idx + 1];
	    commands += ';';

	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if(strcmp(argv[idx], "-d") == 0 || strcmp(argv[idx], "--debug") == 0)
	{
	    debug = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "-s") == 0 || strcmp(argv[idx], "--service") == 0)
	{
	    if(idx + 1 >= argc)
            {
		cerr << appName() << ": argument expected for`" << argv[idx] << "'" << endl;
		usage();
		return EXIT_FAILURE;
            }
	    
	    service = argv[idx + 1];

	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if(argv[idx][0] == '-')
	{
	    cerr << appName() << ": unknown option `" << argv[idx] << "'" << endl;
	    usage();
	    return EXIT_FAILURE;
	}
	else
	{
	    ++idx;
	}
    }

    if(argc >= 2 && !commands.empty())
    {
	cerr << appName() << ": `-e' option cannot be used if input files are given" << endl;
	usage();
	return EXIT_FAILURE;
    }

    string queryProxy = communicator()->getProperties()->getProperty(service + ".Query");
    string adminProxy = communicator()->getProperties()->getProperty(service + ".Admin");

    if(queryProxy.empty())
    {
	queryProxy = "Yellow/Query@YellowQueryAdapter";
    }
    if(adminProxy.empty())
    {
	adminProxy = "Yellow/Admin@YellowAdminAdapter";
    }

    AdminPrx admin = AdminPrx::checkedCast(communicator()->stringToProxy(adminProxy));
    if(!admin)
    {
	cerr << appName() << ": `" <<  "' are not valid administrative endpoints" << endl;
	return EXIT_FAILURE;
    }

    QueryPrx query = QueryPrx::checkedCast(communicator()->stringToProxy(queryProxy));
    if(!query)
    {
	cerr << appName() << ": `" <<  "' are not valid query endpoints" << endl;
	return EXIT_FAILURE;
    }

    ParserPtr parser = Parser::createParser(communicator(), admin, query);
    int status = EXIT_SUCCESS;

    if(argc < 2) // No files given
    {
	if(!commands.empty()) // Commands were given
	{
	    int parseStatus = parser->parse(commands, debug);
	    if(parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	}
	else // No commands, let's use standard input
	{
	    int parseStatus = parser->parse(stdin, debug);
	    if(parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	}
    }
    else // Process files given on the command line
    {
	for(idx = 1 ; idx < argc ; ++idx)
	{
	    ifstream test(argv[idx]);
	    if(!test)
	    {
		cerr << appName() << ": can't open `" << argv[idx] << "' for reading: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	    }
	    test.close();
	    
	    string cmd = cpp + " " + argv[idx];
#ifdef _WIN32
	    FILE* cppHandle = _popen(cmd.c_str(), "r");
#else
	    FILE* cppHandle = popen(cmd.c_str(), "r");
#endif
	    if(cppHandle == NULL)
	    {
		cerr << appName() << ": can't run C++ preprocessor: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	    }
	    
	    int parseStatus = parser->parse(cppHandle, debug);
	    
#ifdef _WIN32
	    _pclose(cppHandle);
#else
	    pclose(cppHandle);
#endif

	    if(parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	}
    }

    return status;
}
