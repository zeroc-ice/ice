// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "../Slice/DocCommentParser.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Preprocessor.h"
#include "../Slice/Util.h"
#include "Gen.h"
#include "Ice/CtrlCHandler.h"

#include <algorithm>
#include <iterator>
#include <mutex>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    mutex globalMutex;
    bool interrupted = false;
}

void
interruptedCallback(int /*signal*/)
{
    lock_guard lock(globalMutex);
    interrupted = true;
}

void
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr << "Options:\n"
                  "-h, --help               Show this message.\n"
                  "-v, --version            Display the Ice version.\n"
                  "-DNAME                   Define NAME as 1.\n"
                  "-DNAME=DEF               Define NAME as DEF.\n"
                  "-UNAME                   Remove any definition for NAME.\n"
                  "-IDIR                    Put DIR in the include file search path.\n"
                  "--output-dir DIR         Create files in the directory DIR.\n"
                  "-d, --debug              Print debug messages.\n"
                  "--depend-xml             Generate dependencies in XML format.\n"
                  "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
                  "--validate               Validate command line options.\n"
                  "--list-generated         Emit list of generated files in XML format.\n";
}

int
compile(const vector<string>& argv)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("U", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("I", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("", "output-dir", IceInternal::Options::NeedArg);
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceInternal::Options::NeedArg, "");
    opts.addOpt("", "list-generated");
    opts.addOpt("d", "debug");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();
    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.what() << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    cppArgs.reserve(optargs.size()); // not quite sufficient but keeps clang-tidy happy
    for (const auto& optarg : optargs)
    {
        cppArgs.push_back("-D" + optarg);
    }

    optargs = opts.argVec("U");
    for (const auto& optarg : optargs)
    {
        cppArgs.push_back("-U" + optarg);
    }

    vector<string> includePaths = opts.argVec("I");
    for (const auto& includePath : includePaths)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(includePath));
    }

    string output = opts.optArg("output-dir");

    bool dependxml = opts.isSet("depend-xml");
    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool listGenerated = opts.isSet("list-generated");

    if (args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (validate)
    {
        return EXIT_SUCCESS;
    }

    int status = EXIT_SUCCESS;

    Ice::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if (dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for (auto i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        auto j = find(args.begin(), args.end(), *i);
        if (j != i)
        {
            continue;
        }

        if (dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2JAVA__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit("java", false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if (parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->printMakefileDependencies(os, Preprocessor::SliceXML, includePaths, "-D__SLICE2JAVA__"))
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            FileTracker::instance()->setSource(*i);

            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2JAVA__");

            if (cppHandle == nullptr)
            {
                FileTracker::instance()->error();
                status = EXIT_FAILURE;
                break;
            }

            UnitPtr p = Unit::createUnit("java", false);
            int parseStatus = p->parse(*i, cppHandle, debug);

            if (!icecpp->close())
            {
                p->destroy();
                FileTracker::instance()->error();
                return EXIT_FAILURE;
            }

            if (parseStatus == EXIT_FAILURE)
            {
                p->destroy();
                status = EXIT_FAILURE;
            }
            else
            {
                parseAllDocComments(p, Slice::Java::javaLinkFormatter);

                try
                {
                    Gen gen(icecpp->getBaseName(), includePaths, output);
                    gen.generate(p);
                }
                catch (const Slice::FileException& ex)
                {
                    //
                    // If a file could not be created then cleanup any files we've already created.
                    //
                    FileTracker::instance()->cleanup();
                    p->destroy();
                    consoleErr << argv[0] << ": error: " << ex.what() << endl;
                    status = EXIT_FAILURE;
                    FileTracker::instance()->error();
                    break;
                }
            }

            status |= p->getStatus();
            p->destroy();
        }

        {
            lock_guard lock(globalMutex);
            if (interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if (dependxml)
    {
        os << "</dependencies>\n";
        writeDependencies(os.str(), dependFile);
    }

    if (listGenerated)
    {
        FileTracker::instance()->dumpxml();
    }

    return status;
}

#ifdef _WIN32
int
wmain(int argc, wchar_t* argv[])
#else
int
main(int argc, char* argv[])
#endif
{
    vector<string> args = argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch (const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        consoleErr << args[0] << ": error:"
                   << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
