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

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] slice-file [type-id ...]\n";
    cerr <<
	"Options:\n"
	"-h, --help	      Show this message.\n"
	"-v, --version	      Display the Ice version.\n"
	"-DNAME		      Define NAME as 1.\n"
	"-DNAME=DEF	      Define NAME as DEF.\n"
	"-UNAME		      Remove any definition for NAME.\n"
	"-IDIR		      Put DIR in the include file search path.\n"
	"-d, --debug	      Print debug messages.\n"
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
    string include;
    string output;
    vector<string> includePaths;

    int idx = 1;
    while(idx < argc)
    {
	if(strncmp(argv[idx], "-I", 2) == 0)
	{
	    cppArgs += ' ';
	    cppArgs += argv[idx];

	    string path = argv[idx] + 2;
	    if(path.length())
	    {
		includePaths.push_back(path);
	    }

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
	else if(strcmp(argv[idx], "--include-dir") == 0)
	{
	    if(idx + 1 >= argc)
	    {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    
	    include = argv[idx + 1];
	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if(strcmp(argv[idx], "--output-dir") == 0)
	{
	    if(idx + 1 >= argc)
	    {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    
	    output = argv[idx + 1];
	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
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
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string sourceFile = argv[1];

    int status = EXIT_SUCCESS;

    Preprocessor icecpp(argv[0], sourceFile, cppArgs);
    FILE* cppHandle = icecpp.preprocess(true);
    
    if(cppHandle == 0)
    {
	return EXIT_FAILURE;
    }
    
    UnitPtr unit = Unit::createUnit(false, false, ice, caseSensitive);
    int parseStatus = unit->parse(cppHandle, debug);
    
    if(!icecpp.close())
    {
	unit->destroy();
	return EXIT_FAILURE;
    }	    
    
    if(parseStatus == EXIT_FAILURE)
    {
	status = EXIT_FAILURE;
    }
    else
    {
	Gen gen(argv[0], icecpp.getBaseName(), include, includePaths, output);

	if(argc > 2)
	{
	    for(idx = 2 ; idx < argc; ++idx)
	    {
		ClassDeclPtr classDecl;
		TypeList classTypes = unit->lookupType(argv[idx], false);
		if(!classTypes.empty())
		{
		    classDecl = ClassDeclPtr::dynamicCast(classTypes.front());
		}
		if(!classDecl)
		{
		    cerr << argv[0] << ": invalid type: " << argv[idx] << endl;
		    status = EXIT_FAILURE;
		    break;
		}
		gen.visitClassDefStart(classDecl->definition());
	    }
	}
	else
	{
	    unit->visit(&gen);
	}
    }
    
    unit->destroy();

    return status;
}
