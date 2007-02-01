// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

const char * const Ice::PluginManagerI::_kindOfObject = "plugin";

typedef Ice::Plugin* (*PLUGIN_FACTORY)(const CommunicatorPtr&, const string&, const StringSeq&);
typedef Ice::Logger* (*LOGGER_FACTORY)(const CommunicatorPtr&, const StringSeq&);

void
Ice::PluginManagerI::initializePlugins()
{
    if(_initialized)
    {
        InitializationException ex(__FILE__, __LINE__);
        ex.reason = "plugins already initialized";
        throw ex;
    }

    //
    // Invoke initialize() on the plugins, in the order they were loaded.
    //
    vector<PluginPtr> initializedPlugins;
    try
    {
        for(vector<PluginPtr>::iterator p = _initOrder.begin(); p != _initOrder.end(); ++p)
        {
            (*p)->initialize();
            initializedPlugins.push_back(*p);
        }
    }
    catch(...)
    {
        //
        // Destroy the plugins that have been successfully initialized, in the
        // reverse order.
        //
        for(vector<PluginPtr>::reverse_iterator p = initializedPlugins.rbegin(); p != initializedPlugins.rend(); ++p)
        {
            try
            {
                (*p)->destroy();
            }
            catch(...)
            {
                // Ignore.
            }
        }
        throw;
    }

    _initialized = true;
}

PluginPtr
Ice::PluginManagerI::getPlugin(const string& name)
{
    IceUtil::Mutex::Lock sync(*this);

    if(!_communicator)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    map<string, PluginPtr>::const_iterator r = _plugins.find(name);
    if(r != _plugins.end())
    {
        return (*r).second;
    }

    NotRegisteredException ex(__FILE__, __LINE__);
    ex.kindOfObject = _kindOfObject;
    ex.id = name;
    throw ex;
}

void
Ice::PluginManagerI::addPlugin(const string& name, const PluginPtr& plugin)
{
    IceUtil::Mutex::Lock sync(*this);

    if(!_communicator)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    map<string, PluginPtr>::const_iterator r = _plugins.find(name);
    if(r != _plugins.end())
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = _kindOfObject;
        ex.id = name;
        throw ex;
    }
    _plugins[name] = plugin;
}

void
Ice::PluginManagerI::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_communicator)
    {
        map<string, PluginPtr>::iterator r;
        for(r = _plugins.begin(); r != _plugins.end(); ++r)
        {
            r->second->destroy();
            r->second = 0;
        }
        
        _logger = 0;
        _communicator = 0;
    }

    _libraries = 0;
}

Ice::PluginManagerI::PluginManagerI(const CommunicatorPtr& communicator, const DynamicLibraryListPtr& libraries) :
    _communicator(communicator),
    _libraries(libraries),
    _initialized(false)
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
    // If the Ice.PluginLoadOrder property is defined, load the
    // specified plugins in the specified order, then load any
    // remaining plugins.
    //
    const string prefix = "Ice.Plugin.";
    PropertiesPtr properties = _communicator->getProperties();
    PropertyDict plugins = properties->getPropertiesForPrefix(prefix);

    string loadOrder = properties->getProperty("Ice.PluginLoadOrder");
    string::size_type beg = 0;
    if(!loadOrder.empty())
    {
        const string delim = ", \t\n";
        beg = loadOrder.find_first_not_of(delim, beg);
        while(beg != string::npos)
        {
            string name;
            string::size_type end = loadOrder.find_first_of(delim, beg);
            if(end == string::npos)
            {
                name = loadOrder.substr(beg);
                beg = end;
            }
            else
            {
                name = loadOrder.substr(beg, end - beg);
                beg = loadOrder.find_first_not_of(delim, end);
            }

            map<string, PluginPtr>::iterator p = _plugins.find(name);
            if(p != _plugins.end())
            {
                PluginInitializationException ex(__FILE__, __LINE__);
                ex.reason = "plugin `" + name + "' already loaded";
                throw ex;
            }

            PropertyDict::iterator q = plugins.find("Ice.Plugin." + name);
            if(q != plugins.end())
            {
                loadPlugin(name, q->second, cmdArgs, false);
                plugins.erase(q);
            }
            else
            {
                PluginInitializationException ex(__FILE__, __LINE__);
                ex.reason = "plugin `" + name + "' not defined";
                throw ex;
            }
        }
    }

    //
    // Load any remaining plugins that weren't specified in PluginLoadOrder.
    //
    PropertyDict::const_iterator p;
    for(p = plugins.begin(); p != plugins.end(); ++p)
    {
        string name = p->first.substr(prefix.size());
        loadPlugin(name, p->second, cmdArgs, false);
    }

    string loggerStr = properties->getProperty("Ice.LoggerPlugin");
    if(loggerStr.length() != 0)
    {
        loadPlugin("Logger", loggerStr, cmdArgs, true);
    }

    stringSeqToArgs(cmdArgs, argc, argv);

    //
    // An application can set Ice.InitPlugins=0 if it wants to postpone
    // initialization until after it has interacted directly with the
    // plugins.
    //
    if(properties->getPropertyAsIntWithDefault("Ice.InitPlugins", 1) > 0)
    {
        initializePlugins();
    }
}

