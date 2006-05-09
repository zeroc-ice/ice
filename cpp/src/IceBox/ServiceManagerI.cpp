// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/SliceChecksums.h>
#include <IceBox/ServiceManagerI.h>

using namespace Ice;
using namespace IceBox;
using namespace std;

typedef IceBox::Service* (*SERVICE_FACTORY)(CommunicatorPtr);

IceBox::ServiceManagerI::ServiceManagerI(CommunicatorPtr communicator, int& argc, char* argv[])
    : _communicator(communicator)
{ 
    _logger = _communicator->getLogger();

    for(int i = 1; i < argc; i++)
    {
        _argv.push_back(argv[i]);
    }
}

IceBox::ServiceManagerI::~ServiceManagerI()
{
}

SliceChecksumDict
IceBox::ServiceManagerI::getSliceChecksums(const Current&) const
{
    return sliceChecksums();
}

void
IceBox::ServiceManagerI::shutdown(const Current& current)
{
    _communicator->shutdown();
}

bool
IceBox::ServiceManagerI::start()
{
    try
    {
        ServiceManagerPtr obj = this;

        //
        // Create an object adapter. Services probably should NOT share
        // this object adapter, as the endpoint(s) for this object adapter
        // will most likely need to be firewalled for security reasons.
        //
        ObjectAdapterPtr adapter = _communicator->createObjectAdapter("IceBox.ServiceManager");

	PropertiesPtr properties = _communicator->getProperties();
        string identity = properties->getProperty("IceBox.ServiceManager.Identity");
        if(identity.empty())
        {
            identity = properties->getPropertyWithDefault("IceBox.InstanceName", "IceBox") + "/ServiceManager";
        }
        adapter->add(obj, _communicator->stringToIdentity(identity));

        //
        // Parse the IceBox.LoadOrder property.
        //
        string order = properties->getProperty("IceBox.LoadOrder");
        StringSeq loadOrder;
        if(!order.empty())
        {
            string::size_type beg = order.find_first_not_of(",\t ");
            while(beg != string::npos)
            {
                string::size_type end = order.find_first_of(",\t ", beg);
                if(end == string::npos)
                {
                    loadOrder.push_back(order.substr(beg));
                    beg = end;
                }
                else
                {
                    loadOrder.push_back(order.substr(beg, end - beg));
                    beg = order.find_first_not_of(",\t ", end);
                }
            }
        }

        //
        // Load and start the services defined in the property set
        // with the prefix "IceBox.Service.". These properties should
        // have the following format:
        //
        // IceBox.Service.Foo=entry_point [args]
        //
        // We load the services specified in IceBox.LoadOrder first,
        // then load any remaining services.
        //
        const string prefix = "IceBox.Service.";
        PropertyDict services = properties->getPropertiesForPrefix(prefix);
	PropertyDict::iterator p;
        for(StringSeq::const_iterator q = loadOrder.begin(); q != loadOrder.end(); ++q)
        {
            p = services.find(prefix + *q);
            if(p == services.end())
            {
                FailureException ex(__FILE__, __LINE__);
                ex.reason = "ServiceManager: no service definition for `" + *q + "'";
                throw ex;
            }
            load(*q, p->second);
            services.erase(p);
        }
	for(p = services.begin(); p != services.end(); ++p)
	{
            string name = p->first.substr(prefix.size());
            load(name, p->second);
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
    }
    catch(const FailureException& ex)
    {
        Error out(_logger);
        out << ex.reason;
        stopAll();
        return false;
    }
    catch(const Exception& ex)
    {
        Error out(_logger);
        out << "ServiceManager: " << ex;
        stopAll();
        return false;
    }

    return true;
}

void
IceBox::ServiceManagerI::stop()
{
    stopAll();
}

void
IceBox::ServiceManagerI::load(const string& name, const string& value)
{
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
	try
	{
	    args = IceUtil::Options::split(value.substr(pos + 1));
	}
	catch(const IceUtil::Options::BadQuote& ex)
	{
	    FailureException e(__FILE__, __LINE__);
	    e.reason = "ServiceManager: invalid arguments for service `" + name + "':\n" + ex.reason;
	    throw e;	    
	}
    }
    start(name, entryPoint, args);
}

