// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Service.h>
#include <IcePatch2/FileServerI.h>
#include <IcePatch2/Util.h>
#include <OS.h>

using namespace std;
using namespace Ice;
using namespace IcePatch2;

namespace IcePatch2
{

class AdminI : public Admin
{
public:
    
    AdminI(const CommunicatorPtr& communicator) :
        _communicator(communicator)
    {
    }

    virtual void
    shutdown(const Current&)
    {
        _communicator->shutdown();
    }

private:

    const CommunicatorPtr _communicator;
};

class PatcherService : public Service
{
public:

    PatcherService();

protected:

    virtual bool start(int, char*[]);
    virtual bool stop();

private:

    void usage(const std::string&);
};

};

IcePatch2::PatcherService::PatcherService()
{
}

bool
IcePatch2::PatcherService::start(int argc, char* argv[])
{
    PropertiesPtr properties = communicator()->getProperties();

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

    if(args.size() > 1)
    {
        error("too many arguments");
        usage(argv[0]);
        return false;
    }
    if(args.size() == 1)
    {
        properties->setProperty("IcePatch2.Directory", simplify(args[0]));
    }

    string dataDir = properties->getPropertyWithDefault("IcePatch2.Directory", ".");
    if(dataDir.empty())
    {
        error("no data directory specified");
        usage(argv[0]);
        return false;
    }

    FileInfoSeq infoSeq;

    try
    {
        if(!isAbsolute(dataDir))
        {
            string cwd;
            if(OS::getcwd(cwd) != 0)
            {
                throw "cannot get the current directory:\n" + lastError();
            }
            
            dataDir = cwd + '/' + dataDir;
        }

        loadFileInfoSeq(dataDir, infoSeq);
    }
    catch(const string& ex)
    {
        error(ex);
        return false;
    }
    catch(const char* ex)
    {
        error(ex);
        return false;
    }
    
    const string endpointsProperty = "IcePatch2.Endpoints";
    string endpoints = properties->getProperty(endpointsProperty);
    if(endpoints.empty())
    {
        error("property `" + endpointsProperty + "' is not set");
        return false;
    }
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("IcePatch2");

    ObjectAdapterPtr adminAdapter;
    if(!properties->getProperty("IcePatch2.Admin.Endpoints").empty())
    {
        adminAdapter = communicator()->createObjectAdapter("IcePatch2.Admin");
    }

    const string instanceNameProperty = "IcePatch2.InstanceName";
    string instanceName = properties->getPropertyWithDefault(instanceNameProperty, "IcePatch2");

    Identity id;
    id.category = instanceName;
    id.name = "server";
    adapter->add(new FileServerI(dataDir, infoSeq), id);

    if(adminAdapter)
    {
        Identity adminId;
        adminId.category = instanceName;
        adminId.name = "admin";
        adminAdapter->add(new AdminI(communicator()), adminId);
    }

    adapter->activate();
    if(adminAdapter)
    {
        adminAdapter->activate();
    }

    return true;
}

bool
IcePatch2::PatcherService::stop()
{
    return true;
}

void
IcePatch2::PatcherService::usage(const string& appName)
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
        "--noclose            Do not close open file descriptors."

        // --nochdir is intentionally not shown here. (See the comment in main().)
    );
#endif
    print("Usage: " + appName + " [options] [DIR]\n" + options);
}

int
main(int argc, char* argv[])
{
    IcePatch2::PatcherService svc;
    int status = EXIT_FAILURE;

#ifdef _WIN32
    status = svc.main(argc, argv);
#else
    //
    // For UNIX, force --nochdir option, so the service isn't started
    // with / as the working directory. That way, if the data
    // directory is specified as a relative path, we don't
    // misinterpret that path.
    //
    StringSeq args;
    args.push_back(argv[0]);
    args.push_back("--nochdir");
    for(int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }
    status = svc.main(args);
#endif

    return status;
}
