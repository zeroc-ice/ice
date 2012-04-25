// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Service.h>
#include <IcePatch2/FileServerI.h>
#include <IcePatch2/Util.h>

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

    virtual bool start(int, char*[], int&);
    virtual bool stop();

private:

    void usage(const std::string&);
};

};

IcePatch2::PatcherService::PatcherService()
{
}

bool
IcePatch2::PatcherService::start(int argc, char* argv[], int& status)
{
    PropertiesPtr properties = communicator()->getProperties();

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    
    vector<string> args;
    try
    {
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
        status = EXIT_SUCCESS;
        return false;
    }
    if(opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        status = EXIT_SUCCESS;
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
        if(!IceUtilInternal::isAbsolutePath(dataDir))
        {
            string cwd;
            if(IceUtilInternal::getcwd(cwd) != 0)
            {
                throw "cannot get the current directory:\n" + IceUtilInternal::lastErrorToString();
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
#ifndef _WIN32
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

#ifdef _WIN32

//COMPILERFIX: Borland C++ 2010 doesn't support wmain for console applications.
#ifdef __BCCPLUSPLUS__

int
main(int argc, char* argv[])

#else

int
wmain(int argc, wchar_t* argv[])

#endif
{
    IcePatch2::PatcherService svc;
    int status = EXIT_FAILURE;
    status = svc.main(argc, argv);
    return status;
}

#else

int
main(int argc, char* argv[])
{
    IcePatch2::PatcherService svc;
    int status = EXIT_FAILURE;
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
    return status;
}

#endif
