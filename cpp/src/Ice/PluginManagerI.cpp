//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "PluginManagerI.h"
#include "DynamicLibrary.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Instance.h"
#include "Options.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

const char* const Ice::PluginManagerI::_kindOfObject = "plugin";

namespace
{
    map<string, PluginFactory>* factories = nullptr;
    vector<string>* loadOnInitialization = nullptr;

    class PluginFactoryDestroy
    {
    public:
        ~PluginFactoryDestroy()
        {
            delete factories;
            factories = nullptr;

            delete loadOnInitialization;
            loadOnInitialization = nullptr;
        }
    };
    PluginFactoryDestroy destroy;
}

void
Ice::PluginManagerI::registerPluginFactory(std::string name, PluginFactory factory, bool loadOnInit)
{
    if (!factories)
    {
        factories = new map<string, PluginFactory>();
    }

    auto p = factories->find(name);
    if (p == factories->end())
    {
        factories->insert(make_pair(name, factory));
        if (loadOnInit)
        {
            if (!loadOnInitialization)
            {
                loadOnInitialization = new vector<string>();
            }
            loadOnInitialization->push_back(std::move(name));
        }
    }
}

void
Ice::PluginManagerI::initializePlugins()
{
    if (_initialized)
    {
        throw InitializationException(__FILE__, __LINE__, "plug-ins already initialized");
    }

    //
    // Invoke initialize() on the plug-ins, in the order they were loaded.
    //
    vector<PluginPtr> initializedPlugins;
    try
    {
        for (PluginInfoList::iterator p = _plugins.begin(); p != _plugins.end(); ++p)
        {
            try
            {
                p->plugin->initialize();
            }
            catch (const Ice::PluginInitializationException&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                ostringstream os;
                os << "plugin '" << p->name << "' initialization failed:\n" << ex.what();
                throw PluginInitializationException(__FILE__, __LINE__, os.str());
            }
            catch (...)
            {
                ostringstream os;
                os << "plugin '" << p->name << "' initialization failed:\nunknown exception";
                throw PluginInitializationException(__FILE__, __LINE__, os.str());
            }
            initializedPlugins.push_back(p->plugin);
        }
    }
    catch (...)
    {
        //
        // Destroy the plug-ins that have been successfully initialized, in the
        // reverse order.
        //
        for (vector<PluginPtr>::reverse_iterator p = initializedPlugins.rbegin(); p != initializedPlugins.rend(); ++p)
        {
            try
            {
                (*p)->destroy();
            }
            catch (...)
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
    lock_guard lock(_mutex);

    StringSeq names;
    for (PluginInfoList::iterator p = _plugins.begin(); p != _plugins.end(); ++p)
    {
        names.push_back(p->name);
    }
    return names;
}

PluginPtr
Ice::PluginManagerI::getPlugin(string_view name)
{
    lock_guard lock(_mutex);

    if (!_communicator)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    PluginPtr p = findPlugin(name);
    if (p)
    {
        return p;
    }

    throw NotRegisteredException(__FILE__, __LINE__, _kindOfObject, string{name});
}

void
Ice::PluginManagerI::addPlugin(string name, PluginPtr plugin)
{
    lock_guard lock(_mutex);

    if (!_communicator)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if (findPlugin(name))
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, _kindOfObject, name);
    }

    PluginInfo info;
    info.name = std::move(name);
    info.plugin = std::move(plugin);
    _plugins.push_back(std::move(info));
}

void
Ice::PluginManagerI::destroy() noexcept
{
    lock_guard lock(_mutex);

    if (_communicator)
    {
        if (_initialized)
        {
            //
            // Destroy the plug-ins that have been successfully initialized, in the
            // reverse order.
            //
            for (PluginInfoList::reverse_iterator p = _plugins.rbegin(); p != _plugins.rend(); ++p)
            {
                try
                {
                    p->plugin->destroy();
                }
                catch (const std::exception& ex)
                {
                    Warning out(getProcessLogger());
                    out << "unexpected exception raised by plug-in '" << p->name << "' destruction:\n" << ex.what();
                }
                catch (...)
                {
                    Warning out(getProcessLogger());
                    out << "unexpected exception raised by plug-in '" << p->name << "' destruction";
                }
            }
        }

        _communicator = nullptr;
    }

    _plugins.clear();
}

