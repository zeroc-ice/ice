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

IceBox::ServiceManagerI::ServiceManagerI(CommunicatorPtr communicator, int& argc, char* argv[])
    : _communicator(communicator)
{
    _logger = _communicator->getLogger();

    if (argc > 0)
    {
        _progName = argv[0];
    }

    for (int i = 1; i < argc; i++)
    {
        _argv.push_back(argv[i]);
    }

    PropertiesPtr properties = _communicator->getProperties();
    _options = properties->getCommandLineOptions();
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
IceBox::ServiceManagerI::run()
{
    try
    {
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
        // Load and initialize the services defined in the property set
        // with the prefix "IceBox.Service.". These properties should
        // have the following format:
        //
        // IceBox.Service.Foo=libFoo.so:create [args]
        //
        const string prefix = "IceBox.Service.";
        PropertiesPtr properties = _communicator->getProperties();
        StringSeq services = properties->getProperties(prefix);
        for (StringSeq::size_type i = 0; i < services.size(); i += 2)
        {
            string name = services[i].substr(prefix.size());
            string value = services[i + 1];

            //
            // Separate the entry point from the arguments.
            //
            string exec;
            StringSeq args;
            string::size_type pos = value.find_first_of(" \t\n");
            if (pos == string::npos)
            {
                exec = value;
            }
            else
            {
                exec = value.substr(0, pos);
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

            init(name, exec, args);
        }

        //
        // Invoke start() on the services.
        //
        map<string,ServiceInfo>::const_iterator r;
        for (r = _services.begin(); r != _services.end(); ++r)
        {
            try
            {
                (*r).second.service->start();
            }
            catch (const FailureException& ex)
            {
                throw;
            }
            catch (const Exception& ex)
            {
                FailureException ex;
                ex.reason = "ServiceManager: exception in start for service " + (*r).first + ": " + ex.ice_name();
                throw ex;
            }
        }

        //
        // We may want to notify external scripts that the services
        // have started. This is done by defining the property:
        //
        // IceBox.PrintServicesReady=bundleName
        //
        // Where bundleName is whatever you choose to call this set of
        // services. It will be echoed back as "bundleName ready".
        //
        // This must be done after start() has been invoked on the
        // services.
        //
        string bundleName = properties->getProperty("IceBox.PrintServicesReady");
        if (!bundleName.empty())
        {
            cout << bundleName << " ready" << endl;
        }

        //
        // Start request dispatching after we've started the services.
        //
        adapter->activate();

        _communicator->waitForShutdown();

        //
        // Invoke stop() on the services.
        //
        stopAll();
    }
    catch (const FailureException& ex)
    {
        Error out(_logger);
        out << ex.reason;
        stopAll();
        return EXIT_FAILURE;
    }
    catch (const Exception& ex)
    {
        Error out(_logger);
        out << "ServiceManager: " << ex;
        stopAll();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

IceBox::ServicePtr
IceBox::ServiceManagerI::init(const string& service, const string& exec, const StringSeq& args)
{
    //
    // We need to create a property set to pass to init().
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
    for (j = 0; j < _options.size(); j++)
    {
        if (_options[j].find("--" + service + ".") == 0)
        {
            l.push_back(_options[j]);
        }
    }
    for (j = 0; j < args.size(); j++)
    {
        l.push_back(args[j]);
    }
    for (j = 0; j < _argv.size(); j++)
    {
        if (_argv[j].find("--" + service + ".") == 0)
        {
            l.push_back(_argv[j]);
        }
    }

    //
    // Create the service property set.
    //
    addArgumentPrefix(service);
    int serviceArgc = static_cast<int>(l.size() + 1);
    char** serviceArgv = new char*[serviceArgc + 1];
    serviceArgv[0] = const_cast<char*>(_progName.c_str());
    int k;
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
    string::size_type colon = exec.rfind(':');
    if (colon == string::npos || colon == exec.size() - 1)
    {
        FailureException ex;
        ex.reason = "ServiceManager: invalid factory format `" + exec + "'";
        throw ex;
    }
    string libName = exec.substr(0, colon);
    string funcName = exec.substr(colon + 1);
    DynamicLibraryPtr library = new DynamicLibrary();
    if (!library->load(libName))
    {
        string msg = library->getErrorMessage();
        FailureException ex;
        ex.reason = "ServiceManager: unable to load library `" + libName + "'";
        if (!msg.empty())
        {
            ex.reason += ": " + msg;
        }
        throw ex;
    }

    //
    // Lookup the factory function and invoke it.
    //
    DynamicLibrary::symbol_type sym = library->getSymbol(funcName);
    if (sym == 0)
    {
        string msg = library->getErrorMessage();
        FailureException ex;
        ex.reason = "ServiceManager: unable to load symbol `" + funcName + "'";
        if (!msg.empty())
        {
            ex.reason += ": " + msg;
        }
        throw ex;
    }
    SERVICE_FACTORY factory = (SERVICE_FACTORY)sym;
    ServiceInfo info;
    try
    {
        info.service = factory(_communicator);
    }
    catch (const Exception& ex)
    {
        FailureException ex;
        ex.reason = "ServiceManager: exception in factory function `" + funcName + "': " + ex.ice_name();
        throw ex;
    }

    //
    // Invoke Service::init().
    //
    try
    {
        info.service->init(service, _communicator, serviceProperties, serviceArgs);
        info.library = library;
        _services[service] = info;
    }
    catch (const FailureException& ex)
    {
        throw;
    }
    catch (const Exception& ex)
    {
        FailureException ex;
        ex.reason = "ServiceManager: exception while initializing service " + service + ": " + ex.ice_name();
        throw ex;
    }

    return info.service;
}

void
IceBox::ServiceManagerI::stop(const string& service)
{
    map<string,ServiceInfo>::iterator r = _services.find(service);
    assert(r != _services.end());
    ServiceInfo info = (*r).second;
    _services.erase(r);

    try
    {
        info.service->stop();
    }
    catch (const Exception& ex)
    {
        //
        // Release the service before the library
        //
        info.service = 0;
        info.library = 0;

        FailureException ex;
        ex.reason = "ServiceManager: exception in stop for service " + service + ": " + ex.ice_name();
        throw ex;
    }

    //
    // Release the service before the library
    //
    info.service = 0;
    info.library = 0;
}

void
IceBox::ServiceManagerI::stopAll()
{
    map<string,ServiceInfo>::const_iterator r;
    for (r = _services.begin(); r != _services.end(); ++r)
    {
        try
        {
            stop((*r).first);
        }
        catch (const FailureException& ex)
        {
            Error out(_logger);
            out << ex.reason;
        }
    }
    assert(_services.empty());
}
