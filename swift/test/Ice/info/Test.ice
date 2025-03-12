// Copyright (c) ZeroC, Inc.
#pragma once

#include "Ice/Context.ice"

module Test
{
    interface TestIntf
    {
        void shutdown();

        Ice::Context getEndpointInfoAsContext();

        Ice::Context getConnectionInfoAsContext();
    }
}
