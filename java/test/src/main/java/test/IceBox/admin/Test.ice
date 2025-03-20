// Copyright (c) ZeroC, Inc.

#ifndef TEST_ICE
#define TEST_ICE

#include "Ice/PropertyDict.ice"

[["java:package:test.IceBox.admin"]]
module Test
{
    interface TestFacet
    {
        Ice::PropertyDict getChanges();
    }
}

#endif
