// **********************************************************************
//
// Copyright (c) 2002
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

#include <Ice/Ice.h>
#include <Ice/DynamicLibrary.h>
#include <IceBox/ServiceManagerI.h>
#include <Freeze/Initialize.h>

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
        // Create an object adapter. Services probably should NOT share
        // this object adapter, as the endpoint(s) for this object adapter
        // will most likely need to be firewalled for security reasons.
        //
        ObjectAdapterPtr adapter = _server->communicator()->createObjectAdapter("IceBox.ServiceManager");

	PropertiesPtr properties = _server->communicator()->getProperties();

	string identity = properties->getPropertyWithDefault("IceBox.ServiceManager.Identity", "ServiceManager");
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
	// Don't move after the adapter activation. This allow
	// applications to wait for the service manager to be
	// reachable before sending a signal to shutdown the
	// IceBox.
	//
	_server->shutdownOnInterrupt();

	try
	{
	    adapter->activate();
	}
	catch(const ObjectAdapterDeactivatedException&)
	{
	    //
	    // Expected if the communicator has been shutdown.
	    //
	}

        _server->communicator()->waitForShutdown();
	_server->ignoreInterrupt();

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
    // Create the service property set from the service arguments and
    // the server arguments. The service property set will be use to
    // create a new communicator or we be added to the shared
    // communicator depending on the value of the
    // IceBox.UseSharedCommunicator property.
    //
    StringSeq serviceArgs;
    StringSeq::size_type j;
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
    // Load the entry point.
    //
    DynamicLibraryPtr library = new DynamicLibrary();
    DynamicLibrary::symbol_type sym = library->loadEntryPoint(entryPoint);
    if(sym == 0)
    {
        string msg = library->getErrorMessage();
        FailureException ex(__FILE__, __LINE__);
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
        FailureException e(__FILE__, __LINE__);
        e.reason = "ServiceManager: exception in entry point `" + entryPoint + "': " + ex.ice_name();
        throw e;
    }
    catch (...)
    {
        FailureException e(__FILE__, __LINE__);
        e.reason = "ServiceManager: unknown exception in entry point `" + entryPoint + "'";
        throw e;
    }

    //
    // Invoke Service::start().
    //
    try
    {
	//
	// If Ice.UseSharedCommunicator.<name> is defined create a
	// communicator for the service. The communicator inherits
	// from the shared communicator properties. If it's not
	// defined, add the service properties to the shared
	// commnunicator property set.
	//
	PropertiesPtr properties = _server->communicator()->getProperties();
	if(properties->getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
	{
	    Ice::PropertiesPtr fileProperties = Ice::createProperties(serviceArgs);
	    properties->parseCommandLineOptions("", fileProperties->getCommandLineOptions());

	    serviceArgs = properties->parseCommandLineOptions("Ice", serviceArgs);
	    serviceArgs = properties->parseCommandLineOptions(service, serviceArgs);
	}
	else
	{
	    int argc = 0;
	    char **argv = 0;
	    
	    Ice::PropertiesPtr serviceProperties = properties->clone();

	    Ice::PropertiesPtr fileProperties = Ice::createProperties(serviceArgs);
	    serviceProperties->parseCommandLineOptions("", fileProperties->getCommandLineOptions());

	    serviceArgs = serviceProperties->parseCommandLineOptions("Ice", serviceArgs);
	    serviceArgs = serviceProperties->parseCommandLineOptions(service, serviceArgs);

	    info.communicator = initializeWithProperties(argc, argv, serviceProperties);
	}
	
	Ice::CommunicatorPtr communicator = info.communicator ? info.communicator : _server->communicator();

        ::IceBox::ServicePtr s = ::IceBox::ServicePtr::dynamicCast(info.service);

        if(s)
	{
	    //
	    // IceBox::Service
	    //
            s->start(service, communicator, serviceArgs);
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

	    info.dbEnv = ::Freeze::initialize(communicator, properties->getProperty("IceBox.DBEnvName." + service));

            fs->start(service, communicator, serviceArgs, info.dbEnv);
	}
        info.library = library;
        _services[service] = info;
    }
    catch(const Freeze::DBException& ex)
    {
        FailureException e(__FILE__, __LINE__);
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
        FailureException e(__FILE__, __LINE__);
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
	// Close the database environment if the service database
	// environment is set.
	//
	if(info.dbEnv)
	{
	    info.dbEnv->close();
	    info.dbEnv = 0;
	}
    }
    catch(const ::Freeze::DBException& ex)
    {
        info.service = 0;

	if(info.communicator)
	{
	    try
	    {
		info.communicator->destroy();
	    }
	    catch(const Ice::Exception&)
	    {
	    }
	    info.communicator = 0;
	}

        info.library = 0;

        FailureException e(__FILE__, __LINE__);
        e.reason = "ServiceManager: database exception in stop for service " + service + ": " + ex.ice_name() +
		   "\n" + ex.message;
        throw e;
    }
    catch(const Exception& ex)
    {
        info.service = 0;

	if(info.communicator)
	{
	    try
	    {
		info.communicator->destroy();
	    }
	    catch(const Ice::Exception&)
	    {
	    }
	    info.communicator = 0;
	}

        info.library = 0;

        FailureException e(__FILE__, __LINE__);
        e.reason = "ServiceManager: exception in stop for service " + service + ": " + ex.ice_name();
        throw e;
    }

    //
    // Release the service, the service communicator and then the
    // library. The order is important, the service must be release
    // before destroying the communicator so that the communicator
    // leak detector doesn't report potential leaks, and the
    // communicator must be destroyed before the library is released
    // since the library will destroy its global state.
    //
    info.service = 0;

    if(info.communicator)
    {
	try
	{
	    info.communicator->destroy();
	}
	catch(const Ice::Exception&)
	{
	}
	info.communicator = 0;
    }

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
            stop(string((*r++).first));
        }
        catch(const FailureException& ex)
        {
            Error out(_logger);
            out << ex.reason;
        }
    }
    assert(_services.empty());
}
