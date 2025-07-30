// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Options.h"
#include "../Slice/DocCommentParser.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Preprocessor.h"
#include "../Slice/Util.h"
#include "Ice/CtrlCHandler.h"
#include "PythonUtil.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <mutex>
#include <string>

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

        vector<string> sliceFiles;
        try
        {
            // The non-option arguments are the Slice files.
            sliceFiles = opts.parse(args);
        }
        catch (const IceInternal::BadOptException& e)
        {
            consoleErr << args[0] << ": error: " << e.what() << endl;
            usage(args[0]);
            return EXIT_FAILURE;
        }

        if (opts.isSet("help"))
        {
            usage(args[0]);
            return EXIT_SUCCESS;
        }

        if (opts.isSet("version"))
        {
            consoleErr << ICE_STRING_VERSION << endl;
            return EXIT_SUCCESS;
        }

        vector<string> preprocessorArgs;
        vector<string> optargs = opts.argVec("D");
        preprocessorArgs.reserve(optargs.size());
        for (const auto& arg : optargs)
        {
            preprocessorArgs.push_back("-D" + arg);
        }

        optargs = opts.argVec("U");
        for (const auto& arg : optargs)
        {
            preprocessorArgs.push_back("-U" + arg);
        }

        vector<string> includePaths = opts.argVec("I");
        for (const auto& includePath : includePaths)
        {
            preprocessorArgs.push_back("-I" + Preprocessor::normalizeIncludePath(includePath));
        }

        string outputDir = opts.optArg("output-dir");

        bool depend = opts.isSet("depend");

        bool dependXML = opts.isSet("depend-xml");

        string dependFile = opts.optArg("depend-file");

        bool debug = opts.isSet("debug");

        string buildArg = opts.optArg("build");

        string listArg = opts.optArg("list-generated");

        if (sliceFiles.empty())
        {
            consoleErr << args[0] << ": error: no input file" << endl;
            usage(args[0]);
            return EXIT_FAILURE;
        }

        if (depend && dependXML)
        {
            consoleErr << args[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
            usage(args[0]);
            return EXIT_FAILURE;
        }

        if (buildArg != "modules" && buildArg != "index" && buildArg != "all")
        {
            consoleErr << args[0] << ": error: invalid argument for --build: " << buildArg << endl;
            usage(args[0]);
            return EXIT_FAILURE;
        }

        if (listArg != "modules" && listArg != "index" && listArg != "all" && !listArg.empty())
        {
            consoleErr << args[0] << ": error: invalid argument for --list-generated: " << listArg << endl;
            usage(args[0]);
            return EXIT_FAILURE;
        }

        if (!outputDir.empty() && !IceInternal::directoryExists(outputDir))
        {
            consoleErr << args[0] << ": error: argument for --output-dir does not exist or is not a directory" << endl;
            return EXIT_FAILURE;
        }

        int status = EXIT_SUCCESS;

        Ice::CtrlCHandler ctrlCHandler;
        ctrlCHandler.setCallback(interruptedCallback);

        DependencyGenerator dependencyGenerator;
        PackageVisitor packageVisitor;

        std::map<string, StringList> dependencyMap;

        for (const auto& fileName : sliceFiles)
        {
            PreprocessorPtr preprocessor;
            UnitPtr unit;
            try
            {
                preprocessor = Preprocessor::create(args[0], fileName, preprocessorArgs);
                FILE* preprocessedHandle = preprocessor->preprocess("-D__SLICE2PY__");
                assert(preprocessedHandle);

                unit = Unit::createUnit("python", false);
                int parseStatus = unit->parse(fileName, preprocessedHandle, debug);

                preprocessor->close();

                if (parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    // Collect the dependencies of the unit.
                    dependencyGenerator.addDependenciesFor(unit);

                    // Collect the package imports and generated files.
                    unit->visit(&packageVisitor);

                    if (buildArg == "modules" || buildArg == "all")
                    {
                        parseAllDocComments(unit, Slice::Python::pyLinkFormatter);

                        generate(unit, outputDir);
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
        }

        if (status == EXIT_FAILURE)
        {
            // If the compilation failed, clean up any created files.
            FileTracker::instance()->cleanup();
            return status;
        }

        if (depend)
        {
            for (const auto& [source, files] : packageVisitor.generated())
            {
                for (const auto& file : files)
                {
                    dependencyGenerator.writeMakefileDependencies(dependFile, source, file);
                }
            }
        }
        else if (dependXML)
        {
            dependencyGenerator.writeXMLDependencies(dependFile);
        }
        else if (!listArg.empty())
        {
            std::set<string> generated;

            for (const auto& [source, files] : packageVisitor.generated())
            {
                for (const auto& file : files)
                {
                    bool skip = (listArg == "modules" && file.find("/__init__.py") != string::npos) ||
                                (listArg == "index" && file.find("/__init__.py") == string::npos);

                    if (skip)
                    {
                        continue;
                    }
                    generated.insert(file);
                }
            }

            for (const auto& file : generated)
            {
                cout << file << endl;
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

        return status;
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