void
Ice::PluginManagerI::loadPlugin(const string& name, const string& pluginSpec, StringSeq& cmdArgs, bool isLogger)
{
    assert(_communicator);

    //
    // Separate the entry point from the arguments.
    //
    string entryPoint;
    StringSeq args;
    const string delim = " \t\n";
    string::size_type pos = pluginSpec.find_first_of(delim);
    if(pos == string::npos)
    {
        entryPoint = pluginSpec;
    }
    else
    {
        entryPoint = pluginSpec.substr(0, pos);
        string::size_type beg = pluginSpec.find_first_not_of(delim, pos);
        while(beg != string::npos)
        {
            string::size_type end = pluginSpec.find_first_of(delim, beg);
            if(end == string::npos)
            {
                args.push_back(pluginSpec.substr(beg));
                beg = end;
            }
            else
            {
                args.push_back(pluginSpec.substr(beg, end - beg));
                beg = pluginSpec.find_first_not_of(delim, end);
            }
        }
    }

    //
    // Convert command-line options into properties. First we
    // convert the options from the plug-in configuration, then
    // we convert the options from the application command-line.
    //
    PropertiesPtr properties = _communicator->getProperties();
    args = properties->parseCommandLineOptions(name, args);
    cmdArgs = properties->parseCommandLineOptions(name, cmdArgs);

    //
    // Load the entry point symbol.
    //
    PluginPtr plugin;
    DynamicLibraryPtr library = new DynamicLibrary();
    DynamicLibrary::symbol_type sym = library->loadEntryPoint(entryPoint);
    if(sym == 0)
    {
        ostringstream out;
        string msg = library->getErrorMessage();
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
    // Invoke the factory function. No exceptions can be raised
    // by the factory function because it's declared extern "C".
    //
    if(isLogger)
    {
        LOGGER_FACTORY factory = (LOGGER_FACTORY)sym;
        _logger = factory(_communicator, args);
        if(!_logger)
        {
            PluginInitializationException e(__FILE__, __LINE__);
            ostringstream out;
            out << "failure in entry point `" << entryPoint << "'";
            e.reason = out.str();
            throw e;
        }
    }
    else
    {
        PLUGIN_FACTORY factory = (PLUGIN_FACTORY)sym;
        plugin = factory(_communicator, name, args);
        if(!plugin)
        {
            PluginInitializationException e(__FILE__, __LINE__);
            ostringstream out;
            out << "failure in entry point `" << entryPoint << "'";
            e.reason = out.str();
            throw e;
        }

        _plugins[name] = plugin;
        _initOrder.push_back(plugin);
    }
    _libraries->add(library);
}

Ice::LoggerPtr
Ice::PluginManagerI::getLogger() const
{
    return _logger;
}

