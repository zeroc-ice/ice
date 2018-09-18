// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/ConsoleUtil.h>
#include <IceUtil/Options.h>
#include <ServiceInstaller.h>

using namespace std;
using namespace IceInternal;

int run(const Ice::StringSeq&);

Ice::CommunicatorPtr communicator;

void
destroyCommunicator(int)
{
    communicator->destroy();
}

bool debug = false;
bool pauseEnabled = true;
bool pause = false;

int
wmain(int argc, wchar_t* argv[])
{
    int status = 0;

    try
    {
        Ice::CtrlCHandler ctrlCHandler;
        Ice::InitializationData id;
        id.properties = Ice::createProperties();
        id.properties->setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL");
        Ice::CommunicatorHolder ich(argc, argv, id);
        communicator = ich.communicator();

        ctrlCHandler.setCallback(&destroyCommunicator);

        status = run(Ice::argsToStringSeq(argc, argv));
    }
    catch(const std::exception& ex)
    {
        consoleErr << ex.what() << endl;
        status = 1;
    }

    if(pauseEnabled && (pause || debug || status != 0))
    {
        system("pause");
    }

    return status;
}

void
usage(const string& name)
{
    string defaultImagePath = IceServiceInstaller::getServiceInstallerPath();
    if(defaultImagePath.empty())
    {
        defaultImagePath = string("<error: cannot retrieve path of ") + name + ">";
    }
    defaultImagePath += "\\<service>";
#ifdef _DEBUG
    defaultImagePath += 'd';
#endif
    defaultImagePath += ".exe";

    consoleErr << "Usage: " << name
         << " [options] service config-file [property] [property]\n";
    consoleErr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-n, --nopause        Do not call pause after displaying a message.\n"
        "-v, --version        Display the Ice version.\n"
        "-u, --uninstall      Uninstall the Windows service.\n"
        "\n"
        "service must be icegridregistry, icegridnode or glacier2router.\n"
        "\n"
        "config-file          Path to the Ice configuration file for this service.\n"
        "                     If the path starts with HKLM\\ the configuration will be\n"
        "                     read from the corresponding Windows registry keyword in\n"
        "                     HKEY_LOCAL_MACHINE.\n"
        "\n"
        "Valid properties:\n"
        "AutoStart            0 = Manual, 1 = Automatic, 2 = Automatic (Delayed Start)\n"
        "                     The default value is 1.\n"
        "Debug                Show diagnostics when installing/uninstalling a service.\n"
        "DependOnRegistry     If non-zero, the service depends on the IceGrid registry\n"
        "                     service (the IceGrid registry service name is computed\n"
        "                     using Ice.Default.Locator in <config-file>).\n"
        "Description          Description of the service.\n"
        "DisplayName          Display name of the service.\n"
        "EventLog             The name of the EventLog used by this service;\n"
        "                     the default is Application.\n"
        "ImagePath            Full path to <service>.exe. The default value is\n"
        "                     " << defaultImagePath << "\n" <<
        "ObjectName           Account used to run the service. Defaults to\n"
        "                     NT Authority\\LocalService.\n"
        "Password             Password for ObjectName.\n"
        ;
}

int
run(const Ice::StringSeq& args)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("u", "uninstall");
    opts.addOpt("n", "nopause");

    vector<string> propNames = IceServiceInstaller::getPropertyNames();

    for(size_t i = 0; i < propNames.size(); ++i)
    {
        opts.addOpt("", propNames[i], IceUtilInternal::Options::NeedArg);
    }

    vector<string> commands;
    try
    {
        commands = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << "Error:" << e.reason << endl;
        usage(args[0]);
        return 1;
    }

    pauseEnabled = !opts.isSet("nopause");

    if(opts.isSet("help"))
    {
        usage(args[0]);
        pause = true;
        return 0;
    }
    if(opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        pause = true;
        return 0;
    }

    if(commands.size() != 2)
    {
        usage(args[0]);
        return 1;
    }

    int serviceType = -1;
    for(int j = 0; j < IceServiceInstaller::serviceCount; ++j)
    {
        if(commands[0] == IceServiceInstaller::serviceTypeToLowerString(j))
        {
            serviceType = j;
            break; // for
        }
    }

    if(serviceType == -1)
    {
        consoleErr << "Invalid service " << commands[0] << endl;
        return 1;
    }

    string configFile = commands[1];

    try
    {
        IceServiceInstaller installer(serviceType, configFile, communicator);

        if(opts.isSet("uninstall"))
        {
            installer.uninstall();
        }
        else
        {
            Ice::PropertiesPtr properties = communicator->getProperties();

            for(size_t j = 0; j < propNames.size(); ++j)
            {
                if(opts.isSet(propNames[j]))
                {
                    properties->setProperty(propNames[j], opts.optArg(propNames[j]));
                }
            }

            debug = properties->getPropertyAsInt("Debug") > 0;

            installer.install(properties);
        }
    }
    catch(const exception& ex)
    {
        consoleErr << "Error: " << ex.what() << endl;
        return 1;
    }
    return 0;
}
