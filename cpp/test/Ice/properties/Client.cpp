//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "TestHelper.h"
#include <fstream>

using namespace std;

string configPath;

class Client : public Test::TestHelper
{
public:
    void run(int, char**);
};

void
Client::run(int, char**)
{
    ifstream in("./config/configPath");
    if (!in)
    {
        test(false);
    }

    if (!getline(in, configPath))
    {
        test(false);
    }

    {
        cout << "testing load properties from UTF-8 path... " << flush;
        Ice::PropertiesPtr properties = Ice::createProperties();
        properties->load(configPath);
        test(properties->getProperty("Ice.Trace.Network") == "1");
        test(properties->getProperty("Ice.Trace.Protocol") == "1");
        test(properties->getProperty("Config.Path") == configPath);
        test(properties->getProperty("Ice.ProgramName") == "PropertiesClient");
        cout << "ok" << endl;
    }

    {
        cout << "testing using Ice.Config with multiple config files... " << flush;
        Ice::PropertiesPtr properties;
        Ice::StringSeq args;
        args.push_back("--Ice.Config=config/config.1, config/config.2, config/config.3");
        properties = Ice::createProperties(args);
        test(properties->getProperty("Config1") == "Config1");
        test(properties->getProperty("Config2") == "Config2");
        test(properties->getProperty("Config3") == "Config3");
        cout << "ok" << endl;
    }

    {
        cout << "testing configuration file escapes... " << flush;
        Ice::PropertiesPtr properties;
        Ice::StringSeq args;
        args.push_back("--Ice.Config=config/escapes.cfg");
        properties = Ice::createProperties(args);

        string props[] = {
            "Foo\tBar",
            "3",
            "Foo\\tBar",
            "4",
            "Escape\\ Space",
            "2",
            "Prop1",
            "1",
            "Prop2",
            "2",
            "Prop3",
            "3",
            "My Prop1",
            "1",
            "My Prop2",
            "2",
            "My.Prop1",
            "a property",
            "My.Prop2",
            "a     property",
            "My.Prop3",
            "  a     property  ",
            "My.Prop4",
            "  a     property  ",
            "My.Prop5",
            "a \\ property",
            "foo=bar",
            "1",
            "foo#bar",
            "2",
            "foo bar",
            "3",
            "A",
            "1",
            "B",
            "2 3 4",
            "C",
            "5=#6",
            "AServer",
            "\\\\server\\dir",
            "BServer",
            "\\server\\dir",
            ""};

        for (size_t i = 0; props[i] != ""; i += 2)
        {
            test(properties->getProperty(props[i]) == props[i + 1]);
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing ice properties with set default values... " << flush;
        Ice::PropertiesPtr properties = Ice::createProperties();
        string toStringMode = properties->getIceProperty("Ice.ToStringMode");
        test(toStringMode == "Unicode");
        int32_t closeTimeout = properties->getIcePropertyAsInt("Ice.Connection.Client.CloseTimeout");
        test(closeTimeout == 10);
        vector<string> retryIntervals = properties->getIcePropertyAsList("Ice.RetryIntervals");
        test(retryIntervals.size() == 1);
        test(retryIntervals[0] == "0");
        cout << "ok" << endl;
    }

    {
        cout << "testing ice properties with unset default values... " << flush;
        Ice::PropertiesPtr properties = Ice::createProperties();
        string stringValue = properties->getIceProperty("Ice.Admin.Router");
        test(stringValue == "");
        int32_t intValue = properties->getIcePropertyAsInt("Ice.Admin.Router");
        test(intValue == 0);
        vector<string> listValue = properties->getIcePropertyAsList("Ice.Admin.Router");
        test(listValue.size() == 0);
        cout << "ok" << endl;
    }

    {
        cout << "testing that getting an unknown property throws an exception... " << flush;
        Ice::PropertiesPtr properties = Ice::createProperties();
        try
        {
            properties->getIceProperty("Ice.UnknownProperty");
            test(false);
        }
        catch (const Ice::UnknownPropertyException&)
        {
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing that setting an unknown property throws an exception... " << flush;
        Ice::PropertiesPtr properties = Ice::createProperties();
        try
        {
            properties->setProperty("Ice.UnknownProperty", "value");
            test(false);
        }
        catch (const Ice::UnknownPropertyException&)
        {
        }
        cout << "ok" << endl;
    }

    {
        Ice::CommunicatorHolder communicator = Ice::initialize();
        Ice::PropertiesPtr properties = communicator->getProperties();

        cout << "testing that creating an object adapter with unknown properties throws an exception..." << flush;
        properties->setProperty("FooOA.Endpoints", "tcp -h 127.0.0.1");
        properties->setProperty("FooOA.UnknownProperty", "bar");
        try
        {
            Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("FooOA");
            test(false);
        }
        catch (const Ice::UnknownPropertyException&)
        {
        }
        cout << "ok" << endl;

        cout << "testing that creating a proxy with unknown properties throws an exception..." << flush;
        properties->setProperty("FooProxy", "test:tcp -h 127.0.0.1 -p 10000");
        properties->setProperty("FooProxy.UnknownProperty", "bar");
        try
        {
            communicator->propertyToProxy("FooProxy");
            test(false);
        }
        catch (const Ice::UnknownPropertyException&)
        {
        }
    }
}

DEFINE_TEST(Client)
