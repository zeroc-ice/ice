// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Application.h>
#include <Ice/ConsoleUtil.h>
#include <Ice/SliceChecksums.h>
#include <IceStorm/Parser.h>

#ifdef  _WIN32
#   include <fcntl.h>
#   include <io.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceStorm;

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

#ifdef _WIN32

int
wmain(int argc, wchar_t* argv[])
{
    //
    // Enable binary input mode for stdin to avoid automatic conversions.
    //
    _setmode(_fileno(stdin), _O_BINARY);
#else

int
main(int argc, char* argv[])
{
#endif
    Client app;
    Ice::InitializationData id;
    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    id.properties = Ice::createProperties(args);
    id.properties->setProperty("Ice.Warn.Endpoints", "0");
    int rc = app.main(argc, argv, id);
    return rc;
}

void
Client::usage()
{
    consoleErr << "Usage: " << appName() << " [options]\n";
    consoleErr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-e COMMANDS          Execute COMMANDS.\n"
        "-d, --debug          Print debug messages.\n"
        ;
}

int
Client::run(int argc, char* argv[])
{
    string commands;
    bool debug;

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("e", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("d", "debug");

    vector<string> args;
    try
    {
        args = opts.parse(argc, const_cast<const char**>(argv));
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << e.reason << endl;
        usage();
        return EXIT_FAILURE;
    }
    if(!args.empty())
    {
        consoleErr << argv[0] << ": too many arguments" << endl;
        usage();
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage();
        return EXIT_SUCCESS;
    }
    if(opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }
    if(opts.isSet("e"))
    {
        vector<string> optargs = opts.argVec("e");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            commands += *i + ";";
        }
    }
    debug = opts.isSet("debug");

    // The complete set of Ice::Identity -> manager proxies.
    map<Ice::Identity, IceStorm::TopicManagerPrx> managers;
    PropertiesPtr properties = communicator()->getProperties();
    IceStorm::TopicManagerPrx defaultManager;

    Ice::PropertyDict props = communicator()->getProperties()->getPropertiesForPrefix("IceStormAdmin.TopicManager.");
    {
        for(Ice::PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
        {
            //
            // Ignore proxy property settings. eg IceStormAdmin.TopicManager.*.LocatorCacheTimeout
            //
            if(p->first.find('.', strlen("IceStormAdmin.TopicManager.")) == string::npos)
            {
                try
                {
                    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::uncheckedCast(
                        communicator()->propertyToProxy(p->first));
                    managers.insert(map<Ice::Identity, IceStorm::TopicManagerPrx>::value_type(
                                        manager->ice_getIdentity(), manager));
                }
                catch(const Ice::ProxyParseException&)
                {
                    consoleErr << appName() << ": malformed proxy: " << p->second << endl;
                    return EXIT_FAILURE;
                }
            }
        }

        string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
        if(!managerProxy.empty())
        {
            defaultManager = IceStorm::TopicManagerPrx::uncheckedCast(
                communicator()->stringToProxy(managerProxy));
        }
        else if(!managers.empty())
        {
            defaultManager = managers.begin()->second;
        }
    }

    if(!defaultManager)
    {
        string host = properties->getProperty("IceStormAdmin.Host");
        string port = properties->getProperty("IceStormAdmin.Port");

        const int timeout = 3000; // 3s connection timeout.
        ostringstream os;
        os << "IceStorm/Finder";
        os << ":tcp" << (host.empty() ? "" : (" -h \"" + host + "\"")) << " -p " << port << " -t " << timeout;
        os << ":ssl" << (host.empty() ? "" : (" -h \"" + host + "\"")) << " -p " << port << " -t " << timeout;
        IceStorm::FinderPrx finder = IceStorm::FinderPrx::uncheckedCast(communicator()->stringToProxy(os.str()));
        try
        {
            defaultManager = finder->getTopicManager();
        }
        catch(const Ice::LocalException&)
        {
            // Ignore.
        }
    }

    if(!defaultManager)
    {
        consoleErr << appName() << ": no manager proxies configured" << endl;
        return EXIT_FAILURE;
    }

    ParserPtr p = Parser::createParser(communicator(), defaultManager, managers);
    int status = EXIT_SUCCESS;

    if(!commands.empty()) // Commands were given
    {
        int parseStatus = p->parse(commands, debug);
        if(parseStatus == EXIT_FAILURE)
        {
            status = EXIT_FAILURE;
        }
    }
    else // No commands, let's use standard input
    {
        p->showBanner();

        int parseStatus = p->parse(stdin, debug);
        if(parseStatus == EXIT_FAILURE)
        {
            status = EXIT_FAILURE;
        }
    }

    return status;
}
