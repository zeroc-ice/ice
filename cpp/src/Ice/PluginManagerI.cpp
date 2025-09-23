// Copyright (c) ZeroC, Inc.

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

Ice::Plugin::~Plugin() = default;               // avoid weak vtable
Ice::PluginManager::~PluginManager() = default; // avoid weak vtable

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
        for (const auto& plugin : _plugins)
        {
            try
            {
                plugin.plugin->initialize();
            }
            catch (const Ice::PluginInitializationException&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                ostringstream os;
                os << "plugin '" << plugin.name << "' initialization failed:\n" << ex.what();
                throw PluginInitializationException(__FILE__, __LINE__, os.str());
            }
            catch (...)
            {
                ostringstream os;
                os << "plugin '" << plugin.name << "' initialization failed:\nunknown exception";
                throw PluginInitializationException(__FILE__, __LINE__, os.str());
            }
            initializedPlugins.push_back(plugin.plugin);
        }
    }
    catch (...)
    {
        //
        // Destroy the plug-ins that have been successfully initialized, in the
        // reverse order.
        //
        for (auto p = initializedPlugins.rbegin(); p != initializedPlugins.rend(); ++p)
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
    for (const auto& plugin : _plugins)
    {
        names.push_back(plugin.name);
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
            for (auto p = _plugins.rbegin(); p != _plugins.rend(); ++p)
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

Ice::PluginManagerI::PluginManagerI(CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

bool
Ice::PluginManagerI::loadPlugins()
{
    assert(_communicator);
    bool libraryLoaded = false;

    const string prefix = "Ice.Plugin.";
    PropertiesPtr properties = _communicator->getProperties();
    PropertyDict plugins = properties->getPropertiesForPrefix(prefix);

    // First, create plug-ins using the plug-in factories from initData, in order.
    for (const auto& pluginFactory : getInstance(_communicator)->initializationData().pluginFactories)
    {
        string name = pluginFactory.pluginName;

        string key = prefix + name;
        auto r = plugins.find(key);
        if (r != plugins.end())
        {
            loadPlugin(pluginFactory.factoryFunc, name, r->second);
            plugins.erase(r);
        }
        else
        {
            loadPlugin(pluginFactory.factoryFunc, name, "");
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
    for (const auto& name : loadOrder)
    {
        if (findPlugin(name))
        {
            throw PluginInitializationException(__FILE__, __LINE__, "plug-in '" + name + "' already loaded");
        }

        string property = prefix + name;
        auto r = plugins.find(property);
        if (r != plugins.end())
        {
            libraryLoaded |= loadPlugin(nullptr, name, r->second);
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
        libraryLoaded |= loadPlugin(nullptr, key.substr(prefix.size()), value);
    }

    return libraryLoaded;
}

bool
Ice::PluginManagerI::loadPlugin(PluginFactoryFunc factoryFunc, const string& name, const string& pluginSpec)
{
    assert(_communicator);

    if (findPlugin(name))
    {
        // We check and throw this exception before creating the plug-in. For example, the plug-in constructor can
        // register endpoint types, and we don't want such duplication to ever occur.
        throw AlreadyRegisteredException{__FILE__, __LINE__, _kindOfObject, name};
    }

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

        // Convert command-line options into properties.
        PropertiesPtr properties = _communicator->getProperties();
        args = properties->parseCommandLineOptions(name, args);
    }

    if (!factoryFunc)
    {
        assert(!entryPoint.empty());
        DynamicLibrary library;
        DynamicLibrary::symbol_type sym = library.loadEntryPoint(entryPoint);
        if (sym == nullptr)
        {
            ostringstream os;
            os << "unable to load entry point '" << entryPoint << "'";
            const string& msg = library.getErrorMessage();
            if (!msg.empty())
            {
                os << ": " + msg;
            }
            throw PluginInitializationException(__FILE__, __LINE__, os.str());
        }
        libraryLoaded = true;

        factoryFunc = reinterpret_cast<PluginFactoryFunc>(sym);
    }

    //
    // Invoke the factory function. No exceptions can be raised
    // by the factory function because it's declared extern "C".
    //
    PluginPtr plugin{factoryFunc(_communicator, name, args)};
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
