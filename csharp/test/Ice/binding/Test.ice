//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Binding
{

interface TestIntf
{
    string getAdapterName();
}

interface RemoteObjectAdapter
{
    TestIntf getTestIntf();

    void deactivate();
}

interface RemoteCommunicator
{
    RemoteObjectAdapter createObjectAdapter(string name, string transport);
    RemoteObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints);

    void deactivateObjectAdapter(RemoteObjectAdapter adapter);

    void shutdown();
}

}
