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
#include <IceUtil/FileUtil.h>

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
        Ice::InitializationData id;
        id.properties = Ice::createProperties();
        id.properties->load(configPath);
        test(id.properties->getProperty("Ice.Trace.Network") == "1");
        test(id.properties->getProperty("Ice.Trace.Protocol") == "1");       
        test(id.properties->getProperty("Config.Path") == configPath);
        test(id.properties->getProperty("Ice.ProgramName") == "PropertiesClient");
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
    return EXIT_SUCCESS;
}