// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"
#include "Ice/Identity.ice"

["python:identifier:generated.test.Ice.current.Test"]
module Test
{
    interface TestIntf
    {
        string getAdapterName();
        string getConnection();
        Ice::Identity getIdentity();
        string getFacet();
        string getOperation();
        string getMode();
        Ice::Context getContext();
        int getRequestId();
        string getEncoding();

        void shutdown();
    }
}
