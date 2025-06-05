// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

["java:identifier:test.Ice.info.Test"]
module Test
{
    interface TestIntf
    {
        void shutdown();

        Ice::Context getEndpointInfoAsContext();

        Ice::Context getConnectionInfoAsContext();
    }
}
