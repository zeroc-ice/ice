// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/DynamicLibrary.h>
#include <IceBox/ServiceManagerI.h>
#include <Freeze/Initialize.h>
#include <Freeze/DB.h>

using namespace Ice;
using namespace IceInternal;
using namespace std;

typedef IceBox::Service* (*SERVICE_FACTORY)(CommunicatorPtr);

IceBox::ServiceManagerI::ServiceManagerI(Application* server, int& argc, char* argv[])
    : _server(server)
{
    _logger = _server->communicator()->getLogger();

    if(argc > 0)
    {
        _progName = argv[0];
    }

    for(int i = 1; i < argc; i++)
    {
        _argv.push_back(argv[i]);
    }

    PropertiesPtr properties = _server->communicator()->getProperties();
    _options = properties->getCommandLineOptions();
}

IceBox::ServiceManagerI::~ServiceManagerI()
{
}

void
IceBox::ServiceManagerI::shutdown(const Current& current)
{
    _server->communicator()->shutdown();
}

int
IceBox::ServiceManagerI::run()
{
    try
    {
        ServiceManagerPtr obj = this;

	//
	// Prefix the adapter name and object identity with the value
	// of the IceBox.Name property.
	//
        PropertiesPtr properties = _server->communicator()->getProperties();
	string namePrefix = properties->getProperty("IceBox.Name");
	if(!namePrefix.empty())
	{
	    namePrefix += ".";
	}

        //
        // Create an object adapter. Services probably should NOT share
        // this object adapter, as the endpoint(s) for this object adapter
        // will most likely need to be firewalled for security reasons.
        //
        ObjectAdapterPtr adapter = 
	    _server->communicator()->createObjectAdapterFromProperty(namePrefix + "ServiceManagerAdapter",
								     "IceBox.ServiceManager.Endpoints");

	string identity = properties->getPropertyWithDefault("IceBox.ServiceManager.Identity", 
							     namePrefix + "ServiceManager");
        adapter->add(obj, stringToIdentity(identity));

        //
        // Load and start the services defined in the property set
        // with the prefix "IceBox.Service.". These properties should
        // have the following format:
        //
        // IceBox.Service.Foo=entry_point [args]
        //
        const string prefix = "IceBox.Service.";

        PropertyDict services = properties->getPropertiesForPrefix(prefix);
	PropertyDict::const_iterator p;
	for(p = services.begin(); p != services.end(); ++p)
	{
	    string name = p->first.substr(prefix.size());
	    const string& value = p->second;

            //
            // Separate the entry point from the arguments.
            //
            string entryPoint;
            StringSeq args;
            string::size_type pos = value.find_first_of(" \t\n");
            if(pos == string::npos)
            {
                entryPoint = value;
            }
            else
            {
                entryPoint = value.substr(0, pos);
                string::size_type beg = value.find_first_not_of(" \t\n", pos);
                while(beg != string::npos)
                {
                    string::size_type end = value.find_first_of(" \t\n", beg);
                    if(end == string::npos)
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

            start(name, entryPoint, args);
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
        if(!bundleName.empty())
        {
            cout << bundleName << " ready" << endl;
        }

        //
        // Start request dispatching after we've started the services.
        //
        adapter->activate();

	_server->shutdownOnInterrupt();
        _server->communicator()->waitForShutdown();

        //
        // Invoke stop() on the services.
        //
        stopAll();
    }
    catch(const FailureException& ex)
    {
        Error out(_logger);
        out << ex.reason;
        stopAll();
        return EXIT_FAILURE;
    }
    catch(const Exception& ex)
    {
        Error out(_logger);
        out << "ServiceManager: " << ex;
        stopAll();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
IceBox::ServiceManagerI::start(const string& service, const string& entryPoint, const StringSeq& args)
{
    //
    // We need to create a property set to pass to start().
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
    StringSeq serviceArgs;
    StringSeq::size_type j;
    for(j = 0; j < _options.size(); j++)
    {
        if(_options[j].find("--" + service + ".") == 0)
        {
            serviceArgs.push_back(_options[j]);
        }
    }
    for(j = 0; j < args.size(); j++)
    {
        serviceArgs.push_back(args[j]);
    }
    for(j = 0; j < _argv.size(); j++)
    {
        if(_argv[j].find("--" + service + ".") == 0)
        {
            serviceArgs.push_back(_argv[j]);
        }
    }

    //
    // Create the service property set.
    //
    PropertiesPtr serviceProperties = createProperties(serviceArgs);
    serviceArgs = serviceProperties->parseCommandLineOptions("Ice", serviceArgs);
    serviceArgs = serviceProperties->parseCommandLineOptions(service, serviceArgs);

    //
    // Load the entry point.
    //
    DynamicLibraryPtr library = new DynamicLibrary();
    DynamicLibrary::symbol_type sym = library->loadEntryPoint(entryPoint);
    if(sym == 0)
    {
        string msg = library->getErrorMessage();
        FailureException ex;
        ex.reason = "ServiceManager: unable to load entry point `" + entryPoint + "'";
        if(!msg.empty())
        {
            ex.reason += ": " + msg;
        }
        throw ex;
    }

    //
    // Invoke the factory function.
    //
    SERVICE_FACTORY factory = (SERVICE_FACTORY)sym;
    ServiceInfo info;
    try
    {
        info.service = factory(_server->communicator());
    }
    catch(const Exception& ex)
    {
        FailureException e;
        e.reason = "ServiceManager: exception in entry point `" + entryPoint + "': " + ex.ice_name();
        throw e;
    }
    catch (...)
    {
        FailureException e;
        e.reason = "ServiceManager: unknown exception in entry point `" + entryPoint + "'";
        throw e;
    }

    //
    // Invoke Service::start().
    //
    try
    {
        ::IceBox::ServicePtr s = ::IceBox::ServicePtr::dynamicCast(info.service);
        if(s)
	{
	    //
	    // IceBox::Service
	    //
            s->start(service, _server->communicator(), serviceProperties, serviceArgs);
	}
	else
	{
	    //
	    // IceBox::FreezeService
	    //
	    // Either open the database environment or if it has already been opened
	    // retrieve it from the map.
	    //
            ::IceBox::FreezeServicePtr fs = ::IceBox::FreezeServicePtr::dynamicCast(info.service);

	    PropertiesPtr properties = _server->communicator()->getProperties();
	    string propName = "IceBox.DBEnvName." + service;
	    info.dbEnvName = properties->getProperty(propName);

	    DBEnvironmentInfo dbInfo;
	    map<string,DBEnvironmentInfo>::iterator r = _dbEnvs.find(info.dbEnvName);
	    if(r == _dbEnvs.end())
	    {
	        dbInfo.dbEnv = ::Freeze::initialize(_server->communicator(), info.dbEnvName);
	        dbInfo.openCount = 1;
	    }
	    else
	    {
	        dbInfo = r->second;
		++dbInfo.openCount;
	    }
	    _dbEnvs[info.dbEnvName] = dbInfo;

            fs->start(service, _server->communicator(), serviceProperties, serviceArgs, dbInfo.dbEnv);
	}
        info.library = library;
        _services[service] = info;
    }
    catch(const Freeze::DBException& ex)
    {
        FailureException e;
        e.reason = "ServiceManager: database exception while starting service " + service + ": " + ex.ice_name() +
		   "\n" + ex.message;
        throw e;
    }
    catch(const FailureException&)
    {
        throw;
    }
    catch(const Exception& ex)
    {
        FailureException e;
        e.reason = "ServiceManager: exception while starting service " + service + ": " + ex.ice_name();
        throw e;
    }
}

void
IceBox::ServiceManagerI::stop(const string& service)
{
    map<string,ServiceInfo>::iterator r = _services.find(service);
    assert(r != _services.end());
    ServiceInfo info = r->second;
    _services.erase(r);

    try
    {
        info.service->stop();

	//
	// Close the database environment if service is a Freeze service and the 
	// database open count is one.
	//
        ::IceBox::FreezeServicePtr fs = IceBox::FreezeServicePtr::dynamicCast(info.service);
	if(fs)
	{
	    map<string,DBEnvironmentInfo>::iterator r = _dbEnvs.find(info.dbEnvName);
	    assert(r != _dbEnvs.end());
	    DBEnvironmentInfo dbInfo = r->second;
	    if(--dbInfo.openCount == 0)
	    {
	        dbInfo.dbEnv->close();
		_dbEnvs.erase(info.dbEnvName);
	    }
	    else
	    {
	        _dbEnvs[info.dbEnvName] = dbInfo;
	    }
	}
    }
    catch(const ::Freeze::DBException& ex)
    {
        //
        // Release the service before the library
        //
        info.service = 0;
        info.library = 0;

        FailureException e;
        e.reason = "ServiceManager: database exception in stop for service " + service + ": " + ex.ice_name() +
		   "\n" + ex.message;
        throw e;
    }
    catch(const Exception& ex)
    {
        //
        // Release the service before the library
        //
        info.service = 0;
        info.library = 0;

        FailureException e;
        e.reason = "ServiceManager: exception in stop for service " + service + ": " + ex.ice_name();
        throw e;
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
    map<string,ServiceInfo>::const_iterator r = _services.begin();
    while(r != _services.end())
    {
        try
        {
            stop((*r++).first);
        }
        catch(const FailureException& ex)
        {
            Error out(_logger);
            out << ex.reason;
        }
    }
    assert(_services.empty());
}
