// Copyright (c) ZeroC, Inc.

#ifndef TEST_ICE
#define TEST_ICE

#include "Ice/PropertyDict.ice"

["java:identifier:test.IceBox.admin.Test"]
module Test
{
    interface TestFacet
    {
        Ice::PropertyDict getChanges();
    }
}

#endif
