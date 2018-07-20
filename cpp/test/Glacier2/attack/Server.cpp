// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <BackendI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class ServantLocatorI : public virtual ServantLocator
{
public:

    ServantLocatorI() :
        _backend(new BackendI)
    {
    }

    virtual ObjectPtr locate(const Current&, LocalObjectPtr&)
    {
        return _backend;
    }

    virtual void finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
    {
    }

    virtual void deactivate(const string&)
    {
    }

private:

    BackendPtr _backend;
};

class BackendServer : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
BackendServer::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("BackendAdapter.Endpoints", getTestEndpoint());
    ObjectAdapterPtr adapter = communicator->createObjectAdapter("BackendAdapter");
    adapter->addServantLocator(new ServantLocatorI, "");
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(BackendServer)
