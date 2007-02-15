// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/Service.h>
#include <IceBox/ServiceManagerI.h>

using namespace std;
using namespace Ice;
using namespace IceBox;

namespace IceBox
{

class IceBoxService : public Ice::Service
{
public:

    IceBoxService();

protected:

    virtual bool start(int, char*[]);
    virtual bool stop();

private:

    void usage(const std::string&);
    ServiceManagerIPtr _serviceManager;
};

}

IceBox::IceBoxService::IceBoxService()
{
}

bool
IceBox::IceBoxService::start(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        error(e.reason);
        usage(argv[0]);
        return false;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return false;
    }
    if(opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        return false;
    }

    if(!args.empty())
    {
        usage(argv[0]);
        return false;
    }

    _serviceManager = new ServiceManagerI(communicator(), argc, argv);
    return _serviceManager->start();
}

bool
IceBox::IceBoxService::stop()
{
    if(_serviceManager)
    {
        _serviceManager->stop();
        _serviceManager = 0;
    }
    return true;
}

void
IceBox::IceBoxService::usage(const string& appName)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.";
#ifdef _WIN32
    if(checkSystem())
    {
        options.append(
        "\n"
        "\n"
        "--service NAME       Run as the Windows service NAME.\n"
        "\n"
        "--install NAME [--display DISP] [--executable EXEC] [args]\n"
        "                     Install as Windows service NAME. If DISP is\n"
        "                     provided, use it as the display name,\n"
        "                     otherwise NAME is used. If EXEC is provided,\n"
        "                     use it as the service executable, otherwise\n"
        "                     this executable is used. Any additional\n"
        "                     arguments are passed unchanged to the\n"
        "                     service at startup.\n"
        "--uninstall NAME     Uninstall Windows service NAME.\n"
        "--start NAME [args]  Start Windows service NAME. Any additional\n"
        "                     arguments are passed unchanged to the\n"
        "                     service.\n"
        "--stop NAME          Stop Windows service NAME."
        );
    }
#else
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--pidfile FILE       Write process ID into FILE.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory."
    );
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

int
main(int argc, char* argv[])
{
    IceBox::IceBoxService svc;
    return svc.main(argc, argv);
}

