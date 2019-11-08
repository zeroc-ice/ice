//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/PropertiesAdmin.ice>

module Test
{

interface TestIntf
{
    void sleep(int ms);
}

interface RemoteCommunicator
{
    TestIntf* getObject();

    int getThreadHookStartCount();
    int getThreadHookStopCount();

    int getThreadStartCount();
    int getThreadStopCount();

    void destroy();
}

interface RemoteCommunicatorFactory
{
    RemoteCommunicator* createCommunicator(Ice::PropertyDict props);

    void shutdown();
}

}

#endif
