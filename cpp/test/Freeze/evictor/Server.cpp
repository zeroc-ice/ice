// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <TestI.h>

using namespace std;

class ServantFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr
    create(const string& type)
    {
        assert(type == "::Test::Servant");
        return new Test::ServantI;
    }

    virtual void
    destroy()
    {
    }
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator, const Freeze::DBEnvironmentPtr& dbEnv)
{
    communicator->getProperties()->setProperty("Evictor.Endpoints", "default -p 12345 -t 2000");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Evictor");

    Test::RemoteEvictorFactoryPtr factory = new Test::RemoteEvictorFactoryI(adapter, dbEnv);
    adapter->add(factory, Ice::stringToIdentity("factory"));

    Ice::ObjectFactoryPtr servantFactory = new ServantFactory;
    communicator->addObjectFactory(servantFactory, "::Test::Servant");

    adapter->activate();

    communicator->waitForShutdown();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
    Freeze::DBEnvironmentPtr dbEnv;
    string dbEnvDir = "db";

    try
    {
        communicator = Ice::initialize(argc, argv);
        if(argc != 1)
        {
            dbEnvDir = argv[1];
            dbEnvDir += "/";
            dbEnvDir += "db";
        }
        dbEnv = Freeze::initialize(communicator, dbEnvDir);
        status = run(argc, argv, communicator, dbEnv);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(dbEnv)
    {
        dbEnv->close();
        dbEnv = 0;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
