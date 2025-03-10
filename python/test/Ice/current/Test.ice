// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"
#include "Ice/Identity.ice"

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
