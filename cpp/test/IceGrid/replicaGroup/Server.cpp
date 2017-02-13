// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace std;

class Server : public Ice::Application
{
public:

    virtual int run(int argc, char* argv[]);
};

int
Server::run(int, char**)
{
    Ice::ObjectAdapterPtr adpt = communicator()->createObjectAdapter("ReplicatedAdapter");
    Ice::PropertiesPtr properties = communicator()->getProperties();
    Ice::ObjectPtr object = new TestI(properties);
    adpt->add(object, communicator()->stringToIdentity(properties->getProperty("Ice.ProgramName")));
    adpt->add(object, communicator()->stringToIdentity(properties->getProperty("Identity")));
    shutdownOnInterrupt();
    try
    {
        adpt->activate();
        communicator()->getAdmin();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        //
        // getAdmin might raise this if communicator is shutdown by
        // servant.
        //
    }
    communicator()->waitForShutdown();
    ignoreInterrupt();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    Server app;
    return app.main(argc, argv);
}
