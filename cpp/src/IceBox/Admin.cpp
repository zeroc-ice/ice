// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Ice/SliceChecksums.h>
#include <IceUtil/Options.h>
#include <IceBox/IceBox.h>

using namespace std;
using namespace Ice;

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

#ifdef _WIN32

int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
    Client app;
    return app.main(argc, argv);
}

void
Client::usage()
{
    cerr << "Usage: " << appName() << " [options] [command...]\n";
    cerr <<
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
Client::run(int argc, char* argv[])
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> commands;
    try
    {
        commands = opts.parse(argc, const_cast<const char**>(argv));
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cerr << e.reason << endl;
        usage();
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage();
        return EXIT_SUCCESS;
    }
    if(opts.isSet("version"))
    {
        cout << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    if(commands.empty())
    {
        usage();
        return EXIT_FAILURE;
    }

    ObjectPrxPtr base = communicator()->propertyToProxy("IceBoxAdmin.ServiceManager.Proxy");

    if(base == 0)
    {
        //
        // The old deprecated way to retrieve the service manager proxy
        //

        PropertiesPtr properties = communicator()->getProperties();

        Identity managerIdentity;
        managerIdentity.category = properties->getPropertyWithDefault("IceBox.InstanceName", "IceBox");
        managerIdentity.name = "ServiceManager";

        string managerProxy;
        if(properties->getProperty("Ice.Default.Locator").empty())
        {
            string managerEndpoints = properties->getProperty("IceBox.ServiceManager.Endpoints");
            if(managerEndpoints.empty())
            {
                cerr << appName() << ": property `IceBoxAdmin.ServiceManager.Proxy' is not set" << endl;
                return EXIT_FAILURE;
            }

            managerProxy = "\"" + communicator()->identityToString(managerIdentity) + "\" :" + managerEndpoints;
        }
        else
        {
            string managerAdapterId = properties->getProperty("IceBox.ServiceManager.AdapterId");
            if(managerAdapterId.empty())
            {
                cerr << appName() << ": property `IceBoxAdmin.ServiceManager.Proxy' is not set" << endl;
                return EXIT_FAILURE;
            }

            managerProxy = "\"" + communicator()->identityToString(managerIdentity) + "\" @" + managerAdapterId;
        }

        base = communicator()->stringToProxy(managerProxy);
    }

    IceBox::ServiceManagerPrxPtr manager = ICE_CHECKED_CAST(IceBox::ServiceManagerPrx, base);
    if(!manager)
    {
        cerr << appName() << ": `" << base << "' is not an IceBox::ServiceManager" << endl;
        return EXIT_FAILURE;
    }

    Ice::SliceChecksumDict serverChecksums = manager->getSliceChecksums();
    Ice::SliceChecksumDict localChecksums = Ice::sliceChecksums();
    for(Ice::SliceChecksumDict::const_iterator p = localChecksums.begin(); p != localChecksums.end(); ++p)
    {
        Ice::SliceChecksumDict::const_iterator q = serverChecksums.find(p->first);
        if(q == serverChecksums.end())
        {
            cerr << appName() << ": server is using unknown Slice type `" << q->first << "'" << endl;
        }
        else if(p->second != q->second)
        {
            cerr << appName() << ": server is using a different Slice definition of `" << q->first << "'" << endl;
        }
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
                cerr << appName() << ": no service name specified." << endl;
                return EXIT_FAILURE;
            }

            try
            {
                manager->startService(*r);
            }
            catch(const IceBox::NoSuchServiceException&)
            {
                cerr << appName() << ": unknown service `" << *r << "'" << endl;
                return EXIT_FAILURE;
            }
            catch(const IceBox::AlreadyStartedException&)
            {
                cerr << appName() << ": service already started." << endl;
            }
        }
        else if((*r) == "stop")
        {
            if(++r == commands.end())
            {
                cerr << appName() << ": no service name specified." << endl;
                return EXIT_FAILURE;
            }

            try
            {
                manager->stopService(*r);
            }
            catch(const IceBox::NoSuchServiceException&)
            {
                cerr << appName() << ": unknown service `" << *r << "'" << endl;
                return EXIT_FAILURE;
            }
            catch(const IceBox::AlreadyStoppedException&)
            {
                cerr << appName() << ": service already stopped." << endl;
            }
        }
        else
        {
            cerr << appName() << ": unknown command `" << *r << "'" << endl;
            usage();
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
