// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <fstream>

using namespace std;

namespace
{

string configPath;

class Client : public Ice::Application
{
public:
    virtual int
    run(int, char*[])
    {
        Ice::PropertiesPtr properties = communicator()->getProperties();
        test(properties->getProperty("Ice.Trace.Network") == "1");
        test(properties->getProperty("Ice.Trace.Protocol") == "1");
        test(properties->getProperty("Config.Path") == configPath);
        test(properties->getProperty("Ice.ProgramName") == "PropertiesClient");
        test(appName() == properties->getProperty("Ice.ProgramName"));
        return EXIT_SUCCESS;
    };
};

}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
#endif

    ifstream in("./config/configPath");
    if(!in)
    {
        test(false);
    }

    if(!getline(in, configPath))
    {
        test(false);
    }
    try
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
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }

    cout << "testing load properties from UTF-8 path using Ice::Application... " << flush;
    Client c;
    c.main(argc, argv, configPath.c_str());
    cout << "ok" << endl;

    try
    {
        //
        // Try to load multiple config files.
        //
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
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }

    try
    {
        cout << "testing configuration file escapes... " << flush;
        Ice::PropertiesPtr properties;
        Ice::StringSeq args;
        args.push_back("--Ice.Config=config/escapes.cfg");
        properties = Ice::createProperties(args);

        string props[] = { "Foo\tBar", "3",
                           "Foo\\tBar", "4",
                           "Escape\\ Space", "2",
                           "Prop1", "1",
                           "Prop2", "2",
                           "Prop3", "3",
                           "My Prop1", "1",
                           "My Prop2", "2",
                           "My.Prop1", "a property",
                           "My.Prop2", "a     property",
                           "My.Prop3", "  a     property  ",
                           "My.Prop4", "  a     property  ",
                           "My.Prop5", "a \\ property",
                           "foo=bar", "1",
                           "foo#bar", "2",
                           "foo bar", "3",
                           "A", "1",
                           "B", "2 3 4",
                           "C", "5=#6",
                           "AServer", "\\\\server\\dir",
                           "BServer", "\\server\\dir",
                           ""
        } ;

        for(size_t i = 0; props[i] != ""; i += 2)
        {
            test(properties->getProperty(props[i]) == props[i + 1]);
        }

        cout << "ok" << endl;
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
