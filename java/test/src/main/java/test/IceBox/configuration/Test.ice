// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/BuiltinSequences.ice"

["java:identifier:test.IceBox.configuration.Test"]
module Test
{
    interface TestIntf
    {
        string getProperty(string name);
        Ice::StringSeq getArgs();
    }
}
