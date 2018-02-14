// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

string
TestI::impossibleException(bool _cpp_throw, const Current&)
{
    if(_cpp_throw)
    {
        throw Test::TestImpossibleException();
    }
    //
    // Return a value so we can be sure that the stream position
    // is reset correctly if finished() throws.
    //
    return "Hello";
}

string
TestI::intfUserException(bool _cpp_throw, const Current&)
{
    if(_cpp_throw)
    {
        throw Test::TestIntfUserException();
    }
    //
    // Return a value so we can be sure that the stream position
    // is reset correctly if finished() throws.
    //
    return "Hello";
}

void
TestI::asyncResponse(const Current&)
{
    //
    // Only relevant for AMD.
    //
}

void
TestI::asyncException(const Current&)
{
    //
    // Only relevant for AMD.
    //
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

