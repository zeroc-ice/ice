// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "Ice/Ice.h"
#include "IceBox/IceBox.h"

using namespace std;
using namespace IceInternal;

int run(const Ice::CommunicatorPtr&, const Ice::StringSeq&);

int
#ifdef _WIN32
wmain(int argc, wchar_t* argv[])
#else
main(int argc, char* argv[])
#endif
{
    int status = 0;

    try
    {
        Ice::CtrlCHandler ctrlCHandler;

        // Initialize with a Properties object with the correct property prefix enabled.
        auto properties =
            Ice::createProperties(argc, argv, make_shared<Ice::Properties>(vector<string>{"IceBoxAdmin"}));
        Ice::InitializationData initData{.properties = properties};
        auto communicator = Ice::initialize(std::move(initData));
        Ice::CommunicatorHolder ich{communicator};
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
    consoleErr << "Usage: " << name << " [options] [command...]\n";
    consoleErr << "Options:\n"
                  "-h, --help           Show this message.\n"
                  "-v, --version        Display the Ice version.\n"
                  "\n"
                  "Commands:\n"
                  "start SERVICE        Start a service.\n"
                  "stop SERVICE         Stop a service.\n"
                  "shutdown             Shutdown the server.\n";
}

int
run(const Ice::CommunicatorPtr& communicator, const Ice::StringSeq& args)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> commands;
    try
    {
        commands = opts.parse(args);
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

    if (commands.empty())
    {
        usage(args[0]);
        return 1;
    }

    auto manager = communicator->propertyToProxy<IceBox::ServiceManagerPrx>("IceBoxAdmin.ServiceManager.Proxy");
    if (!manager)
    {
        consoleErr << args[0] << ": property 'IceBoxAdmin.ServiceManager.Proxy' is not set" << endl;
        return 1;
    }

    for (auto r = commands.begin(); r != commands.end(); ++r)
    {
        if ((*r) == "shutdown")
        {
            manager->shutdown();
        }
        else if ((*r) == "start")
        {
            if (++r == commands.end())
            {
                consoleErr << args[0] << ": no service name specified." << endl;
                return 1;
            }

            try
            {
                manager->startService(*r);
            }
            catch (const IceBox::NoSuchServiceException&)
            {
                consoleErr << args[0] << ": unknown service '" << *r << "'" << endl;
                return 1;
            }
            catch (const IceBox::AlreadyStartedException&)
            {
                consoleErr << args[0] << ": service already started." << endl;
            }
        }
        else if ((*r) == "stop")
        {
            if (++r == commands.end())
            {
                consoleErr << args[0] << ": no service name specified." << endl;
                return 1;
            }

            try
            {
                manager->stopService(*r);
            }
            catch (const IceBox::NoSuchServiceException&)
            {
                consoleErr << args[0] << ": unknown service '" << *r << "'" << endl;
                return 1;
            }
            catch (const IceBox::AlreadyStoppedException&)
            {
                consoleErr << args[0] << ": service already stopped." << endl;
            }
        }
        else
        {
            consoleErr << args[0] << ": unknown command '" << *r << "'" << endl;
            usage(args[0]);
            return 1;
        }
    }

    return 0;
}
