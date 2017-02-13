// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
