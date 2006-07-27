// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Options.h>
#include <Slice/Preprocessor.h>
#include <Slice/RubyUtil.h>

#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace Slice::Ruby;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] slice-files...\n";
    cerr <<        
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-DNAME               Define NAME as 1.\n"
        "-DNAME=DEF           Define NAME as DEF.\n"
        "-UNAME               Remove any definition for NAME.\n"
        "-IDIR                Put DIR in the include file search path.\n"
	"-E                   Print preprocessor output on stdout.\n"
        "--output-dir DIR     Create files in the directory DIR.\n"
        "-d, --debug          Print debug messages.\n"
        "--ice                Permit `Ice' prefix (for building Ice source code only)\n"
        "--all                Generate code for Slice definitions in included files.\n"
        "--checksum           Generate checksums for Slice definitions.\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

int
main(int argc, char* argv[])
{
    string cppArgs;
    vector<string> includePaths;
    bool preprocess;
    string output;
    bool debug;
    bool ice;
    bool all;
    bool checksum;
    bool caseSensitive;

    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceUtil::Options::NeedArg);
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "all");
    opts.addOpt("", "checksum");
    opts.addOpt("", "case-sensitive");
     
    vector<string> args;
    try
    {
        args = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
	cerr << argv[0] << ": " << e.reason << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage(argv[0]);
	return EXIT_SUCCESS;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	cout << ICE_STRING_VERSION << endl;
	return EXIT_SUCCESS;
    }
    if(opts.isSet("D"))
    {
	vector<string> optargs = opts.argVec("D");
	for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
	{
	    cppArgs += " -D" + *i;
	}
    }
    if(opts.isSet("U"))
    {
	vector<string> optargs = opts.argVec("U");
	for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
	{
	    cppArgs += " -U" + *i;
	}
    }
    if(opts.isSet("I"))
    {
	includePaths = opts.argVec("I");
	for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
	{
	    cppArgs += " -I" + *i;
	}
    }
    preprocess = opts.isSet("E");
    if(opts.isSet("output-dir"))
    {
	output = opts.optArg("output-dir");
    }
    debug = opts.isSet("d") || opts.isSet("debug");
    ice = opts.isSet("ice");
    all = opts.isSet("all");
    checksum = opts.isSet("checksum");
    caseSensitive = opts.isSet("case-sensitive");

    if(args.empty())
    {
        cerr << argv[0] << ": no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        Preprocessor icecpp(argv[0], *i, cppArgs);
        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            return EXIT_FAILURE;
        }

	if(preprocess)
	{
	    char buf[4096];
	    while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != NULL)
	    {
		if(fputs(buf, stdout) == EOF)
		{
		    return EXIT_FAILURE;
		}
	    }
	    if(!icecpp.close())
	    {
		return EXIT_FAILURE;
	    }
	}
	else
	{
	    UnitPtr u = Unit::createUnit(false, all, ice, caseSensitive);
	    int parseStatus = u->parse(cppHandle, debug);

	    if(!icecpp.close())
	    {
		u->destroy();
		return EXIT_FAILURE;
	    }

	    if(parseStatus == EXIT_FAILURE)
	    {
		status = EXIT_FAILURE;
	    }
	    else
	    {
		string base = icecpp.getBaseName();
		string::size_type pos = base.find_last_of("/\\");
		if(pos != string::npos)
		{
		    base.erase(0, pos + 1);
		}

		string file = base + ".rb";
		if(!output.empty())
		{
		    file = output + '/' + file;
		}

		IceUtil::Output out;
		out.open(file.c_str());
		if(!out)
		{
		    cerr << argv[0] << ": can't open `" << file << "' for writing" << endl;
		    u->destroy();
		    return EXIT_FAILURE;
		}

		printHeader(out);
		out << "\n# Generated from file `" << base << ".ice'\n";

		//
		// Generate the Ruby mapping.
		//
		generate(u, all, checksum, includePaths, out);
	    }

	    u->destroy();
	}
    }

    return status;
}
