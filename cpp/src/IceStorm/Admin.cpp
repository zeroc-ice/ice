// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "Ice/Ice.h"
#include "Parser.h"

#ifdef _WIN32
#    include <fcntl.h>
#    include <io.h>
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
        Ice::CtrlCHandler ctrlCHandler;

        auto properties = Ice::createProperties(argc, argv, "IceStormAdmin");
        properties->setProperty("Ice.Warn.Endpoints", "0");

        Ice::InitializationData initData{.properties = properties};
        auto communicator = Ice::initialize(std::move(initData));
        Ice::CommunicatorHolder ich(communicator);

        ctrlCHandler.setCallback([communicator](int) { communicator->destroy(); });

        status = run(communicator, Ice::argsToStringSeq(argc, argv));
    }
    catch (const std::exception& ex)
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
    consoleErr << "Options:\n"
                  "-h, --help           Show this message.\n"
                  "-v, --version        Display the Ice version.\n"
                  "-e COMMANDS          Execute COMMANDS.\n"
                  "-d, --debug          Print debug messages.\n";
}

int
run(const shared_ptr<Ice::Communicator>& communicator, const Ice::StringSeq& args)
{
    string commands;
    bool debug;

    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("e", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("d", "debug");

    try
    {
        if (!opts.parse(args).empty())
        {
            consoleErr << args[0] << ": too many arguments" << endl;
            usage(args[0]);
            return 1;
        }
    }
    catch (const IceInternal::BadOptException& e)
    {
        consoleErr << e.what() << endl;
        usage(args[0]);
        return 1;
    }

    if (opts.isSet("help"))
    {
        usage(args[0]);
        return 0;
    }
    if (opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        return 0;
    }
    if (opts.isSet("e"))
    {
        vector<string> optargs = opts.argVec("e");
        for (const auto& arg : optargs)
        {
            commands += arg + ";";
        }
    }
    debug = opts.isSet("debug");

    // The complete set of Ice::Identity -> manager proxies.
    map<Ice::Identity, IceStorm::TopicManagerPrx> managers;
    auto properties = communicator->getProperties();
    optional<IceStorm::TopicManagerPrx> defaultManager;

    auto props = communicator->getProperties()->getPropertiesForPrefix("IceStormAdmin.TopicManager.");
    {
        for (const auto& p : props)
        {
            //
            // Ignore proxy property settings. eg IceStormAdmin.TopicManager.*.LocatorCacheTimeout
            //
            if (p.first.find('.', strlen("IceStormAdmin.TopicManager.")) == string::npos)
            {
                try
                {
                    auto manager = communicator->propertyToProxy<IceStorm::TopicManagerPrx>(p.first);
                    assert(manager);
                    managers.insert({manager->ice_getIdentity(), *manager});
                }
                catch (const Ice::ParseException&)
                {
                    consoleErr << args[0] << ": malformed proxy: " << p.second << endl;
                    return 1;
                }
            }
        }

        string managerProxy = properties->getIceProperty("IceStormAdmin.TopicManager.Default");
        if (!managerProxy.empty())
        {
            defaultManager = IceStorm::TopicManagerPrx(communicator, managerProxy);
        }
        else if (!managers.empty())
        {
            defaultManager = managers.begin()->second;
        }
    }

    if (!defaultManager)
    {
        string host = properties->getIceProperty("IceStormAdmin.Host");
        string port = properties->getIceProperty("IceStormAdmin.Port");

        const int timeout = 3000; // 3s connection timeout.
        ostringstream os;
        os << "IceStorm/Finder";
        os << ":tcp" << (host.empty() ? "" : (" -h \"" + host + "\"")) << " -p " << port << " -t " << timeout;
        os << ":ssl" << (host.empty() ? "" : (" -h \"" + host + "\"")) << " -p " << port << " -t " << timeout;

        IceStorm::FinderPrx finder{communicator, os.str()};
        try
        {
            defaultManager = finder->getTopicManager();
        }
        catch (const Ice::LocalException&)
        {
            // Ignore.
        }
    }

    if (!defaultManager)
    {
        consoleErr << args[0] << ": no manager proxies configured" << endl;
        return 1;
    }

    IceStorm::Parser p(communicator, *defaultManager, managers);
    int status = 0;

    if (!commands.empty()) // Commands were given
    {
        int parseStatus = p.parse(commands, debug);
        if (parseStatus == 1)
        {
            status = 1;
        }
    }
    else // No commands, let's use standard input
    {
        p.showBanner();

        int parseStatus = p.parse(stdin, debug);
        if (parseStatus == 1)
        {
            status = 1;
        }
    }

    return status;
}
