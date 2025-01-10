//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"

#include "Ice/Ice.h"
#include <utility>

using namespace Test;

TestI::TestI(Ice::StringSeq args) : _args(std::move(args)) {}

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
