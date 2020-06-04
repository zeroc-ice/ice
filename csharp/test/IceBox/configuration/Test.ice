//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

[[suppress-warning:reserved-identifier]]

module ZeroC::IceBox::Test::Configuration
{

interface TestIntf
{
    string getProperty(string name);
    Ice::StringSeq getArgs();

}

}
