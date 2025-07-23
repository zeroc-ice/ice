// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Options.h"
#include "../Slice/DocCommentParser.h"
#include "../Slice/FileTracker.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Preprocessor.h"
#include "../Slice/Util.h"
#include "Gen.h"
#include "Ice/CtrlCHandler.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <mutex>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

using namespace std;
using namespace IceInternal;
using namespace Slice;

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
                      "--depend-xml             Generate dependencies in XML format.\n"
                      "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
                      "--validate               Validate command line options.\n"
                      "--all                    Generate code for Slice definitions in included files.\n"
                      "--list-generated         Emit list of generated files in XML format.\n";
    }

    void validateMatlabMetadata(const UnitPtr& unit)
    {
        map<string, MetadataInfo> knownMetadata;

        // "matlab:identifier"
        MetadataInfo identifierInfo = {
            .validOn =
                {typeid(Module),
                 typeid(InterfaceDecl),
                 typeid(Operation),
                 typeid(ClassDecl),
                 typeid(Slice::Exception),
                 typeid(Struct),
                 typeid(Sequence),
                 typeid(Dictionary),
                 typeid(Enum),
                 typeid(Enumerator),
                 typeid(Const),
                 typeid(Parameter),
                 typeid(DataMember)},
            .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        };
        knownMetadata.emplace("matlab:identifier", std::move(identifierInfo));

        // Pass this information off to the parser's metadata validation logic.
        Slice::validateMetadata(unit, "matlab", std::move(knownMetadata));
    }

    int compile(const vector<string>& argv)
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
        opts.addOpt("", "all");

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
        cppArgs.reserve(optargs.size()); // keeps clang-tidy happy
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

        string output = opts.optArg("output-dir");

        bool dependxml = opts.isSet("depend-xml");

        string dependFile = opts.optArg("depend-file");

        bool debug = opts.isSet("debug");

        bool all = opts.isSet("all");

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
            auto p = find(args.begin(), args.end(), *i);
            if (p != i)
            {
                continue;
            }

            if (dependxml)
            {
                PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
                FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2MATLAB__");

                if (cppHandle == nullptr)
                {
                    return EXIT_FAILURE;
                }

                UnitPtr u = Unit::createUnit("matlab", false);
                int parseStatus = u->parse(*i, cppHandle, debug);
                u->destroy();

                if (parseStatus == EXIT_FAILURE)
                {
                    return EXIT_FAILURE;
                }

                if (!icecpp->printMakefileDependencies(os, Preprocessor::SliceXML, includePaths, "-D__SLICE2MATLAB__"))
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
                FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2MATLAB__");

                if (cppHandle == nullptr)
                {
                    return EXIT_FAILURE;
                }

                UnitPtr u = Unit::createUnit("matlab", all);
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
                    parseAllDocComments(u, Slice::matlabLinkFormatter);

                    string base = icecpp->getBaseName();
                    string::size_type pos = base.find_last_of("/\\");
                    if (pos != string::npos)
                    {
                        base.erase(0, pos + 1);
                    }

                    try
                    {
                        validateMatlabMetadata(u);

                        CodeVisitor codeVisitor(output);
                        u->visit(&codeVisitor);
                    }
                    catch (const Slice::FileException& ex)
                    {
                        //
                        // If a file could not be created, then cleanup any created files.
                        //
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.what() << endl;
                        status = EXIT_FAILURE;
                        FileTracker::instance()->error();
                        break;
                    }
                }

                status |= u->getStatus();
                u->destroy();
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
