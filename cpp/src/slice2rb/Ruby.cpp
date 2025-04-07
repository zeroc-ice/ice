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
                      "-E                       Print preprocessor output on stdout.\n"
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
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "all");

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

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool all = opts.isSet("all");

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
        auto p = find(args.begin(), args.end(), *i);
        if (p != i)
        {
            continue;
        }

        if (depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2RB__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit("ruby", false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if (parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->printMakefileDependencies(
                    os,
                    depend ? Preprocessor::Ruby : Preprocessor::SliceXML,
                    includePaths,
                    "-D__SLICE2RB__"))
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
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2RB__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            if (preprocess)
            {
                char buf[4096];
                while (fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != nullptr)
                {
                    if (fputs(buf, stdout) == EOF)
                    {
                        return EXIT_FAILURE;
                    }
                }
                if (!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit("ruby", all);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if (!icecpp->close())
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }

                if (parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    string base = icecpp->getBaseName();
                    string::size_type pos = base.find_last_of("/\\");
                    if (pos != string::npos)
                    {
                        base.erase(0, pos + 1);
                    }

                    string file = base + ".rb";
                    if (!output.empty())
                    {
                        file = output + '/' + file;
                    }

                    try
                    {
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

                        //
                        // Generate the Ruby mapping.
                        //
                        generate(u, all, includePaths, out);

                        out.close();
                    }
                    catch (const Slice::FileException& ex)
                    {
                        // If a file could not be created, then cleanup
                        // any created files.
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.what() << endl;
                        return EXIT_FAILURE;
                    }
                }

                status |= u->getStatus();
                u->destroy();
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
