//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Options.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/ConsoleUtil.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include "Gen.h"

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

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
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr <<
        "Options:\n"
        "-h, --help               Show this message.\n"
        "-v, --version            Display the Ice version.\n"
        "-DNAME                   Define NAME as 1.\n"
        "-DNAME=DEF               Define NAME as DEF.\n"
        "-UNAME                   Remove any definition for NAME.\n"
        "-IDIR                    Put DIR in the include file search path.\n"
        "-E                       Print preprocessor output on stdout.\n"
        "--output-dir DIR         Create files in the directory DIR.\n"
        "-d, --debug              Print debug messages.\n"
        "--depend                 Generate Makefile dependencies.\n"
        "--depend-xml             Generate dependencies in XML format.\n"
        "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
        "--validate               Validate command line options.\n"
        "--impl                   Generate sample implementations.\n"
        ;
}

int
compile(const vector<string>& argv)
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
    opts.addOpt("", "impl");
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceUtilInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();
    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.reason << endl;
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
        consoleErr << ICE_STRING_VERSION << endl;
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

    bool impl = opts.isSet("impl");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    if(args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
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

    ostringstream os;
    if(dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        vector<string>::iterator j = find(args.begin(), args.end(), *i);
        if(j != i)
        {
            continue;
        }

        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2CS__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit(false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if(parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(os, depend ? Preprocessor::CSharp : Preprocessor::SliceXML, includePaths,
                                                  "-D__SLICE2CS__"))
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
            FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2CS__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }
            if(preprocess)
            {
                char buf[4096];
                while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != nullptr)
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
                UnitPtr p = Unit::createUnit(false);
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
                        Gen gen(icecpp->getBaseName(), includePaths, output, impl);
                        gen.generate(p);
                        if(impl)
                        {
                            gen.generateImpl(p);
                        }
                    }
                    catch(const Slice::FileException& ex)
                    {
                        // If a file could not be created, then
                        // cleanup any created files.
                        FileTracker::instance()->cleanup();
                        p->destroy();
                        consoleErr << argv[0] << ": error: " << ex.reason() << endl;
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
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if(dependxml)
    {
        os << "</dependencies>\n";
    }

    if(depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    return status;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        consoleErr << args[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
