// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <Gen.h>

using namespace std;
using namespace Slice;

namespace
{

IceUtil::Mutex* globalMutex = 0;
bool interrupted = false;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

void
interruptedCallback(int /*signal*/)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

    interrupted = true;
}

void
usage(const char* n)
{
    getErrorStream() << "Usage: " << n << " [options] slice-files...\n";
    getErrorStream() <<     
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
        "--ice                    Permit `Ice' prefix (for building Ice source code only).\n"
        "--underscore             Permit underscores in Slice identifiers.\n"
        "--checksum               Generate checksums for Slice definitions.\n"
        "--stream                 Generate marshaling support for public stream API.\n"
        ;
}

int
compile(int argc, char* argv[])
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "header-ext", IceUtilInternal::Options::NeedArg, "h");
    opts.addOpt("", "source-ext", IceUtilInternal::Options::NeedArg, "cpp");
    opts.addOpt("", "add-header", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "include-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "dll-export", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "impl");
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-header");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "underscore");
    opts.addOpt("", "checksum");
    opts.addOpt("", "stream");

    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        getErrorStream() << argv[0] << ": " << e.reason << endl;
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
        getErrorStream() << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    string headerExtension = opts.optArg("header-ext");
    string sourceExtension = opts.optArg("source-ext");
    
    vector<string> extraHeaders = opts.argVec("add-header");

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-D" + *i);
    }

    optargs = opts.argVec("U");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-U" + *i);
    }

    vector<string> includePaths;
    includePaths = opts.argVec("I");
    for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    bool preprocess = opts.isSet("E");

    string include = opts.optArg("include-dir");

    string output = opts.optArg("output-dir");

    string dllExport = opts.optArg("dll-export");

    bool impl = opts.isSet("impl");

    bool depend = opts.isSet("depend");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool underscore = opts.isSet("underscore");

    bool checksum = opts.isSet("checksum");

    bool stream = opts.isSet("stream");

    if(args.empty())
    {
        getErrorStream() << argv[0] << ": error: no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        vector<string>::iterator p = find(args.begin(), args.end(), *i);
        if(p != i)
        {
            continue;
        }

        if(depend)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2CPP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }
            
            UnitPtr u = Unit::createUnit(false, false, ice, underscore);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if(parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(Preprocessor::CPlusPlus, includePaths,
                                                  "-D__SLICE2CPP__", sourceExtension, headerExtension))
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2CPP__");

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
                if(!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit(false, false, ice, underscore);
                int parseStatus = u->parse(*i, cppHandle, debug);
            
                if(!icecpp->close())
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
                    try
                    {
                        Gen gen(icecpp->getBaseName(), headerExtension, sourceExtension, extraHeaders, include,
                                includePaths, dllExport, output, impl, checksum, stream, ice);
                        gen.generate(u);
                    }
                    catch(const Slice::FileException& ex)
                    {
                        // If a file could not be created, then
                        // cleanup any created files.
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        getErrorStream() << argv[0] << ": error: " << ex.reason() << endl;
                        return EXIT_FAILURE;
                    }
                }

                u->destroy();
            }
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

            if(interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    return status;
}

int
main(int argc, char* argv[])
{
    try
    {
        return compile(argc, argv);
    }
    catch(const std::exception& ex)
    {
        getErrorStream() << argv[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(const std::string& msg)
    {
        getErrorStream() << argv[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(const char* msg)
    {
        getErrorStream() << argv[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        getErrorStream() << argv[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
