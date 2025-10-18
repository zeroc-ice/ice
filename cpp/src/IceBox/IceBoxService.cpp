// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "../Ice/Options.h"
#include "ServiceManagerI.h"

using namespace std;
using namespace Ice;
using namespace IceBox;

namespace IceBox
{
    class IceBoxService : public Ice::Service
    {
    protected:
        bool start(int, char*[], int&) final;
        bool stop() final;

    private:
        void usage(const std::string&);
        ServiceManagerIPtr _serviceManager;
    };
}

bool
IceBox::IceBoxService::start(int argc, char* argv[], int& status)
{
    // Run through the command line arguments removing all the service
    // properties.
    vector<string> args = Ice::argsToStringSeq(argc, argv);
    PropertiesPtr properties = communicator()->getProperties();
    const string prefix = "IceBox.Service.";
    PropertyDict services = properties->getPropertiesForPrefix(prefix);
    for (const auto& service : services)
    {
        string name = service.first.substr(prefix.size());
        auto q = args.begin();
        while (q != args.end())
        {
            if (q->find("--" + name + ".") == 0)
            {
                q = args.erase(q);
                continue;
            }
            ++q;
        }
    }

    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    try
    {
        args = opts.parse(args);
    }
    catch (const IceInternal::BadOptException& e)
    {
        error(e.what());
        usage(argv[0]);
        return false;
    }

    if (opts.isSet("help"))
    {
        usage(argv[0]);
        status = EXIT_SUCCESS;
        return false;
    }
    if (opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        status = EXIT_SUCCESS;
        return false;
    }

    if (!args.empty())
    {
        usage(argv[0]);
        return false;
    }

    _serviceManager = make_shared<ServiceManagerI>(communicator(), argc, argv);

    return _serviceManager->start();
}

bool
IceBox::IceBoxService::stop()
{
    if (_serviceManager)
    {
        _serviceManager->stop();
        _serviceManager = nullptr;
    }
    return true;
}

void
IceBox::IceBoxService::usage(const string& appName)
{
    string options = "Options:\n"
                     "-h, --help           Show this message.\n"
                     "-v, --version        Display the Ice version.";
#ifndef _WIN32
    options.append("\n"
                   "\n"
                   "--daemon             Run as a daemon.\n"
                   "--pidfile FILE       Write process ID into FILE.\n"
                   "--noclose            Do not close open file descriptors.\n"
                   "--nochdir            Do not change the current working directory.");
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

#ifdef _WIN32

int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
    IceBox::IceBoxService svc;

    InitializationData initData;
    // Initialize the service with a Properties object with the correct property prefix enabled.
    // Since IceStorm is an IceBox service, we need to enable the IceBox property prefix as we allow configuring
    // IceStorm properties through IceBox properties.
    initData.properties = make_shared<Properties>("IceBox", "IceStorm");
    initData.properties->setProperty("Ice.Admin.DelayCreation", "1");
    return svc.main(argc, argv, std::move(initData));
}
