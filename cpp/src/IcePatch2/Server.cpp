// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Service.h>
#include <IcePatch2/FileServerI.h>
#include <IcePatch2/Util.h>

#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

namespace IcePatch2
{

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
    string dataDir;

    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage(argv[0]);
	    return false;
	}
	else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return false;
	}
        else if(argv[i][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
            usage(argv[0]);
            return false;
        }
        else
        {
            if(dataDir.empty())
            {
                dataDir = argv[i];
            }
            else
            {
		cerr << argv[0] << ": too many arguments" << endl;
		usage(argv[0]);
		return false;
            }
        }
    }

    PropertiesPtr properties = communicator()->getProperties();

    if(dataDir.empty())
    {
	string dataDir = properties->getProperty("IcePatch2.Directory");
	if(dataDir.empty())
	{
	    cerr << argv[0] << ": no data directory specified" << endl;
	    usage(argv[0]);
	    return false;
	}
    }

    FileInfoSeq infoSeq;

    try
    {
#ifdef _WIN32
	char cwd[_MAX_PATH];
	if(_getcwd(cwd, _MAX_PATH) == NULL)
#else
	    char cwd[PATH_MAX];
	if(getcwd(cwd, PATH_MAX) == NULL)
#endif
	{
	    throw string("cannot get the current directory: ") + strerror(errno);
	}
	
	dataDir = normalize(string(cwd) + '/' + dataDir);

	loadFileInfoSeq(dataDir, infoSeq);
    }
    catch(const string& ex)
    {
        cerr << argv[0] << ": " << ex << endl;
        return false;
    }
    catch(const char* ex)
    {
        cerr << argv[0] << ": " << ex << endl;
        return false;
    }
    
    const char* endpointsProperty = "IcePatch2.Endpoints";
    if(properties->getProperty(endpointsProperty).empty())
    {
	cerr << argv[0] << ": property `" << endpointsProperty << "' is not set" << endl;
	return false;
    }
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("IcePatch2");

    const char* adminEndpointsProperty = "IcePatch2.Admin.Endpoints";
    ObjectAdapterPtr adminAdapter;
    if(!properties->getProperty(adminEndpointsProperty).empty())
    {
	adminAdapter = communicator()->createObjectAdapter("IcePatch2.Admin");
    }

    const char* idProperty = "IcePatch2.Identity";
    Identity id = stringToIdentity(properties->getPropertyWithDefault(idProperty, "IcePatch2/server"));
    adapter->add(new FileServerI(dataDir, infoSeq), id);

    if(adminAdapter)
    {
	const char* adminIdProperty = "IcePatch2.AdminIdentity";
	Identity adminId = stringToIdentity(properties->getPropertyWithDefault(adminIdProperty, "IcePatch2/admin"));
//	adminAdapter->add(new AdminI(communicator()), adminId);
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
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory."
    );
#endif
    cerr << "Usage: " << appName << " [options] [DIR]" << endl;
    cerr << options << endl;
}

int
main(int argc, char* argv[])
{
    IcePatch2::PatcherService svc;
    return svc.main(argc, argv);
}
