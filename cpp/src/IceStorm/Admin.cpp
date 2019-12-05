//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/ConsoleUtil.h>
#include <IceStorm/Parser.h>

#ifdef  _WIN32
#   include <fcntl.h>
#   include <io.h>
#endif

using namespace std;
using namespace IceInternal;

int run(const shared_ptr<Ice::Communicator>&, const Ice::StringSeq&);

int
#ifdef _WIN32
wmain(int argc, wchar_t* argv[])
{
    //
    // Enable binary input mode for stdin to avoid automatic conversions.
    //
    _setmode(_fileno(stdin), _O_BINARY);
#else
main(int argc, char* argv[])
{
#endif
    int status = 0;

    try
    {
        IceUtil::CtrlCHandler ctrlCHandler;

        Ice::InitializationData id;
        Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
        id.properties = Ice::createProperties(args);
        id.properties->setProperty("Ice.Warn.Endpoints", "0");

        Ice::CommunicatorHolder ich(id);
        auto communicator = ich.communicator();

        ctrlCHandler.setCallback([communicator](int) { communicator->destroy(); });

        status = run(communicator, args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << ex.what() << endl;
        status = 1;
    }

    return status;
}

void
usage(const string& name)
{
    consoleErr << "Usage: " << name << " [options]\n";
    consoleErr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-e COMMANDS          Execute COMMANDS.\n"
        "-d, --debug          Print debug messages.\n"
        ;
}

int
run(const shared_ptr<Ice::Communicator>& communicator, const Ice::StringSeq& args)
{
    string commands;
    bool debug;

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("e", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("d", "debug");

    try
    {
        if(!opts.parse(args).empty())
        {
            consoleErr << args[0] << ": too many arguments" << endl;
            usage(args[0]);
            return 1;
        }
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << e.reason << endl;
        usage(args[0]);
        return 1;
    }

    if(opts.isSet("help"))
    {
        usage(args[0]);
        return 0;
    }
    if(opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        return 0;
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
    map<Ice::Identity, shared_ptr<IceStorm::TopicManagerPrx>> managers;
    auto properties = communicator->getProperties();
    shared_ptr<IceStorm::TopicManagerPrx> defaultManager;

    auto props = communicator->getProperties()->getPropertiesForPrefix("IceStormAdmin.TopicManager.");
    {
        for(const auto& p : props)
        {
            //
            // Ignore proxy property settings. eg IceStormAdmin.TopicManager.*.LocatorCacheTimeout
            //
            if(p.first.find('.', strlen("IceStormAdmin.TopicManager.")) == string::npos)
            {
                try
                {
                    auto manager = Ice::uncheckedCast<IceStorm::TopicManagerPrx>(
                        communicator->propertyToProxy(p.first));
                    managers.insert({manager->ice_getIdentity(), manager});
                }
                catch(const Ice::ProxyParseException&)
                {
                    consoleErr << args[0] << ": malformed proxy: " << p.second << endl;
                    return 1;
                }
            }
        }

        string managerProxy = properties->getProperty("IceStormAdmin.TopicManager.Default");
        if(!managerProxy.empty())
        {
            defaultManager = Ice::uncheckedCast<IceStorm::TopicManagerPrx>(communicator->stringToProxy(managerProxy));
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
        auto finder = Ice::uncheckedCast<IceStorm::FinderPrx>(communicator->stringToProxy(os.str()));
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
        consoleErr << args[0] << ": no manager proxies configured" << endl;
        return 1;
    }

    IceStorm::Parser p(communicator, defaultManager, managers);
    int status = 0;

    if(!commands.empty()) // Commands were given
    {
        int parseStatus = p.parse(commands, debug);
        if(parseStatus == 1)
        {
            status = 1;
        }
    }
    else // No commands, let's use standard input
    {
        p.showBanner();

        int parseStatus = p.parse(stdin, debug);
        if(parseStatus == 1)
        {
            status = 1;
        }
    }

    return status;
}
