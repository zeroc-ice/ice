// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/ConsoleUtil.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <Gen.h>
#include <stdlib.h>

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

namespace
{

IceUtil::Mutex* globalMutex = 0;
bool interrupted = false;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

static vector<string>
splitCommas(string& str)
{
    vector<string> strings;
    size_t oldPos = 0;
    size_t commaPos = str.find(",");
    string token;

    while (commaPos != string::npos)
    {
        token = str.substr(oldPos, commaPos-oldPos);
        strings.push_back(token);

        oldPos = commaPos+1;
        commaPos = str.find(",", oldPos);
    }
    token = str.substr(oldPos);
    strings.push_back(token);

    return strings;
}

void
interruptedCallback(int)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

    interrupted = true;
}

void
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "--validate           Validate command line options.\n"
        "-DNAME               Define NAME as 1.\n"
        "-DNAME=DEF           Define NAME as DEF.\n"
        "-UNAME               Remove any definition for NAME.\n"
        "-IDIR                Put DIR in the include file search path.\n"
        "-E                   Print preprocessor output on stdout.\n"
        "--output-dir DIR     Create files in the directory DIR.\n"
        "--sort-order a,b,c   Define the sorting order for module navigation.\n"
        "--hdr FILE           Use the contents of FILE as the header.\n"
        "--ftr FILe           Use the contents of FILE as the footer.\n"
        "--indexhdr FILE      Use the contents of FILE as the header of the index/toc page (default=--hdr).\n"
        "--indexftr FILE      Use the contents of FILE as the footer of the index/toc page (default=--ftr).\n"
        "--image-dir DIR      Directory containing images for style sheets.\n"
        "--logo-url URL       Link to URL from logo image (requires --image-dir).\n"
        "--search ACTION      Generate search box with specified ACTION.\n"
        "--index NUM          Generate subindex if it has at least NUM entries (0 for no index, default=1).\n"
        "--summary NUM        Print a warning if a summary sentence exceeds NUM characters.\n"
        "-d, --debug          Print debug messages.\n"
        "--ice                Allow reserved Ice prefix in Slice identifiers\n"
        "                     deprecated: use instead [[\"ice-prefix\"]] metadata.\n"
        "--underscore         Allow underscores in Slice identifiers\n"
        "                     deprecated: use instead [[\"underscore\"]] metadata.\n"
        ;
}

int
compile(const vector<string>& argv)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg, ".");
    opts.addOpt("", "sort-order", IceUtilInternal::Options::NeedArg, ".");
    opts.addOpt("", "hdr", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "ftr", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "indexhdr", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "indexftr", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "index", IceUtilInternal::Options::NeedArg, "1");
    opts.addOpt("", "image-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "logo-url", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "search", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "summary", IceUtilInternal::Options::NeedArg, "0");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "underscore");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();
    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.reason << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    vector<string>::const_iterator i;
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-D" + *i);
    }

    optargs = opts.argVec("U");
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-U" + *i);
    }

    optargs = opts.argVec("I");
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    string sortorderstring = opts.optArg("sort-order");
    vector<string> sort_order = splitCommas(sortorderstring);

    string header = opts.optArg("hdr");

    string footer = opts.optArg("ftr");

    string indexHeader = opts.optArg("indexhdr");

    string indexFooter = opts.optArg("indexftr");

    string ind = opts.optArg("index");
    unsigned indexCount = 0;
    if(!ind.empty())
    {
        istringstream s(ind);
        s >>  indexCount;
        if(!s)
        {
            consoleErr << argv[0] << ": error: the --index operation requires a positive integer argument"
                       << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }
    }

    string imageDir = opts.optArg("image-dir");

    string logoURL = opts.optArg("logo-url");

    string searchAction = opts.optArg("search");

    string warnSummary = opts.optArg("summary");
    unsigned summaryCount = 0;
    if(!warnSummary.empty())
    {
        istringstream s(warnSummary);
        s >>  summaryCount;
        if(!s)
        {
            consoleErr << argv[0] << ": error: the --summary operation requires a positive integer argument"
                       << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }
    }

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool underscore = opts.isSet("underscore");

    if(args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(validate)
    {
        return EXIT_SUCCESS;
    }

    UnitPtr p = Unit::createUnit(true, false, ice, underscore);

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    for(vector<string>::size_type idx = 0; idx < args.size(); ++idx)
    {
        PreprocessorPtr icecpp = Preprocessor::create(argv[0], args[idx], cppArgs);
        FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2CONFLUENCE__");

        if(cppHandle == 0)
        {
            p->destroy();
            return EXIT_FAILURE;
        }
        if(preprocess)
        {
            char buf[4096];
            while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != ICE_NULLPTR)
            {
                if(fputs(buf, stdout) == EOF)
                {
                    p->destroy();
                    return EXIT_FAILURE;
                }
            }
        }
        else
        {
            status = p->parse(args[idx], cppHandle, debug);
        }

        if(!icecpp->close())
        {
            p->destroy();
            return EXIT_FAILURE;
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

            if(interrupted)
            {
                return EXIT_FAILURE;
            }
        }
    }

    if(status == EXIT_SUCCESS && !preprocess)
    {
        try
        {
            Slice::generate(p, output, header, footer, indexHeader, indexFooter, imageDir, logoURL,
                            searchAction, indexCount, summaryCount, sort_order);
        }
        catch(const Slice::FileException& ex)
        {
            // If a file could not be created, then cleanup any
            // created files.
            FileTracker::instance()->cleanup();
            p->destroy();
            consoleErr << argv[0] << ": error: " << ex.reason() << endl;
            return EXIT_FAILURE;
        }
        catch(...)
        {
            FileTracker::instance()->cleanup();
            consoleErr << args[0] << ": error:" << "unknown exception" << endl;
            status = EXIT_FAILURE;
        }
    }

    p->destroy();

    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

        if(interrupted)
        {
            FileTracker::instance()->cleanup();
            return EXIT_FAILURE;
        }
    }

    return status;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        consoleErr << args[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
