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

#include <Ice/Ice.h>
#include <Parser.h>

using namespace std;
using namespace Ice;

void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options] [file...]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"-e COMMANDS          Execute COMMANDS.\n"
	"-d, --debug          Print debug messages.\n"
	;
}

int
runParser(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string commands;
    bool debug = false;

    int idx = 1;
    while(idx < argc)
    {
	if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
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
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
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
	else if(argv[idx][0] == '-')
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

    if(argc >= 2 && !commands.empty())
    {
	cerr << argv[0] << ": `-e' option cannot be used if input files are given" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "PhoneBook.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator->stringToProxy(proxy);
    PhoneBookPrx phoneBook = PhoneBookPrx::checkedCast(base);
    if(!phoneBook)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    ParserPtr parser = Parser::createParser(communicator, phoneBook);
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
	    FILE* file = fopen(argv[idx], "r");
	    if(file == NULL)
	    {
		cerr << argv[0] << ": can't open file `" << argv[idx] << "': " << strerror(errno) << endl;
		return EXIT_FAILURE;
	    }
	    
	    int parseStatus = parser->parse(file, debug);

	    fclose(file);

	    if(parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	}
    }

    return status;
}
