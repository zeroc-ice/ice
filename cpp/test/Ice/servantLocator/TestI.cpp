// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

void
TestI::requestFailedException(const Current&)
{
}

void
TestI::unknownUserException(const Current&)
{
}

void
TestI::unknownLocalException(const Current&)
{
}

void
TestI::unknownException(const Current&)
{
}

void
TestI::userException(const Current&)
{
}

void
TestI::localException(const Current&)
{
}

void
TestI::stdException(const Current&)
{
}

void
TestI::cppException(const Current&)
{
}

void
TestI::unknownExceptionWithServantException(const Current&)
{
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

void
TestI::shutdown(const Current& current)
{
    current.adapter->deactivate();
}

string
CookieI::message() const
{
    return "blahblah";
}

