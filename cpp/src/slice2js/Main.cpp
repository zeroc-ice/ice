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
#include <iostream>
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
                  "--depend                 Generate Makefile dependencies.\n"
                  "--depend-xml             Generate dependencies in XML format.\n"
                  "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
                  "--validate               Validate command line options.\n"
                  "--stdout                 Print generated code to stdout.\n"
                  "--typescript             Generate TypeScript declarations.\n"
                  "--depend-json            Generate dependency information in JSON format.\n";
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
    opts.addOpt("", "stdout");
    opts.addOpt("", "typescript");
    opts.addOpt("", "output-dir", IceInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-json");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceInternal::Options::NeedArg, "");
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

    vector<string> preprocessorArgs;
    for (const string& arg : opts.argVec("D"))
    {
        preprocessorArgs.push_back("-D" + arg);
    }

    for (const string& arg : opts.argVec("U"))
    {
        preprocessorArgs.push_back("-U" + arg);
    }

    vector<string> includePaths = opts.argVec("I");
    for (const string& path : includePaths)
    {
        preprocessorArgs.push_back("-I" + Preprocessor::normalizeIncludePath(path));
    }

    bool useStdout = opts.isSet("stdout");

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependJSON = opts.isSet("depend-json");

    bool dependXml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool typeScript = opts.isSet("typescript");

    if (args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (depend && dependJSON)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-json" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (depend && dependXml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (dependXml && dependJSON)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend-xml and --depend-json" << endl;
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
    if (dependJSON)
    {
        os << "{" << endl;
    }
    else if (dependXml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    //
    // Create a copy of args without the duplicates.
    //
    set<string> sources(args.begin(), args.end());

    for (auto i = sources.cbegin(); i != sources.cend();)
    {
        PreprocessorPtr preprocessor = Preprocessor::create(argv[0], *i, preprocessorArgs);
        FILE* cppHandle = preprocessor->preprocess(true, "-D__SLICE2JS__");

        if (cppHandle == nullptr)
        {
            return EXIT_FAILURE;
        }

        if (depend || dependJSON || dependXml)
        {
            UnitPtr u = Unit::createUnit("js", false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if (parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            bool last = (++i == sources.cend());

            if (!preprocessor->printMakefileDependencies(
                    os,
                    depend ? Preprocessor::JavaScript
                           : (dependJSON ? Preprocessor::JavaScriptJSON : Preprocessor::SliceXML),
                    includePaths,
                    "-D__SLICE2JS__"))
            {
                return EXIT_FAILURE;
            }

            if (!preprocessor->close())
            {
                return EXIT_FAILURE;
            }

            if (dependJSON)
            {
                if (!last)
                {
                    os << ",";
                }
                os << "\n";
            }
        }
        else
        {
            UnitPtr p = Unit::createUnit("js", false);
            int parseStatus = p->parse(*i, cppHandle, debug);

            if (!preprocessor->close())
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
                parseAllDocComments(p, Slice::JavaScript::jsLinkFormatter);

                DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
                assert(dc);
                try
                {
                    if (useStdout)
                    {
                        Gen gen(preprocessor->getBaseName(), includePaths, output, typeScript, cout);
                        gen.generate(p);
                    }
                    else
                    {
                        Gen gen(preprocessor->getBaseName(), includePaths, output, typeScript);
                        gen.generate(p);
                    }
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

    if (dependJSON)
    {
        os << "}\n";
    }
    else if (dependXml)
    {
        os << "</dependencies>\n";
    }

    if (depend || dependJSON || dependXml)
    {
        writeDependencies(os.str(), dependFile);
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
        consoleErr << args[0] << ": error:unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
