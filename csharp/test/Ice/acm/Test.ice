//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::ACM
{

interface TestIntf
{
    void sleep(int seconds);
    void startHeartbeatCount();
    void waitForHeartbeatCount(int count);
}

interface RemoteObjectAdapter
{
    TestIntf* getTestIntf();
    void deactivate();
}

interface RemoteCommunicator
{
    RemoteObjectAdapter createObjectAdapter(int idleTimeout, bool keepAlive);
    void shutdown();
}

}
