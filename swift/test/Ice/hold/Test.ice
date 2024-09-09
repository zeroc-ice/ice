// Copyright (c) ZeroC, Inc.
#pragma once

module Test
{

interface Hold
{
    void putOnHold(int seconds);
    void waitForHold();
    void shutdown();
}

}
