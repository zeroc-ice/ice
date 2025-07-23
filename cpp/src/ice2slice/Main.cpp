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
#include <cassert>
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
                  "--validate               Validate command line options.\n";
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

    string outputDir = opts.optArg("output-dir");
    if (outputDir.empty())
    {
        outputDir = ".";
    }

    bool debug = opts.isSet("debug");

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

    // Create a copy of args without the duplicates.
    vector<string> sources;
    for (const auto& arg : args)
    {
        auto p = find(sources.begin(), sources.end(), arg);
        if (p == sources.end())
        {
            sources.push_back(arg);
        }
    }

    for (auto i = sources.begin(); i != sources.end();)
    {
        PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
        FILE* cppHandle = icecpp->preprocess(true, "-D__ICE2SLICE__");

        if (cppHandle == nullptr)
        {
            return EXIT_FAILURE;
        }

        UnitPtr p = Unit::createUnit("", false);
        int parseStatus = p->parse(*i, cppHandle, debug);

        if (!icecpp->close())
        {
            p->destroy();
            return EXIT_FAILURE;
        }

        if (parseStatus == EXIT_FAILURE)
        {
            status = EXIT_FAILURE;
        }
        else
        {
            parseAllDocComments(p, Slice::slice2LinkFormatter);

            DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
            assert(dc);

            string baseName = icecpp->getBaseName();
            // Remove any directory components from the base name.
            string::size_type pos = baseName.find_last_of("/\\");
            if (pos != string::npos)
            {
                baseName = baseName.substr(pos);
            }

            try
            {
                Gen gen(outputDir + baseName);
                gen.generate(p);
            }
            catch (const Slice::FileException& ex)
            {
                //
                // If a file could not be created, then clean up any created files.
                //
                FileTracker::instance()->cleanup();
                p->destroy();
                consoleErr << argv[0] << ": error: " << ex.what() << endl;
                return EXIT_FAILURE;
            }
        }

        status |= p->getStatus();
        p->destroy();
        ++i;

        {
            lock_guard lock(globalMutex);
            if (interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
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
    vector<string> args = Slice::argvToArgs(argc, argv);
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