Ice::PluginManagerI::PluginManagerI(const CommunicatorPtr& communicator)
    : _communicator(communicator),
      _initialized(false)
{
}

bool
Ice::PluginManagerI::loadPlugins(int& argc, const char* argv[])
{
    assert(_communicator);

    bool libraryLoaded = false;

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
    if (loadOnInitialization)
    {
        for (vector<string>::const_iterator p = loadOnInitialization->begin(); p != loadOnInitialization->end(); ++p)
        {
            string property = prefix + *p;
            PropertyDict::iterator r = plugins.find(property);
            if (r != plugins.end())
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
    // Ice.Plugin.<name>=entry_point [args]
    //
    // If the Ice.PluginLoadOrder property is defined, load the
    // specified plug-ins in the specified order, then load any
    // remaining plug-ins.
    //
    StringSeq loadOrder = properties->getIcePropertyAsList("Ice.PluginLoadOrder");
    for (StringSeq::const_iterator p = loadOrder.begin(); p != loadOrder.end(); ++p)
    {
        string name = *p;

        if (findPlugin(name))
        {
            throw PluginInitializationException(__FILE__, __LINE__, "plug-in '" + name + "' already loaded");
        }

        string property = prefix + name;
        PropertyDict::iterator r = plugins.find(property);
        if (r != plugins.end())
        {
            libraryLoaded |= loadPlugin(name, r->second, cmdArgs);
            plugins.erase(r);
        }
        else
        {
            throw PluginInitializationException(__FILE__, __LINE__, "plug-in '" + name + "' not defined");
        }
    }

    //
    // Load any remaining plug-ins that weren't specified in PluginLoadOrder.
    //

    for (const auto& [key, value] : plugins)
    {
        libraryLoaded |= loadPlugin(key.substr(prefix.size()), value, cmdArgs);
    }
    stringSeqToArgs(cmdArgs, argc, argv);

    return libraryLoaded;
}

bool
Ice::PluginManagerI::loadPlugin(const string& name, const string& pluginSpec, StringSeq& cmdArgs)
{
    assert(_communicator);

    bool libraryLoaded = false;

    string entryPoint;
    StringSeq args;
    if (!pluginSpec.empty())
    {
        //
        // Split the entire property value into arguments. An entry point containing spaces
        // must be enclosed in quotes.
        //
        try
        {
            args = IceInternal::Options::split(pluginSpec);
        }
        catch (const IceInternal::BadOptException& ex)
        {
            throw PluginInitializationException(
                __FILE__,
                __LINE__,
                "invalid arguments for plug-in '" + name + "':\n" + string{ex.what()});
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

    PluginFactory factory = nullptr;

    //
    // Always check the static plugin factory table first, it takes
    // precedence over the entryPoint specified in the plugin
    // property value.
    //
    if (factories)
    {
        map<string, PluginFactory>::const_iterator p = factories->find(name);
        if (p != factories->end())
        {
            factory = p->second;
        }
    }

    //
    // If we didn't find the factory, get the factory using the entry
    // point symbol.
    //
    if (!factory)
    {
        assert(!entryPoint.empty());
        DynamicLibrary library;
        DynamicLibrary::symbol_type sym = library.loadEntryPoint(entryPoint);
        if (sym == nullptr)
        {
            ostringstream os;
            string msg = library.getErrorMessage();
            os << "unable to load entry point '" << entryPoint << "'";
            if (!msg.empty())
            {
                os << ": " + msg;
            }
            throw PluginInitializationException(__FILE__, __LINE__, os.str());
        }
        libraryLoaded = true;

        factory = reinterpret_cast<PluginFactory>(sym);
    }

    //
    // Invoke the factory function. No exceptions can be raised
    // by the factory function because it's declared extern "C".
    //
    PluginPtr plugin(factory(_communicator, name, args));
    if (!plugin)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "failure in entry point '" + entryPoint + "'");
    }

    PluginInfo info;
    info.name = name;
    info.plugin = plugin;
    _plugins.push_back(std::move(info));
    return libraryLoaded;
}

Ice::PluginPtr
Ice::PluginManagerI::findPlugin(string_view name) const
{
    for (const auto& p : _plugins)
    {
        if (name == p.name)
        {
            return p.plugin;
        }
    }
    return nullptr;
}
