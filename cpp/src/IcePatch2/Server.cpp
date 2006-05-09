// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    virtual CommunicatorPtr initializeCommunicator(int&, char*[]);

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
    if(properties->getProperty(endpointsProperty).empty())
    {
	error("property `" + endpointsProperty + "' is not set");
	return false;
    }
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("IcePatch2");

    const string adminEndpointsProperty = "IcePatch2.Admin.Endpoints";
    ObjectAdapterPtr adminAdapter;
    if(!properties->getProperty(adminEndpointsProperty).empty())
    {
	adminAdapter = communicator()->createObjectAdapter("IcePatch2.Admin");
    }

    const string instanceNameProperty = "IcePatch2.InstanceName";
    string instanceName = properties->getPropertyWithDefault(instanceNameProperty, "IcePatch2");

    const string idProperty = "IcePatch2.Identity";
    string idStr = properties->getProperty(idProperty);
    if(idStr.empty())
    {
	idStr = instanceName + "/server";
    }
    Identity id = communicator()->stringToIdentity(idStr);
    adapter->add(new FileServerI(dataDir, infoSeq), id);

    if(adminAdapter)
    {
	const string adminIdProperty = "IcePatch2.AdminIdentity";
	string adminIdStr = properties->getProperty(adminIdProperty);
	if(adminIdStr.empty())
	{
	    adminIdStr = instanceName + "/admin";
	}
	Identity adminId = communicator()->stringToIdentity(adminIdStr);
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

CommunicatorPtr
IcePatch2::PatcherService::initializeCommunicator(int& argc, char* argv[])
{
    return Service::initializeCommunicator(argc, argv);
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
    char** v = new char*[argc + 2];
    char** vsave = new char*[argc + 2]; // We need to keep a copy of the vector because svc.main modifies argv.

    v[0] = new char[strlen(argv[0]) + 1];
    strcpy(v[0], argv[0]);
    vsave[0] = v[0];

    v[1] = new char[sizeof("--nochdir")];
    strcpy(v[1], "--nochdir");
    vsave[1] = v[1];

    int i;
    for(i = 1; i < argc; ++i)
    {
        v[i + 1] = new char[strlen(argv[i]) + 1];
        strcpy(v[i + 1], argv[i]);
	vsave[i + 1] = v[i + 1];
    }
    v[argc + 1] = 0;

    try
    {
	int ac = argc + 1;
        status = svc.main(ac, v);
    }
    catch(...)
    {
        // Ignore exceptions -- the only thing left to do is to free memory.
    }

    for(i = 0; i < argc + 1; ++i)
    {
        delete[] vsave[i];
    }
    delete[] v;
    delete[] vsave;
#endif

    return status;
}
