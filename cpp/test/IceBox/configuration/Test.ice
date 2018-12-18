// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

module Test
{

interface TestIntf
{
    string getProperty(string name);
    Ice::StringSeq getArgs();

}

}
