// Copyright (c) ZeroC, Inc.
#pragma once

module Test
{

interface Hold
{
    void putOnHold(int seconds);
    void waitForHold();
    void setOneway(int value, int expected);
    int set(int value, int delay);
    void shutdown();
}

}
