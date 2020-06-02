//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

module ZeroC::IceBox::Test::Configuration
{

interface TestIntf
{
    string getProperty(string name);
    Ice::StringSeq getArgs();

}

}
