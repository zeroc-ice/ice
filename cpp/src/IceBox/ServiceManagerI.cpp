// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/DynamicLibrary.h>
#include <IceBox/ServiceManagerI.h>

using namespace Ice;
using namespace IceInternal;
using namespace std;

typedef IceBox::ServicePtr (*SERVICE_FACTORY)(CommunicatorPtr);

IceBox::ServiceManagerI::ServiceManagerI(CommunicatorPtr communicator)
    : _communicator(communicator)
{
}

IceBox::ServiceManagerI::~ServiceManagerI()
{
}

void
IceBox::ServiceManagerI::shutdown(const Current& current)
{
    _communicator->shutdown();
}

int
IceBox::ServiceManagerI::run(int& argc, char* argv[])
{
    try
    {
        _logger = _communicator->getLogger();

        ServiceManagerPtr obj = this;

        //
        // Create an object adapter. Services probably should NOT share
        // this object adapter, as the endpoint(s) for this object adapter
        // will most likely need to be firewalled for security reasons.
        //
        ObjectAdapterPtr adapter = _communicator->createObjectAdapterFromProperty("ServiceManagerAdapter",
                                                                                  "IceBox.ServiceManager.Endpoints");
        adapter->add(obj, stringToIdentity("ServiceManager"));

        //
        // Load and initialize the services.
        //
        if (!initServices(argc, argv))
        {
            stopServices();
            return EXIT_FAILURE;
        }

        //
        // Invoke start() on the services.
        //
        if (!startServices())
        {
            stopServices();
            return EXIT_FAILURE;
        }

        //
        // Start request dispatching after we've started the services.
        //
        adapter->activate();

        _communicator->waitForShutdown();

        //
        // Invoke stop() on the services.
        //
        stopServices();
    }
    catch (const Exception& ex)
    {
        Error out(_logger);
        out << "ServiceManager: " << ex;
        stopServices();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool
IceBox::ServiceManagerI::initServices(int& argc, char* argv[])
{
    //
    // Retrieve all properties with the prefix "IceBox.Service.".
    // These properties should have the following format:
    //
    // IceBox.Service.Foo=Package.Foo [args]
    //
    const string prefix = "IceBox.Service.";
    PropertiesPtr properties = _communicator->getProperties();
    StringSeq allOptions = properties->getCommandLineOptions();
    StringSeq arr = properties->getProperties(prefix);
    for (StringSeq::size_type i = 0; i < arr.size(); i += 2)
    {
        string name = arr[i].substr(prefix.size());
        string value = arr[i + 1];

        //
        // Separate the factory function from the arguments.
        //
        string factoryFunc;
        StringSeq args;
        string::size_type pos = value.find_first_of(" \t\n");
        if (pos == string::npos)
        {
            factoryFunc = value;
        }
        else
        {
            factoryFunc = value.substr(0, pos);
            string::size_type beg = value.find_first_not_of(" \t\n", pos);
            while (beg != string::npos)
            {
                string::size_type end = value.find_first_of(" \t\n", beg);
                if (end == string::npos)
                {
                    args.push_back(value.substr(beg));
                    beg = end;
                }
                else
                {
                    args.push_back(value.substr(beg, end - beg));
                    beg = value.find_first_not_of(" \t\n", end);
                }
            }
        }

        //
        // Now we need to create a property set to pass to init().
        // The property set is populated from a number of sources.
        // The precedence order (from lowest to highest) is:
        //
        // 1. Properties defined in the server property set (e.g.,
        //    that were defined in the server's configuration file)
        // 2. Service arguments
        // 3. Server arguments
        //
        // We'll compose an array of arguments in the above order.
        //
        vector<string> l;
        StringSeq::size_type j;
        int k;
        for (j = 0; j < allOptions.size(); j++)
        {
            if (allOptions[j].find("--" + name + ".") == 0)
            {
                l.push_back(allOptions[j]);
            }
        }
        for (j = 0; j < args.size(); j++)
        {
            l.push_back(args[j]);
        }
        for (k = 1; k < argc; k++)
        {
            string s = argv[k];
            if (s.find("--" + name + ".") == 0)
            {
                l.push_back(s);
            }
        }

        //
        // Create the service property set.
        //
        addArgumentPrefix(name);
        int serviceArgc = static_cast<int>(l.size() + 1);
        char** serviceArgv = new char*[serviceArgc + 1];
        serviceArgv[0] = argv[0];
        for (k = 1; k < serviceArgc; k++)
        {
            serviceArgv[k] = const_cast<char*>(l[k - 1].c_str());
        }
        PropertiesPtr serviceProperties = createProperties(serviceArgc, serviceArgv);
        StringSeq serviceArgs;
        for (k = 1; k < serviceArgc; k++)
        {
            serviceArgs.push_back(serviceArgv[k]);
        }
        delete[] serviceArgv;

        //
        // Load the dynamic library.
        //
        string::size_type colon = factoryFunc.rfind(':');
        if (colon == string::npos || colon == factoryFunc.size() - 1)
        {
            Error out(_logger);
            out << "ServiceManager: invalid factory function `" << factoryFunc << "'";
            return false;
        }
        string libName = factoryFunc.substr(0, colon);
        string funcName = factoryFunc.substr(colon + 1);
        DynamicLibraryPtr library = new DynamicLibrary();
        if (!library->load(libName))
        {
            string msg = library->getErrorMessage();
            Error out(_logger);
            out << "ServiceManager: unable to load library `" << libName << "'";
            if (!msg.empty())
            {
                out << ": " << msg;
            }
            return false;
        }

        //
        // Lookup the factory function and invoke it.
        //
        DynamicLibrary::symbol_type sym = library->getSymbol(funcName);
        if (sym == 0)
        {
            string msg = library->getErrorMessage();
            Error out(_logger);
            out << "ServiceManager: unable to load symbol `" << funcName << "'";
            if (!msg.empty())
            {
                out << ": " << msg;
            }
            return false;
        }
        SERVICE_FACTORY factory = (SERVICE_FACTORY)sym;
        ServicePtr service;
        try
        {
            service = factory(_communicator);
        }
        catch (const Exception& ex)
        {
            Error out(_logger);
            out << "ServiceManager: exception in factory function `" << funcName << "': " << ex;
            return false;
        }

        //
        // Invoke Service::init().
        //
        try
        {
            service->init(name, _communicator, serviceProperties, serviceArgs);
            _services[name] = service;
            _libraries.push_back(library);
        }
        catch (const ServiceFailureException& ex)
        {
            Error out(_logger);
            out << "ServiceManager: initialization failed for service " << name;
            return false;
        }
        catch (const Exception& ex)
        {
            Error out(_logger);
            out << "ServiceManager: exception while initializing service " << name << ": " << ex;
            return false;
        }
    }

    return true;
}

bool
IceBox::ServiceManagerI::startServices()
{
    map<string,ServicePtr>::const_iterator r;
    for (r = _services.begin(); r != _services.end(); ++r)
    {
        try
        {
            (*r).second->start();
        }
        catch (const ServiceFailureException& ex)
        {
            Error out(_logger);
            out << "ServiceManager: start failed for service " << (*r).first;
            return false;
        }
        catch (const Exception& ex)
        {
            Error out(_logger);
            out << "ServiceManager: exception in start for service " << (*r).first << ": " << ex;
            return false;
        }
    }

    return true;
}

void
IceBox::ServiceManagerI::stopServices()
{
    map<string,ServicePtr>::const_iterator r;
    for (r = _services.begin(); r != _services.end(); ++r)
    {
        try
        {
            (*r).second->stop();
        }
        catch (const Exception& ex)
        {
            Error out(_logger);
            out << "ServiceManager: exception in stop for service " << (*r).first << ": " << ex;
        }
    }
    _services.clear();
}
