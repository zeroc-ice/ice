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
    cerr << "Usage: " << n << " [options] slice-files...\n";
    cerr <<        
        "Options:\n"
        "-h, --help              Show this message.\n"
        "-v, --version           Display the Ice version.\n"
        "-DNAME                  Define NAME as 1.\n"
        "-DNAME=DEF              Define NAME as DEF.\n"
        "-UNAME                  Remove any definition for NAME.\n"
        "-IDIR                   Put DIR in the include file search path.\n"
        "--tie                   Generate TIE classes.\n"
        "--impl                  Generate sample implementations.\n"
        "--impl-tie              Generate sample TIE implementations.\n"
	"--depend                Generate Makefile dependencies.\n"
        "-d, --debug             Print debug messages.\n"
        "--ice                   Permit `Ice' prefix (for building Ice source code only)\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

int
main(int argc, char* argv[])
{
    string cppArgs;
    vector<string> includePaths;
    bool tie = false;
    bool impl = false;
    bool implTie = false;
    bool debug = false;
    bool ice = false;
    bool caseSensitive = false;
    bool depend = false;

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
        else if(strncmp(argv[idx], "-D", 2) == 0 ||
                 strncmp(argv[idx], "-U", 2) == 0)
        {
            cppArgs += ' ';
            cppArgs += argv[idx];

            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "-h") == 0 ||
                 strcmp(argv[idx], "--help") == 0)
        {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[idx], "-v") == 0 ||
                 strcmp(argv[idx], "--version") == 0)
        {
            cout << ICE_STRING_VERSION << endl;
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[idx], "-d") == 0 ||
                 strcmp(argv[idx], "--debug") == 0)
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
        else if(strcmp(argv[idx], "--tie") == 0)
        {
            tie = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--impl") == 0)
        {
            impl = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--impl-tie") == 0)
        {
            implTie = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
	else if(strcmp(argv[idx], "--depend") == 0)
	{
	    depend = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
        else if(argv[idx][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[idx] << "'"
                 << endl;
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
        cerr << argv[0] << ": no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(impl && implTie)
    {
        cerr << argv[0] << ": cannot specify both --impl and --impl-tie" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    for(idx = 1 ; idx < argc ; ++idx)
    {
	if(depend)
	{
	    Preprocessor icecpp(argv[0], argv[idx], cppArgs);
	    icecpp.printMakefileDependencies();
	}
	else
	{
	    Preprocessor icecpp(argv[0], argv[idx], cppArgs);
	    FILE* cppHandle = icecpp.preprocess(false);

	    if(cppHandle == 0)
	    {
		return EXIT_FAILURE;
	    }

	    UnitPtr p = Unit::createUnit(false, false, ice, caseSensitive);
	    int parseStatus = p->parse(cppHandle, debug);

	    if(!icecpp.close())
	    {
		return EXIT_FAILURE;
	    }	    
	    
	    if(parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	    else
	    {
		Gen gen(argv[0], icecpp.getBaseName(), includePaths);
		if(!gen)
		{
		    p->destroy();
		    return EXIT_FAILURE;
		}
		gen.generate(p);
#if 0
		if(tie)
		{
		    gen.generateTie(p);
		}
		if(impl)
		{
		    gen.generateImpl(p);
		}
		if(implTie)
		{
		    gen.generateImplTie(p);
		}
#endif
	    }

	    p->destroy();
	}
    }

    return status;
}
