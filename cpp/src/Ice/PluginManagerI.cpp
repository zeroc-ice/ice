// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/PluginManagerI.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

typedef Ice::Plugin* (*PLUGIN_FACTORY)(const CommunicatorPtr&, const string&, const StringSeq&);

PluginPtr
Ice::PluginManagerI::getPlugin(const string& name)
{
    IceUtil::Mutex::Lock sync(*this);

    if(!_communicator)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    map<string, PluginInfo>::const_iterator r = _plugins.find(name);
    if(r != _plugins.end())
    {
        return (*r).second.plugin;
    }

    throw PluginNotFoundException(__FILE__, __LINE__);
}

void
Ice::PluginManagerI::addPlugin(const string& name, const PluginPtr& plugin)
{
    IceUtil::Mutex::Lock sync(*this);

    if(!_communicator)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    map<string, PluginInfo>::const_iterator r = _plugins.find(name);
    if(r != _plugins.end())
    {
        throw PluginExistsException(__FILE__, __LINE__);
    }
    PluginInfo info;
    info.plugin = plugin;
    _plugins[name] = info;
}

void
Ice::PluginManagerI::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_communicator)
    {
	map<string, PluginInfo>::iterator r;
	for(r = _plugins.begin(); r != _plugins.end(); ++r)
	{
	    r->second.plugin->destroy();
	    r->second.plugin = 0;
	    r->second.library = 0;
	}
	
	_communicator = 0;
    }
}

Ice::PluginManagerI::PluginManagerI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
Ice::PluginManagerI::loadPlugins(int& argc, char* argv[])
{
    assert(_communicator);

    StringSeq cmdArgs = argsToStringSeq(argc, argv);

    //
    // Load and initialize the plug-ins defined in the property set
    // with the prefix "Ice.Plugin.". These properties should
    // have the following format:
    //
    // Ice.Plugin.name=entry_point [args]
    //
    const string prefix = "Ice.Plugin.";
    PropertiesPtr properties = _communicator->getProperties();
    PropertyDict plugins = properties->getPropertiesForPrefix(prefix);
    PropertyDict::const_iterator p;
    for(p = plugins.begin(); p != plugins.end(); ++p)
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

        //
        // Convert command-line options into properties. First we
        // convert the options from the plug-in configuration, then
        // we convert the options from the application command-line.
        //
        args = properties->parseCommandLineOptions(name, args);
        cmdArgs = properties->parseCommandLineOptions(name, cmdArgs);

        loadPlugin(name, entryPoint, args);
    }

    stringSeqToArgs(cmdArgs, argc, argv);
}

void
Ice::PluginManagerI::loadPlugin(const string& name, const string& entryPoint, const StringSeq& args)
{
    assert(_communicator);

    //
    // Load the entry point symbol.
    //
    PluginInfo info;
    info.library = new DynamicLibrary();
    DynamicLibrary::symbol_type sym = info.library->loadEntryPoint(entryPoint);
    if(sym == 0)
    {
        ostringstream out;
        string msg = info.library->getErrorMessage();
        out << "unable to load entry point `" << entryPoint << "'";
        if(!msg.empty())
        {
            out << ": " + msg;
        }
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = out.str();
        throw ex;
    }

    //
    // Invoke the factory function.
    //
    PLUGIN_FACTORY factory = (PLUGIN_FACTORY)sym;
    try
    {
        info.plugin = factory(_communicator, name, args);
    }
    catch(const Exception& ex)
    {
        //
        // Do NOT propagate the exception from the entry point,
        // because the library will be closed.
        //
        ostringstream out;
        out << "exception in entry point `" << entryPoint << "'\n"
            << "original exception:\n"
            << ex;
        PluginInitializationException e(__FILE__, __LINE__);
        e.reason = out.str();
        throw e;
    }
    catch (...)
    {
        //
        // Do NOT propagate the exception from the entry point,
        // because the library will be closed.
        //
        ostringstream out;
        out << "unknown exception in entry point `" << entryPoint << "'\n";
        PluginInitializationException e(__FILE__, __LINE__);
        e.reason = out.str();
        throw e;
    }

    _plugins[name] = info;
}
