// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Service.h>
#include <IcePack/Registry.h>

using namespace std;
using namespace IcePack;

namespace IcePack
{

class RegistryService : public Ice::Service
{
public:

    RegistryService();

protected:

    virtual bool start(int, char*[]);

private:

    void usage(const std::string&);

    std::auto_ptr<Registry> _registry;
};

} // End of namespace IcePack

IcePack::RegistryService::RegistryService()
{
}

bool
IcePack::RegistryService::start(int argc, char* argv[])
{
    bool nowarn = false;
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
        else if(strcmp(argv[i], "--nowarn") == 0)
        {
            nowarn = true;
        }
        else
        {
            error("unknown option `" + string(argv[i]) + "'");
            usage(argv[0]);
            return false;
        }
    }

    _registry = auto_ptr<Registry>(new Registry(communicator()));
    if(!_registry->start(nowarn, false))
    {
        return false;
    }

    return true;
}

void
IcePack::RegistryService::usage(const string& appName)
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
    cerr << "Usage: " << appName << " [options]" << endl;
    cerr << options << endl;
}

int
main(int argc, char* argv[])
{
    RegistryService svc;
    return svc.main(argc, argv);
}
