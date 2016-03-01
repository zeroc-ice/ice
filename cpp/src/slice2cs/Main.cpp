// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include "Gen.h"

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
        "-h, --help              Show this message.\n"
        "-v, --version           Display the Ice version.\n"
        "--validate               Validate command line options.\n"
        "-DNAME                  Define NAME as 1.\n"
        "-DNAME=DEF              Define NAME as DEF.\n"
        "-UNAME                  Remove any definition for NAME.\n"
        "-IDIR                   Put DIR in the include file search path.\n"
        "-E                      Print preprocessor output on stdout.\n"
        "--output-dir DIR        Create files in the directory DIR.\n"
        "--tie                   Generate TIE classes.\n"
        "--impl                  Generate sample implementations.\n"
        "--impl-tie              Generate sample TIE implementations.\n"
        "--depend                Generate Makefile dependencies.\n"
        "--depend-xml            Generate dependencies in XML format.\n"
        "--depend-file FILE      Write dependencies to FILE instead of standard output.\n"
        "-d, --debug             Print debug messages.\n"
        "--ice                   Allow reserved Ice prefix in Slice identifiers.\n"
        "--underscore            Allow underscores in Slice identifiers.\n"
        "--checksum              Generate checksums for Slice definitions.\n"
        ;
}

int
compile(int argc, char* argv[])
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "tie");
    opts.addOpt("", "impl");
    opts.addOpt("", "impl-tie");
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceUtilInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "underscore");
    opts.addOpt("", "checksum");

    bool validate = false;
    for(int i = 0; i < argc; ++i)
    {
        if(string(argv[i]) == "--validate")
        {
            validate = true;
            break;
        }
    }

    vector<string> args;
    try
    {
        args = opts.parse(argc, const_cast<const char**>(argv));
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        getErrorStream() << argv[0] << ": error: " << e.reason << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
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

    vector<string> includePaths = opts.argVec("I");
    for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool tie = opts.isSet("tie");

    bool impl = opts.isSet("impl");

    bool implTie = opts.isSet("impl-tie");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool underscore = opts.isSet("underscore");

    bool checksum = opts.isSet("checksum");

    if(args.empty())
    {
        getErrorStream() << argv[0] << ": error: no input file" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(impl && implTie)
    {
        getErrorStream() << argv[0] << ": error: cannot specify both --impl and --impl-tie" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(depend && dependxml)
    {
        getErrorStream() << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(validate)
    {
        return EXIT_SUCCESS;
    }

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    DependOutputUtil out(dependFile);
    if(dependxml)
    {
        out.os() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

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

        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2CS__");

            if(cppHandle == 0)
            {
                out.cleanup();
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit(false, false, ice, underscore);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if(parseStatus == EXIT_FAILURE)
            {
                out.cleanup();
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(out.os(), depend ? Preprocessor::CSharp : Preprocessor::SliceXML, includePaths,
                                                  "-D__SLICE2CS__"))
            {
                out.cleanup();
                return EXIT_FAILURE;
            }

            if(!icecpp->close())
            {
                out.cleanup();
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2CS__");

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
                UnitPtr p = Unit::createUnit(false, false, ice, underscore);
                int parseStatus = p->parse(*i, cppHandle, debug);

                if(!icecpp->close())
                {
                    p->destroy();
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
                        Gen gen(icecpp->getBaseName(), includePaths, output, impl, implTie);
                        gen.generate(p);
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
                        if(checksum)
                        {
                            gen.generateChecksums(p);
                        }
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
            }
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

            if(interrupted)
            {
                out.cleanup();
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if(dependxml)
    {
        out.os() << "</dependencies>\n";
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
