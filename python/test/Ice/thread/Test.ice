// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
};

interface RemoteCommunicator
{
    TestIntf* getObject();

    int getThreadHookStartCount();
    int getThreadHookStopCount();

    int getThreadStartCount();
    int getThreadStopCount();

    void destroy();
};

interface RemoteCommunicatorFactory
{
    RemoteCommunicator* createCommunicator(Ice::PropertyDict props);

    void shutdown();
};

};

#endif
