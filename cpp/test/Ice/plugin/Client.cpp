// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>

using namespace std;

namespace
{

class MyPlugin : public Ice::Plugin
{

public:

    MyPlugin() :
        _initialized(false),
        _destroyed(false)
    {
    }

    bool
    isInitialized() const
    {
        return _initialized;
    }

    bool
    isDestroyed() const
    {
        return _destroyed;
    }

    void
    initialize()
    {
        _initialized = true;
    }

    void
    destroy()
    {
        _destroyed = true;
    }

    ~MyPlugin()
    {
        test(!_initialized || _destroyed); // If initialized, we must be destroyed too.
    }

private:

    bool _initialized;
    bool _destroyed;
};
ICE_DEFINE_PTR(MyPluginPtr, MyPlugin);

}

extern "C"
{

Ice::Plugin*
createMyPlugin(const ::Ice::CommunicatorPtr&, const std::string&, const ::Ice::StringSeq&)
{
    return new MyPlugin();
}

}

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    if(argc < 2)
    {
        ostringstream os;
        os << "usage: " << argv[0] << " <plugindir>";
        cerr << os.str() << endl;
        throw invalid_argument(os.str());
    }

    //
    // Plugin directory is provided as the last argument
    //
#if defined(ICE_OS_UWP)
    string pluginDir = "plugins/uwp/";
#else
    string pluginDir = argv[argc - 1];
    pluginDir += "/";
#endif

    Ice::registerPluginFactory("Static1", createMyPlugin, true); // true = Load on communicator initialization
    Ice::registerPluginFactory("Static2", createMyPlugin, false);

    cout << "testing static plugin factory... " << flush;
    try
    {
        Ice::CommunicatorHolder communicator = initialize(argc, argv);
        MyPluginPtr plugin = ICE_DYNAMIC_CAST(MyPlugin, communicator->getPluginManager()->getPlugin("Static1"));
        test(plugin && plugin->isInitialized());
        try
        {
            communicator->getPluginManager()->getPlugin("Static2");
        }
        catch(const Ice::NotRegisteredException&)
        {
        }
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.Static2", "1");
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
        MyPluginPtr plugin = ICE_DYNAMIC_CAST(MyPlugin, communicator->getPluginManager()->getPlugin("Static1"));
        test(plugin && plugin->isInitialized());
        plugin = ICE_DYNAMIC_CAST(MyPlugin, communicator->getPluginManager()->getPlugin("Static2"));
        test(plugin && plugin->isInitialized());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing a simple plug-in... " << flush;
    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.Test", pluginDir + "TestPlugin:createPlugin");
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    try
    {
        int majorVersion = (ICE_INT_VERSION / 10000);
        int minorVersion = (ICE_INT_VERSION / 100) - majorVersion * 100;
        int patchVersion = ICE_INT_VERSION % 100;
        ostringstream os;
        os << pluginDir << "TestPlugin,";
        os << majorVersion * 10 + minorVersion;
        if(patchVersion >= 60)
        {
            os << 'b' << (patchVersion - 60);
        }
        else if(patchVersion >= 50)
        {
            os << 'a' << (patchVersion - 50);
        }
        os << ":createPlugin";
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.Test", os.str());
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.Test", pluginDir + "TestPlugin,10:createPlugin");
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
        test(false);
    }
    catch(const Ice::PluginInitializationException&)
    {
    }

    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.Test", "TestPlugin,1.0.0:createPlugin");
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
        test(false);
    }
    catch(const Ice::PluginInitializationException&)
    {
    }

    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.Test",
                                pluginDir + "TestPlugin:createPluginWithArgs 'C:\\Program Files\\' --DatabasePath "
                                "'C:\\Program Files\\Application\\db'" );
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing a simple plug-in that fails to initialize... " << flush;
    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.Test", pluginDir + "TestPlugin:createPluginInitializeFail");
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
        test(false);
    }
    catch(const Ice::PluginInitializationException& ex)
    {
        test(ex.reason.find("PluginInitializeFailExeption") > 0);
    }
    cout << "ok" << endl;

    cout << "testing plug-in load order... " << flush;
    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.PluginOne", pluginDir + "TestPlugin:createPluginOne");
        properties->setProperty("Ice.Plugin.PluginTwo", pluginDir + "TestPlugin:createPluginTwo");
        properties->setProperty("Ice.Plugin.PluginThree", pluginDir + "TestPlugin:createPluginThree");
        properties->setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo"); // Exclude PluginThree
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing plug-in manager... " << flush;
    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.PluginOne", pluginDir + "TestPlugin:createPluginOne");
        properties->setProperty("Ice.Plugin.PluginTwo", pluginDir + "TestPlugin:createPluginTwo");
        properties->setProperty("Ice.Plugin.PluginThree", pluginDir + "TestPlugin:createPluginThree");
        properties->setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo");
        properties->setProperty("Ice.InitPlugins", "0");
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

        Ice::PluginManagerPtr pm = communicator->getPluginManager();
        test(pm->getPlugin("PluginOne"));
        test(pm->getPlugin("PluginTwo"));
        test(pm->getPlugin("PluginThree"));

        MyPluginPtr p4 = ICE_MAKE_SHARED(MyPlugin);
        pm->addPlugin("PluginFour", p4);
        test(pm->getPlugin("PluginFour"));

        pm->initializePlugins();

        test(p4->isInitialized());

        communicator->destroy();

        test(p4->isDestroyed());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing destroy when a plug-in fails to initialize... " << flush;
    try
    {
        Ice::PropertiesPtr properties = createTestProperties(argc, argv);
        properties->setProperty("Ice.Plugin.PluginOneFail", pluginDir + "TestPlugin:createPluginOneFail");
        properties->setProperty("Ice.Plugin.PluginTwoFail", pluginDir + "TestPlugin:createPluginTwoFail");
        properties->setProperty("Ice.Plugin.PluginThreeFail", pluginDir + "TestPlugin:createPluginThreeFail");
        properties->setProperty("Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
        Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
        test(false);
    }
    catch(const Ice::PluginInitializationException& ex)
    {
        test(ex.reason.find("PluginInitializeFailExeption") > 0);
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
