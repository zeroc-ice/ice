//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/Properties.ice>

module Test
{

interface RemoteCommunicator
{
    Object* getAdmin();

    Ice::PropertyDict getChanges();

    void shutdown();

    void waitForShutdown();

    void destroy();
}

interface RemoteCommunicatorFactory
{
    RemoteCommunicator* createCommunicator(Ice::PropertyDict props);

    void shutdown();
}

interface TestFacet
{
    void op();
}

}

#endif
