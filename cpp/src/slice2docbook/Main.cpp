// **********************************************************************
//
// Copyright (c) 2003
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

#include <Slice/Preprocessor.h>
#include <Gen.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] docbook-file slice-files...\n";
    cerr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-DNAME               Define NAME as 1.\n"
        "-DNAME=DEF           Define NAME as DEF.\n"
        "-UNAME               Remove any definition for NAME.\n"
        "-IDIR                Put DIR in the include file search path.\n"
        "-s, --stand-alone    Create stand-alone docbook file.\n"
        "--no-globals         Don't document the global module.\n"
        "--chapter            Use \"chapter\" instead of \"section\" as\n"
        "                     top-level element.\n"
        "-d, --debug          Print debug messages.\n"
        "--ice                Permit `Ice' prefix (for building Ice source code only)\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

int
main(int argc, char* argv[])
{
    string cppArgs;
    bool debug = false;
    bool ice = false;
    bool caseSensitive = false;
    bool standAlone = false;
    bool noGlobals = false;
    bool chapter = false;

    int idx = 1;
    while(idx < argc)
    {
	if(strncmp(argv[idx], "-I", 2) == 0)
	{
	    cppArgs += ' ';
	    cppArgs += argv[idx];

	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strncmp(argv[idx], "-D", 2) == 0 || strncmp(argv[idx], "-U", 2) == 0)
	{
	    cppArgs += ' ';
	    cppArgs += argv[idx];

	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "-s") == 0 || strcmp(argv[idx], "--stand-alone") == 0)
	{
	    standAlone = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--no-globals") == 0)
	{
	    noGlobals = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--chapter") == 0)
	{
	    chapter = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
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
	else if(strcmp(argv[idx], "--ice") == 0)
	{
	    ice = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--case-sensitive") == 0)
	{
	    caseSensitive = true;
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

    if(argc < 2)
    {
	cerr << argv[0] << ": no docbook file specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string docbook(argv[1]);
    string suffix;
    string::size_type pos = docbook.rfind('.');
    if(pos != string::npos)
    {
	suffix = docbook.substr(pos);
	transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
    }
    if(suffix != ".sgml")
    {
	cerr << argv[0] << ": docbook file must end with `.sgml'" << endl;
	return EXIT_FAILURE;
    }

    if(argc < 3)
    {
	cerr << argv[0] << ": no input file" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    UnitPtr p = Unit::createUnit(true, false, ice, caseSensitive);

    int status = EXIT_SUCCESS;

    for(idx = 2 ; idx < argc ; ++idx)
    {
	Preprocessor icecpp(argv[0], argv[idx], cppArgs);
	FILE* cppHandle = icecpp.preprocess(true);

	if(cppHandle == 0)
	{
	    p->destroy();
	    return EXIT_FAILURE;
	}
	
	status = p->parse(cppHandle, debug);

	if(!icecpp.close())
	{
	    p->destroy();
	    return EXIT_FAILURE;
	}
    }

    if(status == EXIT_SUCCESS)
    {
	Gen gen(argv[0], docbook, standAlone, noGlobals, chapter);
	if(!gen)
	{
	    p->destroy();
	    return EXIT_FAILURE;
	}
	gen.generate(p);
    }
    
    p->destroy();

    return status;
}
