// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

interface Hold
{
    void putOnHold(int seconds);
    void waitForHold();
    ["ami"] int set(int value, int delay);
    void setOneway(int value, int expected);
    void shutdown();
};

};

#endif
