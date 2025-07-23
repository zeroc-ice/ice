// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Options.h"
#include "../Slice/DocCommentParser.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Preprocessor.h"
#include "../Slice/Util.h"
#include "Ice/CtrlCHandler.h"
#include "Ice/StringUtil.h"
#include "PythonUtil.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace Slice::Python;
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

    string getPackageInitPath(const string& packageName, const string& outputDir)
    {
        // Create a new output file for this package.
        string fileName = packageName;
        replace(fileName.begin(), fileName.end(), '.', '/');
        fileName += "/__init__.py";

        string outputPath;
        if (!outputDir.empty())
        {
            outputPath = outputDir + "/";
        }
        else
        {
            outputPath = "./";
        }

        createPackagePath(packageName, outputPath);
        outputPath += fileName;

        FileTracker::instance()->addFile(outputPath);

        return outputPath;
    }

    void usage(const string& n)
    {
        consoleErr << "Usage: " << n << " [options] slice-files...\n";
        consoleErr
            << "Options:\n"
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
               "--no-package             Do not generate Python package hierarchy.\n"
               "--build                  modules|index|all\n"
               "\n"
               "    Controls which types of Python files are generated from the Slice definitions.\n"
               "\n"
               "    modules  Generates only the Python module files for the Slice definitions.\n"
               "    index    Generates only the Python package index files (__init__.py).\n"
               "    all      Generates both module and index files (this is the default if --build is omitted).\n"
               "\n"
               "--list-generated         modules|index|all\n"
               "\n"
               "    Lists the Python files that would be generated for the given Slice definitions, without\n"
               "    producing any output files.\n"
               "\n"
               "    modules  Lists the Python module files generated from the Slice definitions.\n"
               "    index    Lists the Python package index files (__init__.py) that would be created.\n"
               "    all      Lists both module and index files.\n"
               "\n"
               "    All paths are relative to the directory specified with --output-dir.\n"
               "    Each file is listed on a separate line. No duplicates are included.\n";
    }
}

int
Slice::Python::staticCompile(const vector<string>& argv)
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
    opts.addOpt("", "build", IceInternal::Options::NeedArg, "all");
    opts.addOpt("", "list-generated", IceInternal::Options::NeedArg);

    vector<string> args;
    try
    {
        args = opts.parse(argv);
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

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    cppArgs.reserve(optargs.size());
    for (const auto& arg : optargs)
    {
        cppArgs.push_back("-D" + arg);
    }

    optargs = opts.argVec("U");
    for (const auto& arg : optargs)
    {
        cppArgs.push_back("-U" + arg);
    }

    vector<string> includePaths = opts.argVec("I");
    for (const auto& includePath : includePaths)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(includePath));
    }

    string outputDir = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    string buildArg = opts.optArg("build");

    string listArg = opts.optArg("list-generated");

    if (args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --dependxml" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (buildArg != "modules" && buildArg != "index" && buildArg != "all")
    {
        consoleErr << argv[0] << ": error: invalid argument for --build: " << buildArg << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (listArg != "modules" && listArg != "index" && listArg != "all" && !listArg.empty())
    {
        consoleErr << argv[0] << ": error: invalid argument for --list-generated: " << listArg << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!outputDir.empty() && !IceInternal::directoryExists(outputDir))
    {
        consoleErr << argv[0] << ": error: argument for --output-dir does not exist or is not a directory" << endl;
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    Ice::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if (dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    PackageVisitor packageVisitor;

    for (const auto& fileName : args)
    {
        PreprocessorPtr preprocessor = Preprocessor::create(argv[0], fileName, cppArgs);
        FILE* cppHandle = preprocessor->preprocess(true, "-D__SLICE2PY__");

        if (cppHandle == nullptr)
        {
            FileTracker::instance()->cleanup();
            return EXIT_FAILURE;
        }

        UnitPtr unit = Unit::createUnit("python", false);
        int parseStatus = unit->parse(fileName, cppHandle, debug);

        if (parseStatus == EXIT_FAILURE)
        {
            FileTracker::instance()->cleanup();
            unit->destroy();
            return EXIT_FAILURE;
        }

        if (depend || dependxml)
        {
            if (!preprocessor->printMakefileDependencies(
                    os,
                    depend ? Preprocessor::Python : Preprocessor::SliceXML,
                    includePaths,
                    "-D__SLICE2PY__"))
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            parseAllDocComments(unit, Slice::Python::pyLinkFormatter);

            try
            {
                if (buildArg == "modules" || buildArg == "all")
                {
                    // Generate Python code.
                    generate(unit, outputDir);
                }

                // Collect the package imports and generated files.
                unit->visit(&packageVisitor);
            }
            catch (const FileException&)
            {
                // If a file could not be created, then clean up any created files.
                FileTracker::instance()->cleanup();
                throw;
            }
        }

        if (!preprocessor->close())
        {
            return EXIT_FAILURE;
        }

        status |= unit->getStatus();
        unit->destroy();
    }

    if (status == EXIT_FAILURE)
    {
        // If the compilation failed, clean up any created files.
        FileTracker::instance()->cleanup();
        return status;
    }

    if (!listArg.empty())
    {
        if (listArg == "modules" || listArg == "all")
        {
            for (const auto& moduleName : packageVisitor.generatedModules())
            {
                cout << moduleName << endl;
            }
        }

        if (listArg == "index" || listArg == "all")
        {
            for (const auto& fileName : packageVisitor.packageIndexFiles())
            {
                cout << fileName << endl;
            }
        }
    }
    else if (buildArg == "index" || buildArg == "all")
    {
        // Emit the package index files.
        for (const auto& [packageName, imports] : packageVisitor.imports())
        {
            Output out{getPackageInitPath(packageName, outputDir).c_str()};
            writePackageIndex(imports, out);
        }
    }

    {
        lock_guard lock(globalMutex);
        if (interrupted)
        {
            FileTracker::instance()->cleanup();
            return EXIT_FAILURE;
        }
    }

    if (dependxml)
    {
        os << "</dependencies>\n";
    }

    if (depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    return status;
}
