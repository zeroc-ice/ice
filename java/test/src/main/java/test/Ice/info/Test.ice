// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

[["java:package:test.Ice.info"]]
module Test
{

interface TestIntf
{
    void shutdown();

    Ice::Context getEndpointInfoAsContext();

    Ice::Context getConnectionInfoAsContext();
}

}
