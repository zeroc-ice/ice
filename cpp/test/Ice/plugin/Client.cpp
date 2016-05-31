// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>

using namespace std;

DEFINE_TEST("client")

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

    const Ice::CommunicatorPtr _communicator;
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

int
main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;
    Ice::CommunicatorPtr communicator;

    if(argc < 2)
    {
        cerr << "usage: " << argv[0] << " <plugindir>";
    }

    //
    // Plugin directory is provided as the last argument
    //
#if defined(ICE_OS_WINRT)
    string pluginDir = "plugins/winrt/";
#else
    string pluginDir = argv[argc - 1];
    pluginDir += "/";
#endif

    Ice::registerPluginFactory("Static1", createMyPlugin, true); // true = Load on communicator initialization
    Ice::registerPluginFactory("Static2", createMyPlugin, false);

    cout << "testing static plugin factory... " << flush;
    try
    {
        communicator = Ice::initialize(argc, argv);
        MyPluginPtr plugin = ICE_DYNAMIC_CAST(MyPlugin, communicator->getPluginManager()->getPlugin("Static1"));
        test(plugin && plugin->isInitialized());
        try
        {
            communicator->getPluginManager()->getPlugin("Static2");
        }
        catch(const Ice::NotRegisteredException&)
        {
        }
        communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.Static2", "1");
        communicator = Ice::initialize(argc, argv, initData);
        MyPluginPtr plugin = ICE_DYNAMIC_CAST(MyPlugin, communicator->getPluginManager()->getPlugin("Static1"));
        test(plugin && plugin->isInitialized());
        plugin = ICE_DYNAMIC_CAST(MyPlugin, communicator->getPluginManager()->getPlugin("Static2"));
        test(plugin && plugin->isInitialized());
        communicator->destroy();
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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.Test", pluginDir + "TestPlugin:createPlugin");
        communicator = Ice::initialize(argc, argv, initData);
        communicator->destroy();
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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.Test", os.str());
        communicator = Ice::initialize(argc, argv, initData);
        communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.Test", pluginDir + "TestPlugin,10:createPlugin");
        communicator = Ice::initialize(argc, argv, initData);
        test(false);
    }
    catch(const Ice::PluginInitializationException&)
    {
    }
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.Test", "TestPlugin,1.0.0:createPlugin");
        communicator = Ice::initialize(argc, argv, initData);
        test(false);
    }
    catch(const Ice::PluginInitializationException&)
    {
    }
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.Test",
            pluginDir + "TestPlugin:createPluginWithArgs 'C:\\Program Files\\' --DatabasePath "
            "'C:\\Program Files\\Application\\db'" );
        communicator = Ice::initialize(argc, argv, initData);
        communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing a simple plug-in that fails to initialize... " << flush;
    communicator = 0;
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.Test", pluginDir + "TestPlugin:createPluginInitializeFail");
        communicator = Ice::initialize(argc, argv, initData);
        test(false);
    }
    catch(const Ice::PluginInitializationException& ex)
    {
        test(ex.reason.find("PluginInitializeFailExeption") > 0);
    }
    test(!communicator);
    cout << "ok" << endl;

    cout << "testing plug-in load order... " << flush;
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.PluginOne", pluginDir + "TestPlugin:createPluginOne");
        initData.properties->setProperty("Ice.Plugin.PluginTwo", pluginDir + "TestPlugin:createPluginTwo");
        initData.properties->setProperty("Ice.Plugin.PluginThree", pluginDir + "TestPlugin:createPluginThree");
        initData.properties->setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo"); // Exclude PluginThree
        communicator = Ice::initialize(argc, argv, initData);
        communicator->destroy();
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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.PluginOne", pluginDir + "TestPlugin:createPluginOne");
        initData.properties->setProperty("Ice.Plugin.PluginTwo", pluginDir + "TestPlugin:createPluginTwo");
        initData.properties->setProperty("Ice.Plugin.PluginThree", pluginDir + "TestPlugin:createPluginThree");
        initData.properties->setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo");
        initData.properties->setProperty("Ice.InitPlugins", "0");
        communicator = Ice::initialize(argc, argv, initData);

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
    communicator = 0;
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.PluginOneFail", pluginDir + "TestPlugin:createPluginOneFail");
        initData.properties->setProperty("Ice.Plugin.PluginTwoFail", pluginDir + "TestPlugin:createPluginTwoFail");
        initData.properties->setProperty("Ice.Plugin.PluginThreeFail", pluginDir + "TestPlugin:createPluginThreeFail");
        initData.properties->setProperty("Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
        communicator = Ice::initialize(argc, argv, initData);
        test(false);
    }
    catch(const Ice::PluginInitializationException& ex)
    {
        test(ex.reason.find("PluginInitializeFailExeption") > 0);
    }
    test(!communicator);
    cout << "ok" << endl;

    return status;
}
