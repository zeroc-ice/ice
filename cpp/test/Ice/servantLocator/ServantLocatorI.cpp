// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ServantLocatorI.h>
#include <TestCommon.h>

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace Test;

ServantLocatorI::ServantLocatorI(const string& category) :
    _category(category),
    _deactivated(false)
{
}

ServantLocatorI::~ServantLocatorI()
{
    test(_deactivated);
}

Ice::ObjectPtr
ServantLocatorI::locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
{
    test(!_deactivated);
    test(current.id.category == _category || _category.empty());

    if(current.id.name == "unknown")
    {
        return 0;
    }

    test(current.id.name == "locate" || current.id.name == "finished");
    if(current.id.name == "locate")
    {
        exception(current);
    }

    return newServantAndCookie(cookie);
}

void
ServantLocatorI::finished(const Ice::Current& current, const Ice::ObjectPtr& servant,
                          const Ice::LocalObjectPtr& cookie)
{
    test(!_deactivated);
    test(current.id.category == _category  || _category.empty());
    test(current.id.name == "locate" || current.id.name == "finished");

    if(current.id.name == "finished")
    {
        exception(current);
    }

    checkCookie(cookie);
}

void
ServantLocatorI::deactivate(const string&)
{
    test(!_deactivated);

    _deactivated = true;
}

void
ServantLocatorI::exception(const Ice::Current& current)
{
    if(current.operation == "requestFailedException")
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    else if(current.operation == "unknownUserException")
    {
        throw UnknownUserException(__FILE__, __LINE__, "reason");
    }
    else if(current.operation == "unknownLocalException")
    {
        throw UnknownLocalException(__FILE__, __LINE__, "reason");
    }
    else if(current.operation == "unknownException")
    {
        throw UnknownException(__FILE__, __LINE__, "reason");
    }
    else if(current.operation == "userException")
    {
        throwTestIntfUserException();
    }
    else if(current.operation == "localException")
    {
        throw Ice::SocketException(__FILE__, __LINE__, 0);
    }
    else if(current.operation == "stdException")
    {
        throw std::runtime_error("Hello");
    }
    else if(current.operation == "cppException")
    {
        throw 5;
    }
    else if(current.operation == "unknownExceptionWithServantException")
    {
        throw UnknownException(__FILE__, __LINE__, "reason");
    }
}
