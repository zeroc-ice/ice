//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "Ice/Ice.h"

using namespace Test;

TestI::TestI(const Ice::StringSeq& args) : _args(args) {}

std::string
TestI::getProperty(std::string name, const Ice::Current& current)
{
    return current.adapter->getCommunicator()->getProperties()->getProperty(name);
}

Ice::StringSeq
TestI::getArgs(const Ice::Current&)
{
    return _args;
}
