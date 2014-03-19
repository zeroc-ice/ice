// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

class Server : public Ice::Application
{
public:

    virtual int run(int argc, char* argv[]);

};

int
main(int argc, char* argv[])
{
    //
    // Set the 'BuildId' property displayed in the IceGridAdmin GUI
    //
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties();
    initData.properties->setProperty("BuildId", string("Ice ") + ICE_STRING_VERSION);

    Server app;
    int status = app.main(argc, argv, initData);
    return status;
}

int
Server::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Ice::PropertiesPtr properties = communicator()->getProperties();
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Hello");
    Ice::Identity id = communicator()->stringToIdentity(properties->getProperty("Identity"));
    Demo::HelloPtr hello = new HelloI(properties->getProperty("Ice.ProgramName"));
    adapter->add(hello, id);
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
