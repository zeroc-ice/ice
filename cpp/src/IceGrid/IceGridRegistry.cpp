// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/Service.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/TraceLevels.h>
#ifdef __BCPLUSPLUS__
#  include <IceGrid/AdminSessionI.h>
#  include <IceGrid/ReapThread.h>
#  include <IceGrid/Database.h>
#  include <IceGrid/WellKnownObjectsManager.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

class RegistryService : public Service
{
public:

    RegistryService();

    virtual bool shutdown();

protected:

    virtual bool start(int, char*[]);
    virtual void waitForShutdown();
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
RegistryService::shutdown()
{
    assert(_registry);
    _registry->shutdown();
    return true;
}

bool
RegistryService::start(int argc, char* argv[])
{
    bool nowarn;
    bool readonly;

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "nowarn");
    opts.addOpt("", "readonly");
    
    vector<string> args;
    try
    {
#if defined(__BCPLUSPLUS__) && (__BCPLUSPLUS__ >= 0x0600)
        IceUtil::DummyBCC dummy;
#endif
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
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
    nowarn = opts.isSet("nowarn");
    readonly = opts.isSet("readonly");

    if(!args.empty())
    {
        cerr << argv[0] << ": too many arguments" << endl;
        usage(argv[0]);
        return false;
    }

    Ice::PropertiesPtr properties = communicator()->getProperties();

    //
    // Warn the user that setting Ice.ThreadPool.Server isn't useful.
    //
    if(!nowarn && properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.Size", 0) > 0)
    {
        Warning out(communicator()->getLogger());
        out << "setting `Ice.ThreadPool.Server.Size' is not useful, ";
        out << "you should set individual adapter thread pools instead.";
    }

    TraceLevelsPtr traceLevels = new TraceLevels(communicator(), "IceGrid.Registry");
    
    _registry = new RegistryI(communicator(), traceLevels, nowarn, readonly);
    if(!_registry->start())
    {
        return false;
    }

    return true;
}

void
RegistryService::waitForShutdown()
{
    //
    // Wait for the activator shutdown. Once the run method returns
    // all the servers have been deactivated.
    //
    enableInterrupt();
    _registry->waitForShutdown();
    disableInterrupt();
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
    // Make sure that IceGridRegistry doesn't use collocation optimization.
    //
    initData.properties->setProperty("Ice.Default.CollocationOptimized", "0");

    return Service::initializeCommunicator(argc, argv, initData);
}

void
RegistryService::usage(const string& appName)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "--nowarn             Don't print any security warnings.\n"
        "--readonly           Start the master registry in read-only mode.";
#ifndef _WIN32
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--pidfile FILE       Write process ID into FILE.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory.\n"
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
