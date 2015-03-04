// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/PluginManagerI.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Initialize.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

const char * const Ice::PluginManagerI::_kindOfObject = "plugin";

typedef Ice::Plugin* (*PLUGIN_FACTORY)(const CommunicatorPtr&, const string&, const StringSeq&);

void
Ice::PluginManagerI::initializePlugins()
{
    if(_initialized)
    {
        InitializationException ex(__FILE__, __LINE__);
        ex.reason = "plug-ins already initialized";
        throw ex;
    }

    //
    // Invoke initialize() on the plug-ins, in the order they were loaded.
    //
    vector<PluginPtr> initializedPlugins;
    try
    {
        for(PluginInfoList::iterator p = _plugins.begin(); p != _plugins.end(); ++p)
        {
            p->plugin->initialize();
            initializedPlugins.push_back(p->plugin);
        }
    }
    catch(...)
    {
        //
        // Destroy the plug-ins that have been successfully initialized, in the
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

StringSeq
Ice::PluginManagerI::getPlugins()
{
    IceUtil::Mutex::Lock sync(*this);

    StringSeq names;
    for(PluginInfoList::iterator p = _plugins.begin(); p != _plugins.end(); ++p)
    {
        names.push_back(p->name);
    }
    return names;
}

PluginPtr
Ice::PluginManagerI::getPlugin(const string& name)
{
    IceUtil::Mutex::Lock sync(*this);

    if(!_communicator)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    PluginPtr p = findPlugin(name);
    if(p)
    {
        return p;
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

    if(findPlugin(name))
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = _kindOfObject;
        ex.id = name;
        throw ex;
    }

    PluginInfo info;
    info.name = name;
    info.plugin = plugin;
    _plugins.push_back(info);
}

void
Ice::PluginManagerI::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_communicator)
    {
        if(_initialized)
        {

            //
            // Destroy the plug-ins that have been successfully initialized, in the
            // reverse order.
            //
            for(PluginInfoList::reverse_iterator p = _plugins.rbegin(); p != _plugins.rend(); ++p)
            {
                try
                {
                    p->plugin->destroy();
                }
                catch(const std::exception& ex)
                {
                    Warning out(getProcessLogger());
                    out << "unexpected exception raised by plug-in `" << p->name << "' destruction:\n" << ex.what();
                }
                catch(const std::string& str)
                {
                    Warning out(getProcessLogger());
                    out << "unexpected exception raised by plug-in `" << p->name << "' destruction:\n" << str;
                }
                catch(const char* msg)
                {
                    Warning out(getProcessLogger());
                    out << "unexpected exception raised by plug-in `" << p->name << "' destruction:\n" << msg;
                }
                catch(...)
                {
                    Warning out(getProcessLogger());
                    out << "unexpected exception raised by plug-in `" << p->name << "' destruction";
                }
            }
        }

        _communicator = 0;
    }

    _plugins.clear();
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
    // Ice.Plugin.name[.<language>]=entry_point [args]
    //
    // If the Ice.PluginLoadOrder property is defined, load the
    // specified plug-ins in the specified order, then load any
    // remaining plug-ins.
    //
    const string prefix = "Ice.Plugin.";
    PropertiesPtr properties = _communicator->getProperties();
    PropertyDict plugins = properties->getPropertiesForPrefix(prefix);

    StringSeq loadOrder = properties->getPropertyAsList("Ice.PluginLoadOrder");
    for(StringSeq::const_iterator p = loadOrder.begin(); p != loadOrder.end(); ++p)
    {
        string name = *p;

        if(findPlugin(name))
        {
            PluginInitializationException ex(__FILE__, __LINE__);
            ex.reason = "plug-in `" + name + "' already loaded";
            throw ex;
        }

        PropertyDict::iterator r = plugins.find("Ice.Plugin." + name + ".cpp");
        if(r == plugins.end())
        {
            r = plugins.find("Ice.Plugin." + name);
        }
        else
        {
            plugins.erase("Ice.Plugin." + name);
        }

        if(r != plugins.end())
        {
            loadPlugin(name, r->second, cmdArgs);
            plugins.erase(r);
        }
        else
        {
            PluginInitializationException ex(__FILE__, __LINE__);
            ex.reason = "plug-in `" + name + "' not defined";
            throw ex;
        }
    }

    //
    // Load any remaining plug-ins that weren't specified in PluginLoadOrder.
    //

    while(!plugins.empty())
    {
        PropertyDict::iterator p = plugins.begin();

        string name = p->first.substr(prefix.size());

        size_t dotPos = name.find_last_of('.');
        if(dotPos != string::npos)
        {
            string suffix = name.substr(dotPos + 1);
            if(suffix == "java" || suffix == "clr")
            {
                //
                // Ignored
                //
                plugins.erase(p);
            }
            else if(suffix == "cpp")
            {
                name = name.substr(0, dotPos);
                loadPlugin(name, p->second, cmdArgs);
                plugins.erase(p);

                plugins.erase("Ice.Plugin." + name);
            }
            else
            {
                //
                // Name is just a regular name that happens to contain a dot
                //
                dotPos = string::npos;
            }
        }

        if(dotPos == string::npos)
        {
            //
            // Is there a .cpp entry?
            //
            PropertyDict::iterator q = plugins.find("Ice.Plugin." + name + ".cpp");
            if(q != plugins.end())
            {
                plugins.erase(p);
                p = q;
            }

            loadPlugin(name, p->second, cmdArgs);
            plugins.erase(p);
        }
    }

    stringSeqToArgs(cmdArgs, argc, argv);
}

void
Ice::PluginManagerI::loadPlugin(const string& name, const string& pluginSpec, StringSeq& cmdArgs)
{
    assert(_communicator);
    //
    // Split the entire property value into arguments. An entry point containing spaces
    // must be enclosed in quotes.
    //
    StringSeq args;
    try
    {
        args = IceUtilInternal::Options::split(pluginSpec);
    }
    catch(const IceUtilInternal::BadOptException& ex)
    {
        PluginInitializationException e(__FILE__, __LINE__);
        e.reason = "invalid arguments for plug-in `" + name + "':\n" + ex.reason;
        throw e;
    }

    assert(!args.empty());

    //
    // Shift the arguments.
    //
    const string entryPoint = args[0];
    args.erase(args.begin());

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
    DynamicLibraryPtr library = new DynamicLibrary(IceInternal::getInstance(_communicator)->initializationData().stringConverter);
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

    PluginInfo info;
    info.name = name;
    info.plugin = plugin;
    _plugins.push_back(info);

    _libraries->add(library);
}

Ice::PluginPtr
Ice::PluginManagerI::findPlugin(const string& name) const
{
    for(PluginInfoList::const_iterator p = _plugins.begin(); p != _plugins.end(); ++p)
    {
        if(name == p->name)
        {
            return p->plugin;
        }
    }
    return 0;
}

void
IceInternal::loadPlugin(const Ice::CommunicatorPtr& communicator,
                        const string& name,
                        const string& pluginSpec,
                        Ice::StringSeq& cmdArgs)
{
    PluginManagerIPtr pluginManager = PluginManagerIPtr::dynamicCast(getInstance(communicator)->pluginManager());
    pluginManager->loadPlugin(name, pluginSpec, cmdArgs);
}
