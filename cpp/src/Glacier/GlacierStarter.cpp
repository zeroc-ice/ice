// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Service.h>
#include <Glacier/StarterI.h>
#include <fstream>

#ifndef _WIN32
#   include <signal.h>
#   include <sys/wait.h>
#endif

using namespace std;

namespace Glacier
{

class StarterService : public Ice::Service
{
public:

    StarterService();

protected:

    virtual bool start(int, char*[]);
    virtual bool stop();
    virtual Ice::CommunicatorPtr initializeCommunicator(int&, char*[]);

private:

    void usage(const std::string&);

    StarterIPtr _starter;
};

} // End of namespace Glacier

#ifndef _WIN32
extern "C"
{

static void
childHandler(int)
{
    //
    // Call wait to de-allocate any resources allocated for the child
    // process and avoid zombie processes. See man wait or waitpid for
    // more information.
    //
    int olderrno = errno;

    pid_t pid;
    do
    {
	pid = waitpid(-1, 0, WNOHANG);
    }
    while(pid > 0);

    assert(pid != -1 || errno == ECHILD);

    errno = olderrno;
}

}
#endif

Glacier::StarterService::StarterService()
{
}

bool
Glacier::StarterService::start(int argc, char* argv[])
{
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            usage(argv[0]);
            return false;
        }
        else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            trace(ICE_STRING_VERSION);
            return false;
        }
        else
        {
            error("unknown option `" + string(argv[i]) + "'");
            usage(argv[0]);
            return false;
        }
    }

    Ice::PropertiesPtr properties = communicator()->getProperties();

    //
    // Initialize the object adapter (and make sure this object
    // adapter doesn't register itself with the locator).
    // 
    const string endpointsProperty = "Glacier.Starter.Endpoints";
    if(properties->getProperty(endpointsProperty).empty())
    {
	error("property `" + endpointsProperty + "' is not set");
	return false;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Glacier.Starter");

    //
    // Get the permissions verifier, or create one if no verifier is
    // specified.
    //
    string verifierProperty = properties->getProperty("Glacier.Starter.PermissionsVerifier");
    PermissionsVerifierPrx verifier;
    if(!verifierProperty.empty())
    {
	verifier = PermissionsVerifierPrx::checkedCast(communicator()->stringToProxy(verifierProperty));
	if(!verifier)
	{
	    error("permissions verifier `" + verifierProperty + "' is invalid");
	    return false;
	}
    }
    else
    {
	string passwordsProperty = properties->getPropertyWithDefault("Glacier.Starter.CryptPasswords", "passwords");

	ifstream passwordFile(passwordsProperty.c_str());
	if(!passwordFile)
	{
            string err = strerror(errno);
	    error("cannot open `" + passwordsProperty + "' for reading: " + err);
	    return false;
	}

	map<string, string> passwords;

	while(true)
	{
	    string userId;
	    passwordFile >> userId;
	    if(!passwordFile)
	    {
		break;
	    }

	    string password;
	    passwordFile >> password;
	    if(!passwordFile)
	    {
		break;
	    }

	    assert(!userId.empty());
	    assert(!password.empty());
	    passwords.insert(make_pair(userId, password));
	}

	PermissionsVerifierPtr verifierImpl = new CryptPasswordVerifierI(passwords);
	verifier = PermissionsVerifierPrx::uncheckedCast(adapter->addWithUUID(verifierImpl));
    }

    //
    // Create and initialize the starter object.
    //
    _starter = new StarterI(communicator(), verifier);
    adapter->add(_starter, Ice::stringToIdentity("Glacier/starter"));

    //
    // Everything ok, let's go.
    //
    adapter->activate();

    return true;
}

bool
Glacier::StarterService::stop()
{
    //
    // Destroy the starter.
    //
    assert(_starter);
    _starter->destroy();

    return true;
}

Ice::CommunicatorPtr
Glacier::StarterService::initializeCommunicator(int& argc, char* argv[])
{
    //
    // Make sure that this process doesn't use a router.
    //
    Ice::PropertiesPtr defaultProperties = Ice::getDefaultProperties(argc, argv);
    defaultProperties->setProperty("Ice.Default.Router", "");

    return Service::initializeCommunicator(argc, argv);
}

void
Glacier::StarterService::usage(const string& appName)
{
    string options =
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.";
#ifdef _WIN32
    if(!win9x())
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
    cerr << "Usage: " << appName << " [options]" << endl;
    cerr << options << endl;
}

int
main(int argc, char* argv[])
{
#ifndef _WIN32
    //
    // This application forks, so we need a signal handler for child
    // termination.
    //
    struct sigaction action;
    action.sa_handler = childHandler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGCHLD);
    action.sa_flags = 0;
    sigaction(SIGCHLD, &action, 0);
#endif

    Glacier::StarterService svc;
    return svc.main(argc, argv);
}
