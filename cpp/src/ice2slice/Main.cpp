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

class Slice2DocCommentFormatter final : public DocCommentFormatter
{
    string formatLink(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr&) final
    {
        // The only difference with '@link' between the 'Ice' and 'Slice' syntaxes
        // is that the 'Ice' syntax uses '#' whereas the 'Slice' syntax uses '::'.
        string formattedLink = rawLink;
        auto separatorPos = formattedLink.find('#');
        if (separatorPos == 0)
        {
            // We want to avoid converting the relative link '#member' into the global link '::member'.
            // Instead we simply convert it to 'member' with no prefix.
            formattedLink = formattedLink.substr(1);
        }
        else if (separatorPos != string::npos)
        {
            formattedLink.replace(separatorPos, 1, "::");
        }
        return "{@link " + formattedLink + "}";
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
    vector<string> optargs = opts.argVec("D");
    preprocessorArgs.reserve(optargs.size()); // not quite sufficient but keeps clang-tidy happy
    for (const auto& optarg : optargs)
    {
        preprocessorArgs.push_back("-D" + optarg);
    }

    optargs = opts.argVec("U");
    for (const auto& optarg : optargs)
    {
        preprocessorArgs.push_back("-U" + optarg);
    }

    vector<string> includePaths = opts.argVec("I");
    for (const auto& includePath : includePaths)
    {
        preprocessorArgs.push_back("-I" + Preprocessor::normalizeIncludePath(includePath));
    }

    string outputDir = opts.optArg("output-dir");
    if (outputDir.empty())
    {
        outputDir = ".";
    }

    bool debug = opts.isSet("debug");

    if (sliceFiles.empty())
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

    for (const auto& fileName : sliceFiles)
    {
        UnitPtr unit;
        try
        {
            PreprocessorPtr preprocessor = Preprocessor::create(argv[0], fileName, preprocessorArgs);
            FILE* preprocessedHandle = preprocessor->preprocess("-D__ICE2SLICE__");
            assert(preprocessedHandle);

            unit = Unit::createUnit("", false);
            int parseStatus = unit->parse(fileName, preprocessedHandle, debug);

            preprocessor->close();

            if (parseStatus == EXIT_FAILURE)
            {
                status = EXIT_FAILURE;
            }
            else
            {
                Slice2DocCommentFormatter formatter;
                parseAllDocComments(unit, formatter);

                DefinitionContextPtr dc = unit->findDefinitionContext(unit->topLevelFile());
                assert(dc);

                string baseName = preprocessor->getBaseName();
                // Remove any directory components from the base name.
                string::size_type pos = baseName.find_last_of("/\\");
                if (pos != string::npos)
                {
                    baseName = baseName.substr(pos);
                }

                Gen gen(outputDir + baseName);
                gen.generate(unit);
            }

            status |= unit->getStatus();
            unit->destroy();
        }
        catch (...)
        {
            FileTracker::instance()->cleanup();
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
        consoleErr << args[0] << ": error:unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
