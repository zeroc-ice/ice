// Copyright (c) ZeroC, Inc.

#include "ServantLocatorI.h"
#include "Test.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace Test;

ServantLocatorI::ServantLocatorI(string category) : _category(std::move(category)) {}

ServantLocatorI::~ServantLocatorI() { test(_deactivated); }

Ice::ObjectPtr
ServantLocatorI::locate(const Ice::Current& current, shared_ptr<void>& cookie)
{
    test(!_deactivated);
    test(current.id.category == _category || _category.empty());

    if (current.id.name == "unknown")
    {
        return nullptr;
    }

    if (current.id.name == "invalidReturnValue" || current.id.name == "invalidReturnType")
    {
        return nullptr;
    }

    test(current.id.name == "locate" || current.id.name == "finished");
    if (current.id.name == "locate")
    {
        exception(current);
    }

    //
    // Ensure locate() is only called once per request.
    //
    test(_requestId == -1);
    _requestId = current.requestId;

    return newServantAndCookie(cookie);
}

void
ServantLocatorI::finished(const Ice::Current& current, const Ice::ObjectPtr&, const shared_ptr<void>& cookie)
{
    test(!_deactivated);

    //
    // Ensure finished() is only called once per request.
    //
    test(_requestId == current.requestId);
    _requestId = -1;

    test(current.id.category == _category || _category.empty());
    test(current.id.name == "locate" || current.id.name == "finished");

    if (current.id.name == "finished")
    {
        exception(current);
    }

    checkCookie(cookie);
}

void
ServantLocatorI::deactivate(string_view)
{
    test(!_deactivated);

    _deactivated = true;
}

void
ServantLocatorI::exception(const Ice::Current& current)
{
    if (current.operation == "ice_ids")
    {
        throw Test::TestIntfUserException();
    }
    else if (current.operation == "requestFailedException")
    {
        throw Ice::ObjectNotExistException{__FILE__, __LINE__};
    }
    else if (current.operation == "unknownUserException")
    {
        throw UnknownUserException(__FILE__, __LINE__, "reason");
    }
    else if (current.operation == "unknownLocalException")
    {
        throw UnknownLocalException(__FILE__, __LINE__, "reason");
    }
    else if (current.operation == "unknownException")
    {
        throw UnknownException(__FILE__, __LINE__, "reason");
    }
    else if (current.operation == "userException")
    {
        throwTestIntfUserException();
    }
    else if (current.operation == "localException")
    {
        throw Ice::SocketException(__FILE__, __LINE__, 0);
    }
    else if (current.operation == "stdException")
    {
        throw std::runtime_error("Hello");
    }
    else if (current.operation == "cppException")
    {
        throw 5;
    }
    else if (current.operation == "unknownExceptionWithServantException")
    {
        throw UnknownException(__FILE__, __LINE__, "reason");
    }
    else if (current.operation == "impossibleException")
    {
        throw TestIntfUserException(); // Yes, it really is meant to be TestIntfUserException.
    }
    else if (current.operation == "intfUserException")
    {
        throw TestImpossibleException(); // Yes, it really is meant to be TestImpossibleException.
    }
    else if (current.operation == "asyncResponse")
    {
        throw TestImpossibleException();
    }
    else if (current.operation == "asyncException")
    {
        throw TestImpossibleException();
    }
}
