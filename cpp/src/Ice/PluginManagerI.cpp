// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/PluginManagerI.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/Properties.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

typedef Ice::Plugin* (*PLUGIN_FACTORY)(const CommunicatorPtr&, const string&, const StringSeq&);

Ice::PluginManagerI::PluginManagerI(const InstancePtr& instance)
    : _instance(instance)
{
}

PluginPtr
Ice::PluginManagerI::getPlugin(const string& name)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, PluginInfo>::const_iterator r = _plugins.find(name);
    if (r != _plugins.end())
    {
        return (*r).second.plugin;
    }
    throw PluginNotFoundException(__FILE__, __LINE__);
}

void
Ice::PluginManagerI::addPlugin(const string& name, const PluginPtr& plugin)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, PluginInfo>::const_iterator r = _plugins.find(name);
    if (r != _plugins.end())
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

    map<string, PluginInfo>::iterator r;
    for (r = _plugins.begin(); r != _plugins.end(); ++r)
    {
        (*r).second.plugin->destroy();
        (*r).second.plugin = 0;
        (*r).second.library = 0;
    }
}

void
Ice::PluginManagerI::loadPlugins(int& argc, char* argv[])
{
    StringSeq cmdArgs = argsToStringSeq(argc, argv);

    //
    // Load and initialize the plug-ins defined in the property set
    // with the prefix "Ice.Plugin.". These properties should
    // have the following format:
    //
    // Ice.Plugin.name=entry_point [args]
    //
    const string prefix = "Ice.Plugin.";
    PropertiesPtr properties = _instance->properties();
    StringSeq plugins = properties->getProperties(prefix);
    for (StringSeq::size_type i = 0; i < plugins.size(); i += 2)
    {
        string name = plugins[i].substr(prefix.size());
        string value = plugins[i + 1];

        //
        // Separate the entry point from the arguments.
        //
        string entryPoint;
        StringSeq args;
        string::size_type pos = value.find_first_of(" \t\n");
        if (pos == string::npos)
        {
            entryPoint = value;
        }
        else
        {
            entryPoint = value.substr(0, pos);
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
    //
    // Parse the entry point. An entry point has the following format:
    //
    // name[,version]:function
    //
    // The name of the shared library/DLL is constructed from the
    // given information. If no version is supplied, the Ice version
    // is used. For example, consider the following entry point:
    //
    // foo:create
    //
    // This would result in libfoo.so.0.0.1 (Unix) and foo001.dll (Windows),
    // where the Ice version is 0.0.1.
    //
    // Now consider this entry point:
    //
    // foo,1.1:create
    //
    // The library names in this case are libfoo.so.1.1 (Unix) and
    // foo11.dll (Windows).
    //
    // On Windows platforms, a 'd' is appended to the version for debug
    // builds.
    //
    string::size_type colon = entryPoint.rfind(':');
    string::size_type comma = entryPoint.find(',');
    if (colon == string::npos || colon == entryPoint.size() - 1 ||
        (comma != string::npos && (comma > colon || comma == colon - 1)))
    {
        Error out(_instance->logger());
        out << "PluginManager: invalid entry point format `" << entryPoint << "'";
        SystemException ex(__FILE__, __LINE__);
        ex.error = 0;
        throw ex;
    }
    string libSpec = entryPoint.substr(0, colon);
    string funcName = entryPoint.substr(colon + 1);
    string libName, version, debug;
    if (comma == string::npos)
    {
        libName = libSpec;
        version = ICE_STRING_VERSION;
    }
    else
    {
        libName = libSpec.substr(0, comma);
        version = libSpec.substr(comma + 1);
    }

    string lib;

#ifdef _WIN32
    lib = libName;
    for (string::size_type n = 0; n < version.size(); n++)
    {
        if (version[n] != '.') // Remove periods
        {
            lib += version[n];
        }
    }
#   ifdef _DEBUG
    lib += 'd';
#   endif
    lib += ".dll";
#else
    lib = "lib" + libName + ".so." + version;
#endif

    //
    // Load the dynamic library.
    //
    PluginInfo info;
    info.library = new DynamicLibrary();
    if (!info.library->load(lib))
    {
        Error out(_instance->logger());
        string msg = info.library->getErrorMessage();
        out << "PluginManager: unable to load library `" << lib << "'";
        if (!msg.empty())
        {
            out << ": " + msg;
        }
        SystemException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }

    //
    // Lookup the factory function.
    //
    DynamicLibrary::symbol_type sym = info.library->getSymbol(funcName);
    if (sym == 0)
    {
        Error out(_instance->logger());
        string msg = info.library->getErrorMessage();
        out << "PluginManager: unable to find function `" << funcName << "' in library `" << lib << "'";
        if (!msg.empty())
        {
            out << ": " + msg;
        }
        SystemException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }

    //
    // Invoke the factory function.
    //
    PLUGIN_FACTORY factory = (PLUGIN_FACTORY)sym;
    try
    {
        info.plugin = factory(_instance->communicator(), name, args);
    }
    catch (const Exception& ex)
    {
        Error out(_instance->logger());
        out << "PluginManager: exception in factory function `" << funcName << "': " << ex.ice_name();
        SystemException e(__FILE__, __LINE__);
        e.error = 0;
        throw e;
    }

    _plugins[name] = info;
}
