//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

[["java:package:test.IceBox.configuration"]]
module Test
{

interface TestIntf
{
    string getProperty(string name);
    Ice::StringSeq getArgs();

}

}
