// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Preprocessor.h"
#include "../Slice/Util.h"
#include "Ice/CtrlCHandler.h"
#include "Ice/StringUtil.h"
#include "RubyUtil.h"

#include <algorithm>
#include <cassert>
#include <mutex>

#include <cstring>

using namespace std;
using namespace Slice;
using namespace Slice::Ruby;
using namespace IceInternal;

namespace
{
    mutex globalMutex;
    bool interrupted = false;

    void interruptedCallback(int /*signal*/)
    {
        lock_guard lock(globalMutex);
        interrupted = true;
    }

    void usage(const string& n)
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
                      "--all                    Generate code for Slice definitions in included files.\n";
    }
}

int
Slice::Ruby::compile(const vector<string>& argv)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("D", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("U", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("I", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("", "output-dir", IceInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "all");

    vector<string> sliceFiles;
    try
    {
        sliceFiles = opts.parse(argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.what() << endl;
        usage(argv[0]);
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

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependXML = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool all = opts.isSet("all");

    if (sliceFiles.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (depend && dependXML)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    Ice::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    DependencyGenerator dependencyGenerator;

    for (const auto& fileName : sliceFiles)
    {
        UnitPtr unit;
        PreprocessorPtr preprocessor;
        try
        {
            preprocessor = Preprocessor::create(argv[0], fileName, preprocessorArgs);
            FILE* preprocessedHandle = preprocessor->preprocess("-D__SLICE2RB__");
            if (preprocessedHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            unit = Unit::createUnit("ruby", all);
            int parseStatus = unit->parse(fileName, preprocessedHandle, debug);

            preprocessor->close();

            if (parseStatus == EXIT_FAILURE)
            {
                status = EXIT_FAILURE;
            }
            else if (depend || dependXML)
            {
                dependencyGenerator.addDependenciesFor(unit);
                if (depend)
                {
                    string target = removeExtension(baseName(fileName)) + ".rb";
                    dependencyGenerator.writeMakefileDependencies(dependFile, unit->topLevelFile(), target);
                }
                // Else XML dependencies are written below after all units have been processed.
            }
            else
            {
                string base = removeExtension(baseName(fileName));

                string file = base + ".rb";
                if (!output.empty())
                {
                    file = output + '/' + file;
                }

                IceInternal::Output out;
                out.open(file.c_str());
                if (!out)
                {
                    ostringstream oss;
                    oss << "cannot open '" << file << "': " << IceInternal::errorToString(errno);
                    throw FileException(oss.str());
                }
                FileTracker::instance()->addFile(file);

                printHeader(out);
                printGeneratedHeader(out, base + ".ice", "#");
                out << sp;

                // Generate the Ruby mapping.
                generate(unit, all, includePaths, out);

                out.close();

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

    if (dependXML)
    {
        dependencyGenerator.writeXMLDependencies(dependFile);
    }

    return status;
}
