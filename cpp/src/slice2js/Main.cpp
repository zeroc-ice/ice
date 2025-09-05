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

class JSDocCommentFormatter final : public DocCommentFormatter
{
    string formatLink(const string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target) final
    {
        return Slice::JavaScript::jsLinkFormatter(rawLink, source, target);
    }
};

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

    vector<string> sliceFiles;
    try
    {
        sliceFiles = opts.parse(argv);
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

    bool dependXML = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool typeScript = opts.isSet("typescript");

    if (sliceFiles.empty())
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

    if (depend && dependXML)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (dependXML && dependJSON)
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

    DependencyGenerator dependencyGenerator;

    for (const auto& fileName : sliceFiles)
    {
        PreprocessorPtr preprocessor;
        UnitPtr unit;
        try
        {
            preprocessor = Preprocessor::create(argv[0], fileName, preprocessorArgs);
            FILE* preprocessedHandle = preprocessor->preprocess("-D__SLICE2JS__");
            if (preprocessedHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            unit = Unit::createUnit("js", false);
            int parseStatus = unit->parse(fileName, preprocessedHandle, debug);

            preprocessor->close();

            if (parseStatus == EXIT_FAILURE)
            {
                status = EXIT_FAILURE;
            }
            else if (depend || dependJSON || dependXML)
            {
                dependencyGenerator.addDependenciesFor(unit);
                if (depend)
                {
                    string target = removeExtension(baseName(fileName)) + ".js";
                    dependencyGenerator.writeMakefileDependencies(dependFile, unit->topLevelFile(), target);
                }
                // Else JSON and XML dependencies are written below after all units have been processed.
            }
            else
            {
                JSDocCommentFormatter formatter;
                parseAllDocComments(unit, formatter);

                if (useStdout)
                {
                    Gen gen(preprocessor->getBaseName(), includePaths, output, typeScript, cout);
                    gen.generate(unit);
                }
                else
                {
                    Gen gen(preprocessor->getBaseName(), includePaths, output, typeScript);
                    gen.generate(unit);
                }

                status |= unit->getStatus();
            }
            unit->destroy();
        }
        catch (...)
        {
            FileTracker::instance()->cleanup();

            if (preprocessor)
            {
                preprocessor->close();
            }

            if (unit)
            {
                unit->destroy();
            }
            throw;
        }

        {
            lock_guard lock(globalMutex);
            if (interrupted)
            {
                status = EXIT_FAILURE;
                break;
            }
        }
    }

    if (status == EXIT_FAILURE)
    {
        // If the compilation failed, clean up any created files.
        FileTracker::instance()->cleanup();
        return status;
    }

    if (dependJSON)
    {
        dependencyGenerator.writeJSONDependencies(dependFile);
    }
    else if (dependXML)
    {
        dependencyGenerator.writeXMLDependencies(dependFile);
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
