// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/StaticMutex.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <Gen.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;

static IceUtil::StaticMutex _mutex = ICE_STATIC_MUTEX_INITIALIZER;
static bool _interrupted = false;

void
interruptedCallback(int signal)
{
    IceUtil::StaticMutex::Lock lock(_mutex);

    _interrupted = true;
}

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
        "-E                   Print preprocessor output on stdout.\n"
        "-s, --stand-alone    Create stand-alone docbook file.\n"
        "--chapter            Use \"chapter\" instead of \"section\" as\n"
        "                     top-level element.\n"
        "--noindex            Suppress generation of index pages.\n"
        "--sort-fields        Sort fields of structures, classes, and exceptions.\n"
        "-d, --debug          Print debug messages.\n"
        "--ice                Permit `Ice' prefix (for building Ice source code only)\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

int
main(int argc, char* argv[])
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("s", "stand-alone");
    opts.addOpt("", "chapter");
    opts.addOpt("", "noindex");
    opts.addOpt("", "sort-fields");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "case-sensitive");

    vector<string> args;
    try
    {
#if defined(__BCPLUSPLUS__) && (__BCPLUSPLUS__ >= 0x0600)
        IceUtil::DummyBCC dummy;
#endif
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << argv[0] << ": error: " << e.reason << endl;
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
        cerr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    vector<string>::const_iterator i;
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-D" + *i);
    }

    optargs = opts.argVec("U");
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-U" + *i);
    }

    optargs = opts.argVec("I");
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    bool preprocess = opts.isSet("E");

    bool standAlone = opts.isSet("stand-alone");

    bool chapter = opts.isSet("chapter");

    bool noIndex = opts.isSet("noindex");

    bool sortFields = opts.isSet("sort-fields");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool caseSensitive = opts.isSet("case-sensitive");

    if(args.empty())
    {
        getErrorStream() << argv[0] << ": error: no docbook file specified" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    string docbook(args[0]);
    string suffix;
    string::size_type pos = docbook.rfind('.');
    if(pos != string::npos)
    {
        suffix = IceUtilInternal::toLower(docbook.substr(pos));
    }
    if(suffix != ".sgml")
    {
        getErrorStream() << argv[0] << ": error: docbook file must end with `.sgml'" << endl;
        return EXIT_FAILURE;
    }

    if(args.size() < 2)
    {
        getErrorStream() << argv[0] << ": error: no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    UnitPtr p = Unit::createUnit(true, false, ice, caseSensitive);

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    for(vector<string>::size_type idx = 1; idx < args.size(); ++idx)
    {
        Preprocessor icecpp(argv[0], args[idx], cppArgs);
        FILE* cppHandle = icecpp.preprocess(true);

        if(cppHandle == 0)
        {
            p->destroy();
            return EXIT_FAILURE;
        }
        if(preprocess)
        {
            char buf[4096];
            while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != NULL)
            {
                if(fputs(buf, stdout) == EOF)
                {
                    p->destroy();
                    return EXIT_FAILURE;
                }
            }
        }
        else
        {
            status = p->parse(args[idx], cppHandle, debug);
        }

        if(!icecpp.close())
        {
            p->destroy();
            return EXIT_FAILURE;
        }

        {
            IceUtil::StaticMutex::Lock lock(_mutex);

            if(_interrupted)
            {
                return EXIT_FAILURE;
            }
        }
    }

    if(status == EXIT_SUCCESS && !preprocess)
    {
        try
        {
            Gen gen(docbook, standAlone, chapter, noIndex, sortFields);
            gen.generate(p);
        }
        catch(const Slice::FileException& ex)
        {
            // If a file could not be created, then
            // cleanup any created files.
            FileTracker::instance()->cleanup();
            p->destroy();
            getErrorStream() << argv[0] << ": error: " << ex.reason() << endl;
            return EXIT_FAILURE;
        }
    }

    p->destroy();

    {
        IceUtil::StaticMutex::Lock lock(_mutex);

        if(_interrupted)
        {
            FileTracker::instance()->cleanup();
            return EXIT_FAILURE;
        }
    }

    return status;
}
