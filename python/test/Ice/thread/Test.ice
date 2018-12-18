// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/Properties.ice>

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
