// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/SliceChecksums.h>
#include <Ice/ConsoleUtil.h>
#include <IceUtil/Options.h>
#include <IceBox/IceBox.h>

using namespace std;
using namespace IceInternal;

int run(Ice::StringSeq&);

//
// Global variable for destroyCommunicator
//
Ice::CommunicatorPtr communicator;

//
// Callback for CtrlCHandler
//
void
destroyCommunicator(int)
{
    communicator->destroy();
}

int
#ifdef _WIN32
wmain(int argc, wchar_t* argv[])
#else
main(int argc, char* argv[])
#endif
{
    int status = 0;

    try
    {
        //
        // CtrlCHandler must be created before the communicator or any other threads are started
        //
        Ice::CtrlCHandler ctrlCHandler;

        //
        // CommunicatorHolder's ctor initializes an Ice communicator,
        // and it's dtor destroys this communicator.
        //
        Ice::CommunicatorHolder ich(argc, argv);
        communicator = ich.communicator();

        //
        // Destroy communicator on Ctrl-C
        //
        ctrlCHandler.setCallback(&destroyCommunicator);

        Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
        status = run(args);
    }
    catch(const std::exception& ex)
    {
        cerr << ex.what() << endl;
        status = 1;
    }

    return status;
}

void
usage(const string& name)
{
    consoleErr << "Usage: " << name << " [options] [command...]\n";
    consoleErr <<
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "\n"
        "Commands:\n"
        "start SERVICE        Start a service.\n"
        "stop SERVICE         Stop a service.\n"
        "shutdown             Shutdown the server.\n"
        ;
}

int
run(Ice::StringSeq& args)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> commands;
    try
    {
        commands = opts.parse(args);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << e.reason << endl;
        usage(args[0]);
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(args[0]);
        return EXIT_SUCCESS;
    }
    if(opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    if(commands.empty())
    {
        usage(args[0]);
        return EXIT_FAILURE;
    }

    Ice::ObjectPrxPtr base = communicator->propertyToProxy("IceBoxAdmin.ServiceManager.Proxy");

    if(base == 0)
    {
        //
        // The old deprecated way to retrieve the service manager proxy
        //

        Ice::PropertiesPtr properties = communicator->getProperties();

        Ice::Identity managerIdentity;
        managerIdentity.category = properties->getPropertyWithDefault("IceBox.InstanceName", "IceBox");
        managerIdentity.name = "ServiceManager";

        string managerProxy;
        if(properties->getProperty("Ice.Default.Locator").empty())
        {
            string managerEndpoints = properties->getProperty("IceBox.ServiceManager.Endpoints");
            if(managerEndpoints.empty())
            {
                consoleErr << args[0] << ": property `IceBoxAdmin.ServiceManager.Proxy' is not set" << endl;
                return EXIT_FAILURE;
            }

            managerProxy = "\"" + communicator->identityToString(managerIdentity) + "\" :" + managerEndpoints;
        }
        else
        {
            string managerAdapterId = properties->getProperty("IceBox.ServiceManager.AdapterId");
            if(managerAdapterId.empty())
            {
                consoleErr << args[0] << ": property `IceBoxAdmin.ServiceManager.Proxy' is not set" << endl;
                return EXIT_FAILURE;
            }

            managerProxy = "\"" + communicator->identityToString(managerIdentity) + "\" @" + managerAdapterId;
        }

        base = communicator->stringToProxy(managerProxy);
    }

    IceBox::ServiceManagerPrxPtr manager = ICE_CHECKED_CAST(IceBox::ServiceManagerPrx, base);
    if(!manager)
    {
        consoleErr << args[0] << ": `" << base << "' is not an IceBox::ServiceManager" << endl;
        return EXIT_FAILURE;
    }

    for(vector<string>::const_iterator r = commands.begin(); r != commands.end(); ++r)
    {
        if((*r) == "shutdown")
        {
            manager->shutdown();
        }
        else if((*r) == "start")
        {
            if(++r == commands.end())
            {
                consoleErr << args[0] << ": no service name specified." << endl;
                return EXIT_FAILURE;
            }

            try
            {
                manager->startService(*r);
            }
            catch(const IceBox::NoSuchServiceException&)
            {
                consoleErr << args[0] << ": unknown service `" << *r << "'" << endl;
                return EXIT_FAILURE;
            }
            catch(const IceBox::AlreadyStartedException&)
            {
                consoleErr << args[0] << ": service already started." << endl;
            }
        }
        else if((*r) == "stop")
        {
            if(++r == commands.end())
            {
                consoleErr << args[0] << ": no service name specified." << endl;
                return EXIT_FAILURE;
            }

            try
            {
                manager->stopService(*r);
            }
            catch(const IceBox::NoSuchServiceException&)
            {
                consoleErr << args[0] << ": unknown service `" << *r << "'" << endl;
                return EXIT_FAILURE;
            }
            catch(const IceBox::AlreadyStoppedException&)
            {
                consoleErr << args[0] << ": service already stopped." << endl;
            }
        }
        else
        {
            consoleErr << args[0] << ": unknown command `" << *r << "'" << endl;
            usage(args[0]);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
