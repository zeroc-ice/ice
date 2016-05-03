// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/Instance.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

const char * const Ice::PluginManagerI::_kindOfObject = "plugin";

namespace
{

map<string, PLUGIN_FACTORY>* factories = 0;
vector<string>* loadOnInitialization = 0;

class PluginFactoryDestroy
{
public:

    ~PluginFactoryDestroy()
    {
        delete factories;
        factories = 0;

        delete loadOnInitialization;
        loadOnInitialization = 0;
    }
};
PluginFactoryDestroy destroy;

}

void
Ice::PluginManagerI::registerPluginFactory(const std::string& name, PLUGIN_FACTORY factory, bool loadOnInit)
{
    if(factories == 0)
    {
        factories = new map<string, PLUGIN_FACTORY>();
    }

    map<string, PLUGIN_FACTORY>::const_iterator p = factories->find(name);
    if(p == factories->end())
    {
        factories->insert(make_pair(name, factory));
        if(loadOnInit)
        {
            if(loadOnInitialization == 0)
            {
                loadOnInitialization = new vector<string>();
            }
            loadOnInitialization->push_back(name);
        }
    }
}

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
            try
            {
                p->plugin->initialize();
            }
            catch(const Ice::PluginInitializationException&)
            {
                throw;
            }
            catch(const std::exception& ex)
            {
                ostringstream os;
                os << "plugin `" << p->name << "' initialization failed:\n" << ex.what();
                throw PluginInitializationException(__FILE__, __LINE__, os.str());
            }
            catch(...)
            {
                ostringstream os;
                os << "plugin `" << p->name << "' initialization failed:\nunknown exception";
                throw PluginInitializationException(__FILE__, __LINE__, os.str());
            }
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

    const string prefix = "Ice.Plugin.";
    PropertiesPtr properties = _communicator->getProperties();
    PropertyDict plugins = properties->getPropertiesForPrefix(prefix);

    //
    // First, load static plugin factories which were setup to load on
    // communicator initialization. If a matching plugin property is
    // set, we load the plugin with the plugin specification. The
    // entryPoint will be ignored but the rest of the plugin
    // specification might be used.
    //
    if(loadOnInitialization)
    {
        for(vector<string>::const_iterator p = loadOnInitialization->begin(); p != loadOnInitialization->end(); ++p)
        {
            string property = prefix + *p;
            PropertyDict::iterator r = plugins.find(property + ".cpp");
            if(r == plugins.end())
            {
                r = plugins.find(property);
            }
            else
            {
                plugins.erase(property);
            }

            if(r != plugins.end())
            {
                loadPlugin(*p, r->second, cmdArgs);
                plugins.erase(r);
            }
            else
            {
                loadPlugin(*p, "", cmdArgs);
            }
        }
    }

    //
    // Next, load and initialize the plug-ins defined in the property
    // set with the prefix "Ice.Plugin.". These properties should have
    // the following format:
    //
    // Ice.Plugin.name[.<language>]=entry_point [args]
    //
    // If the Ice.PluginLoadOrder property is defined, load the
    // specified plug-ins in the specified order, then load any
    // remaining plug-ins.
    //
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

        string property = prefix + name;
        PropertyDict::iterator r = plugins.find(property + ".cpp");
        if(r == plugins.end())
        {
            r = plugins.find(property);
        }
        else
        {
            plugins.erase(property);
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

                plugins.erase(prefix + name);
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
            PropertyDict::iterator q = plugins.find(prefix + name + ".cpp");
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

    string entryPoint;
    StringSeq args;
    if(!pluginSpec.empty())
    {
        //
        // Split the entire property value into arguments. An entry point containing spaces
        // must be enclosed in quotes.
        //
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
        entryPoint = args[0];
        args.erase(args.begin());

        //
        // Convert command-line options into properties. First we
        // convert the options from the plug-in configuration, then
        // we convert the options from the application command-line.
        //
        PropertiesPtr properties = _communicator->getProperties();
        args = properties->parseCommandLineOptions(name, args);
        cmdArgs = properties->parseCommandLineOptions(name, cmdArgs);
    }

    PluginPtr plugin;
    PLUGIN_FACTORY factory = 0;
    DynamicLibraryPtr library;

    //
    // Always check the static plugin factory table first, it takes
    // precedence over the the entryPoint specified in the plugin
    // property value.
    //
    if(factories)
    {
        map<string, PLUGIN_FACTORY>::const_iterator p = factories->find(name);
        if(p != factories->end())
        {
            factory = p->second;
        }
    }

    //
    // If we didn't find the factory, get the factory using the entry
    // point symbol.
    //
    if(!factory)
    {
        assert(!entryPoint.empty());
        library = new DynamicLibrary();
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
#ifdef __IBMCPP__
	// xlC warns when casting a void* to function pointer
#pragma report(disable, "1540-0216")
#endif

        factory = reinterpret_cast<PLUGIN_FACTORY>(sym);
    }

    //
    // Invoke the factory function. No exceptions can be raised
    // by the factory function because it's declared extern "C".
    //
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

    if(library)
    {
        _libraries->add(library);
    }
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
