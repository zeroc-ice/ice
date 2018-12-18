// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

["objc:prefix:TestHold"]
module Test
{

interface Hold
{
    void putOnHold(int seconds);
    void waitForHold();
    int set(int value, int delay);
    void setOneway(int value, int expected);
    void shutdown();
}

}