void
IceBox::ServiceManagerI::start(const string& service, const string& entryPoint, const StringSeq& args)
{
    //
    // Create the service property set from the service arguments and
    // the server arguments. The service property set will be used to
    // create a new communicator, or will be added to the shared
    // communicator, depending on the value of the
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
    IceInternal::DynamicLibraryPtr library = new IceInternal::DynamicLibrary();
    IceInternal::DynamicLibrary::symbol_type sym = library->loadEntryPoint(entryPoint, false);
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
        info.service = factory(_communicator);
    }
    catch(const Exception& ex)
    {
        FailureException e(__FILE__, __LINE__);
        e.reason = "ServiceManager: exception in entry point `" + entryPoint + "': " + ex.ice_name();
        throw e;
    }
    catch(...)
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
	// If Ice.UseSharedCommunicator.<name> is not defined, create
	// a communicator for the service. The communicator inherits
	// from the shared communicator properties. If it's defined
	// add the service properties to the shared commnunicator
	// property set.
	//
	PropertiesPtr properties = _communicator->getProperties();
	if(properties->getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
	{
	    PropertiesPtr fileProperties = createProperties(serviceArgs);
	    properties->parseCommandLineOptions("", fileProperties->getCommandLineOptions());

	    serviceArgs = properties->parseIceCommandLineOptions(serviceArgs);
	    serviceArgs = properties->parseCommandLineOptions(service, serviceArgs);
	}
	else
	{	
	    PropertiesPtr serviceProperties = properties->clone();

	    //
	    //  Append the service name to the program name if not empty.
	    //
	    string name = serviceProperties->getProperty("Ice.ProgramName");
	    if(name != service)
	    {
		name = name.empty() ? service : name + "-" + service;
	    }

	    //
	    // Load property file eventually specified with
	    // --Ice.Config and add the properties from the file to
	    // the service properties.
	    //
	    PropertiesPtr fileProperties = createProperties(serviceArgs);
	    serviceProperties->parseCommandLineOptions("", fileProperties->getCommandLineOptions());

	    serviceProperties->setProperty("Ice.ProgramName", name);

	    //
	    // Parse Ice and <service>.* command line options.
	    //
	    serviceArgs = serviceProperties->parseIceCommandLineOptions(serviceArgs);
	    serviceArgs = serviceProperties->parseCommandLineOptions(service, serviceArgs);

	    //
	    // Remaining command line options are passed to the
	    // communicator with argc/argv. This is necessary for Ice
	    // plugin properties (e.g.: IceSSL).
	    //
	    int argc = static_cast<int>(serviceArgs.size());
	    char** argv = new char*[argc + 1];
	    int i = 0;
	    for(Ice::StringSeq::const_iterator p = serviceArgs.begin(); p != serviceArgs.end(); ++p, ++i)
	    {
		argv[i] = strdup(p->c_str());
	    }
	    argv[argc] = 0;

	    InitializationData initData;
	    initData.properties = serviceProperties;
	    info.communicator = initialize(argc, argv, initData);

	    for(i = 0; i < argc + 1; ++i)
	    {
		free(argv[i]);
	    }
	    delete[] argv;
	}
	
	CommunicatorPtr communicator = info.communicator ? info.communicator : _communicator;

	//
	// Start the service.
	//
	try
	{
	    info.service->start(service, communicator, serviceArgs);
	}
	catch(...)
	{
	    if(info.communicator)
	    {
		try
		{
		    info.communicator->shutdown();
		    info.communicator->waitForShutdown();
		}
		catch(const Ice::CommunicatorDestroyedException&)
		{
		    //
		    // Ignore, the service might have already destroyed
		    // the communicator for its own reasons.
		    //
		}
		catch(const Ice::Exception& ex)
		{
		    Warning out(_logger);
		    out << "ServiceManager: exception in shutting down communicator for service " << service << ":\n";
		    out << ex;
		}

		try
		{
		    info.communicator->destroy();
		    info.communicator = 0;
		}
		catch(const Exception& ex)
		{
		    Warning out(_logger);
		    out << "ServiceManager: exception in shutting down communicator for service " << service << ":\n";
		    out << ex;
		}
	    }
	    throw;
	}

        info.library = library;
        _services[service] = info;
    }
    catch(const FailureException&)
    {
        throw;
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << "ServiceManager: exception while starting service " << service << ":\n";
	s << ex;

	FailureException e(__FILE__, __LINE__);
	e.reason = s.str();
        throw e;
    }
}

void
IceBox::ServiceManagerI::stopAll()
{
    map<string,ServiceInfo>::iterator p;

    //
    // First, for each service, we call stop on the service and flush its database environment to 
    // the disk.
    //
    for(p = _services.begin(); p != _services.end(); ++p)
    {
	ServiceInfo& info = p->second;
	try
	{
	    info.service->stop();
	}
	catch(const Ice::Exception& ex)
	{
            Warning out(_logger);
	    out << "ServiceManager: exception in stop for service " << p->first << ":\n";
	    out << ex;
	}
	catch(...)
	{
            Warning out(_logger);
	    out << "ServiceManager: unknown exception in stop for service " << p->first;
	}
    }

    for(p = _services.begin(); p != _services.end(); ++p)
    {
	ServiceInfo& info = p->second;

	if(info.communicator)
	{
	    try
	    {
		info.communicator->shutdown();
		info.communicator->waitForShutdown();
	    }
	    catch(const Ice::CommunicatorDestroyedException&)
	    {
		//
		// Ignore, the service might have already destroyed
		// the communicator for its own reasons.
		//
	    }
	    catch(const Ice::Exception& ex)
	    {
		Warning out(_logger);
		out << "ServiceManager: exception in stop for service " << p->first << ":\n";
		out << ex;
	    }
	}

	//
	// Release the service, the service communicator and then the library. The order is important, 
	// the service must be released before destroying the communicator so that the communicator
	// leak detector doesn't report potential leaks, and the communicator must be destroyed before 
	// the library is released since the library will destroy its global state.
	//
	try
	{
	    info.service = 0;
	}
	catch(const Exception& ex)
	{
	    Warning out(_logger);
	    out << "ServiceManager: exception in stop for service " << p->first << ":\n";
	    out << ex;
	}
	catch(...)
	{
            Warning out(_logger);
	    out << "ServiceManager: unknown exception in stop for service " << p->first;
	}

	if(info.communicator)
	{
	    try
	    {
		info.communicator->destroy();
		info.communicator = 0;
	    }
	    catch(const Exception& ex)
	    {
		Warning out(_logger);
		out << "ServiceManager: exception in stop for service " << p->first << ":\n";
		out << ex;
	    }
	}
	
	try
	{
	    info.library = 0;
	}
	catch(const Exception& ex)
	{
	    Warning out(_logger);
	    out << "ServiceManager: exception in stop for service " << p->first << ":\n";
	    out << ex;
	}
	catch(...)
	{
            Warning out(_logger);
	    out << "ServiceManager: unknown exception in stop for service " << p->first;
	}
    }

    _services.clear();
}

