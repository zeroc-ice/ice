// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/BuiltinSequences.ice"

module Test
{
    interface TestIntf
    {
        string getProperty(string name);
        Ice::StringSeq getArgs();
    }
}
