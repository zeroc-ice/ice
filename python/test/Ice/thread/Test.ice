// Copyright (c) ZeroC, Inc.

#ifndef TEST_ICE
#define TEST_ICE

#include "Ice/PropertyDict.ice"

module Test
{

interface TestIntf
{
    void sleep(int ms);
}

interface RemoteCommunicator
{
    TestIntf* getObject();

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
