// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
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
        "-h, --help               Show this message.\n"
        "-v, --version            Display the Ice version.\n"
        "--header-ext EXT         Use EXT instead of the default `h' extension.\n"
        "--source-ext EXT         Use EXT instead of the default `cpp' extension.\n"
        "--add-header HDR[,GUARD] Add #include for HDR (with guard GUARD) to generated source file.\n"
        "-DNAME                   Define NAME as 1.\n"
        "-DNAME=DEF               Define NAME as DEF.\n"
        "-UNAME                   Remove any definition for NAME.\n"
        "-IDIR                    Put DIR in the include file search path.\n"
        "-E                       Print preprocessor output on stdout.\n"
        "--include-dir DIR        Use DIR as the header include directory in source files.\n"
        "--output-dir DIR         Create files in the directory DIR.\n"
        "--dll-export SYMBOL      Use SYMBOL for DLL exports.\n"
        "--impl                   Generate sample implementations.\n"
        "--depend                 Generate Makefile dependencies.\n"
        "-d, --debug              Print debug messages.\n"
        "--ice                    Permit `Ice' prefix (for building Ice source code only)\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

int
main(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "header-ext", IceUtil::Options::NeedArg, "h");
    opts.addOpt("", "source-ext", IceUtil::Options::NeedArg, "cpp");
    opts.addOpt("", "add-header", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "include-dir", IceUtil::Options::NeedArg);
    opts.addOpt("", "output-dir", IceUtil::Options::NeedArg);
    opts.addOpt("", "dll-export", IceUtil::Options::NeedArg);
    opts.addOpt("", "impl");
    opts.addOpt("", "depend");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "case-sensitive");

    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        cout << ICEE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    string headerExtension = opts.optArg("header-ext");
    string sourceExtension = opts.optArg("source-ext");

    vector<string>extraHeaders = opts.argVec("add-header");

    string cppArgs;
    vector<string> optargs = opts.argVec("D");
    vector<string>::const_iterator i;
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs += " -D" + Preprocessor::addQuotes(*i);
    }

    optargs = opts.argVec("U");
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs += " -U" + Preprocessor::addQuotes(*i);
    }

    vector<string> includePaths = opts.argVec("I");
    for(i = includePaths.begin(); i != includePaths.end(); ++i)
    {
	cppArgs += " -I" + Preprocessor::normalizeIncludePath(*i);
    }

    bool preprocess = opts.isSet("E");

    string include = opts.optArg("include-dir");

    string output = opts.optArg("output-dir");

    string dllExport = opts.optArg("dll-export");

    bool impl = opts.isSet("impl");

    bool depend = opts.isSet("depend");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool caseSensitive = opts.isSet("case-sensitive");

    if(args.empty())
    {
        cerr << argv[0] << ": no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    for(i = args.begin(); i != args.end(); ++i)
    {
        if(depend)
        {
            Preprocessor icecpp(argv[0], *i, cppArgs);
            icecpp.printMakefileDependencies(Preprocessor::CPlusPlus);
        }
        else
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
                UnitPtr u = Unit::createUnit(false, false, ice, caseSensitive);
                int parseStatus = u->parse(cppHandle, debug, Slice::IceE);
            
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
                    Gen gen(argv[0], icecpp.getBaseName(), headerExtension, sourceExtension, extraHeaders, include,
                            includePaths, dllExport, output, impl, ice);
                    if(!gen)
                    {
                        u->destroy();
                        return EXIT_FAILURE;
                    }
                    gen.generate(u);
                }

                u->destroy();
            }
        }
    }

    return status;
}
