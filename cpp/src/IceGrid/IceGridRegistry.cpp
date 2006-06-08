// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/Service.h>
#include <IceGrid/RegistryI.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

class RegistryService : public Service
{
public:

    RegistryService();

protected:

    virtual bool start(int, char*[]);
    virtual bool stop();
    virtual CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&);

private:

    void usage(const std::string&);

    RegistryIPtr _registry;
};

} // End of namespace IceGrid

RegistryService::RegistryService()
{
}

bool
RegistryService::start(int argc, char* argv[])
{
    bool nowarn;

    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "nowarn");
    
    vector<string> args;
    try
    {
    	args = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
        error(e.reason);
	usage(argv[0]);
	return false;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage(argv[0]);
	return false;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	print(ICE_STRING_VERSION);
	return false;
    }
    nowarn = opts.isSet("nowarn");

    if(!args.empty())
    {
	usage(argv[0]);
	return false;
    }

    //
    // Warn the user that setting Ice.ThreadPool.Server isn't useful.
    //
    if(communicator()->getProperties()->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.Size", 0) > 0)
    {
	Warning out(communicator()->getLogger());
	out << "setting `Ice.ThreadPool.Server.Size' is not useful,\n";
	out << "you should set individual adapter thread pools instead.";
    }
    
    _registry = new RegistryI(communicator());
    if(!_registry->start(nowarn))
    {
	return false;
    }

    return true;
}

bool
RegistryService::stop()
{
    _registry->stop();
    return true;
}

CommunicatorPtr
RegistryService::initializeCommunicator(int& argc, char* argv[], 
					const InitializationData& initializationData)
{
    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initData.properties);
    
    //
    // Make sure that IceGridRegistry doesn't use
    // thread-per-connection or collocation optimization.
    //
    initData.properties->setProperty("Ice.ThreadPerConnection", "");
    initData.properties->setProperty("Ice.Default.CollocationOptimization", "0");

    return Service::initializeCommunicator(argc, argv, initData);
}

void
RegistryService::usage(const string& appName)
{
    string options =
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"--nowarn             Don't print any security warnings.";
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
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory."
    );
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

int
main(int argc, char* argv[])
{
    RegistryService svc;
    return svc.main(argc, argv);
}
