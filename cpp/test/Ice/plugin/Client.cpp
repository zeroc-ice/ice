// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
        test(_initialized);
        test(_destroyed);
    }

private:

    const Ice::CommunicatorPtr _communicator;
    bool _initialized;
    bool _destroyed;
};
typedef IceUtil::Handle<MyPlugin> MyPluginPtr;

}

int
main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;
    Ice::CommunicatorPtr communicator;

    cout << "testing a simple plug-in... " << flush;
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.Test",
            "plugins/TestPlugin:createPlugin 'C:\\Program Files\\' --DatabasePath "
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
        initData.properties->setProperty("Ice.Plugin.Test", "plugins/TestPlugin:createPluginInitializeFail");
        communicator = Ice::initialize(argc, argv, initData);
        test(false);
    }
    catch(const std::exception& ex)
    {
        test(string(ex.what()) == "PluginInitializeFailExeption");
    }
    test(!communicator);
    cout << "ok" << endl;

    cout << "testing plug-in load order... " << flush;
    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("Ice.Plugin.PluginOne", "plugins/TestPlugin:createPluginOne");
        initData.properties->setProperty("Ice.Plugin.PluginTwo", "plugins/TestPlugin:createPluginTwo");
        initData.properties->setProperty("Ice.Plugin.PluginThree", "plugins/TestPlugin:createPluginThree");
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
        initData.properties->setProperty("Ice.Plugin.PluginOne", "plugins/TestPlugin:createPluginOne");
        initData.properties->setProperty("Ice.Plugin.PluginTwo", "plugins/TestPlugin:createPluginTwo");
        initData.properties->setProperty("Ice.Plugin.PluginThree", "plugins/TestPlugin:createPluginThree");
        initData.properties->setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo");
        initData.properties->setProperty("Ice.InitPlugins", "0");
        communicator = Ice::initialize(argc, argv, initData);

        Ice::PluginManagerPtr pm = communicator->getPluginManager();
        test(pm->getPlugin("PluginOne"));
        test(pm->getPlugin("PluginTwo"));
        test(pm->getPlugin("PluginThree"));

        MyPluginPtr p4 = new MyPlugin;
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
        initData.properties->setProperty("Ice.Plugin.PluginOneFail", "plugins/TestPlugin:createPluginOneFail");
        initData.properties->setProperty("Ice.Plugin.PluginTwoFail", "plugins/TestPlugin:createPluginTwoFail");
        initData.properties->setProperty("Ice.Plugin.PluginThreeFail", "plugins/TestPlugin:createPluginThreeFail");
        initData.properties->setProperty("Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
        communicator = Ice::initialize(argc, argv, initData);
    }
    catch(const std::exception& ex)
    {
        test(string(ex.what()) == "PluginInitializeFailExeption");
    }
    test(!communicator);
    cout << "ok" << endl;

    return status;
}
