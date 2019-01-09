// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;

TestI::TestI(const Ice::StringSeq& args) : _args(args)
{
}

std::string
TestI::getProperty(ICE_IN(std::string) name, const Ice::Current& current)
{
    return current.adapter->getCommunicator()->getProperties()->getProperty(name);
}

Ice::StringSeq
TestI::getArgs(const Ice::Current&)
{
    return _args;
}
