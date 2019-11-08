//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/PropertiesAdmin.ice>

module Test
{

interface RemoteCommunicator
{
    Object* getAdmin();

    Ice::PropertyDict getChanges();

    void addUpdateCallback();
    void removeUpdateCallback();

    //
    // Logger operations
    //
    void print(string message);
    void trace(string category, string message);
    void warning(string message);
    void error(string message);

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
