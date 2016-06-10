// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestAMDI.h>

using namespace std;
using namespace Ice;

#ifdef ICE_CPP11_MAPPING
void
TestAMDI::requestFailedExceptionAsync(function<void ()> response, function<void (exception_ptr)>, const Current&)
{
    response();
}

void
TestAMDI::unknownUserExceptionAsync(function<void ()> response, function<void (exception_ptr)>, const Current&)
{
    response();
}

void
TestAMDI::unknownLocalExceptionAsync(function<void ()> response, function<void (exception_ptr)>, const Current&)
{
    response();
}

void
TestAMDI::unknownExceptionAsync(function<void ()> response, function<void (exception_ptr)>, const Current&)
{
    response();
}

void
TestAMDI::userExceptionAsync(function<void ()> response, function<void (exception_ptr)>, const Current&)
{
    response();
}

void
TestAMDI::localExceptionAsync(function<void ()> response, function<void (exception_ptr)>, const Current&)
{
    response();
}

void
TestAMDI::stdExceptionAsync(function<void ()> response, function<void (exception_ptr)>, const Current&)
{
    response();
}

void
TestAMDI::cppExceptionAsync(function<void ()> response, function<void (exception_ptr)>, const Current&)
{
    response();
}

void
TestAMDI::unknownExceptionWithServantExceptionAsync(function<void ()>,
                                                    function<void (exception_ptr)> error,
                                                    const Current&)
{
    try
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    catch(...)
    {
        error(current_exception());
    }
}

void
TestAMDI::impossibleExceptionAsync(bool _cpp_throw,
                                   function<void (const string&)> response,
                                   function<void (exception_ptr)> error,
                                   const Current&)
{
    if(_cpp_throw)
    {
        try
        {
            throw Test::TestImpossibleException();
        }
        catch(...)
        {
            error(current_exception());
        }
    }
    else
    {
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        response("Hello");
    }
}

void
TestAMDI::intfUserExceptionAsync(bool _cpp_throw,
                                 function<void (const string&)> response,
                                 function<void (exception_ptr)> error,
                                 const Current&)
{
    if(_cpp_throw)
    {
        try
        {
            throw Test::TestIntfUserException();
        }
        catch(...)
        {
            error(current_exception());
        }
    }
    else
    {
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        response("Hello");
    }
}

void
TestAMDI::asyncResponseAsync(function<void ()> response,
                             function<void (exception_ptr)>,
                             const Current&)
{
    response();
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

void
TestAMDI::asyncExceptionAsync(function<void ()> response,
                              function<void (exception_ptr)> error,
                              const Current&)
{
    try
    {
        throw Test::TestIntfUserException();
    }
    catch(...)
    {
        error(current_exception());
    }
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

void
TestAMDI::shutdownAsync(function<void ()> response,
                        function<void (exception_ptr)> error,
                        const Current& current)
{
    current.adapter->deactivate();
    response();
}
#else
void
TestAMDI::requestFailedException_async(const Test::AMD_TestIntf_requestFailedExceptionPtr& cb, const Current&)
{
    cb->ice_response();
}

void
TestAMDI::unknownUserException_async(const Test::AMD_TestIntf_unknownUserExceptionPtr& cb, const Current&)
{
    cb->ice_response();
}

void
TestAMDI::unknownLocalException_async(const Test::AMD_TestIntf_unknownLocalExceptionPtr& cb, const Current&)
{
    cb->ice_response();
}

void
TestAMDI::unknownException_async(const Test::AMD_TestIntf_unknownExceptionPtr& cb, const Current&)
{
    cb->ice_response();
}

void
TestAMDI::userException_async(const Test::AMD_TestIntf_userExceptionPtr& cb, const Current&)
{
    cb->ice_response();
}

void
TestAMDI::localException_async(const Test::AMD_TestIntf_localExceptionPtr& cb, const Current&)
{
    cb->ice_response();
}

void
TestAMDI::stdException_async(const Test::AMD_TestIntf_stdExceptionPtr& cb, const Current&)
{
    cb->ice_response();
}

void
TestAMDI::cppException_async(const Test::AMD_TestIntf_cppExceptionPtr& cb, const Current&)
{
    cb->ice_response();
}

void
TestAMDI::unknownExceptionWithServantException_async(const Test::AMD_TestIntf_unknownExceptionWithServantExceptionPtr& cb, const Current&)
{
    cb->ice_exception(Ice::ObjectNotExistException(__FILE__, __LINE__));
}

void
TestAMDI::impossibleException_async(const Test::AMD_TestIntf_impossibleExceptionPtr& cb, bool _cpp_throw,
                                    const Current&)
{
    if(_cpp_throw)
    {
        cb->ice_exception(Test::TestImpossibleException());
    }
    else
    {
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        cb->ice_response("Hello");
    }
}

void
TestAMDI::intfUserException_async(const Test::AMD_TestIntf_intfUserExceptionPtr& cb, bool _cpp_throw, const Current&)
{
    if(_cpp_throw)
    {
        cb->ice_exception(Test::TestIntfUserException());
    }
    else
    {
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        cb->ice_response("Hello");
    }
}

void
TestAMDI::asyncResponse_async(const Test::AMD_TestIntf_asyncResponsePtr& cb, const Current&)
{
    cb->ice_response();
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

void
TestAMDI::asyncException_async(const Test::AMD_TestIntf_asyncExceptionPtr& cb, const Current&)
{
    cb->ice_exception(Test::TestIntfUserException());
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

void
TestAMDI::shutdown_async(const Test::AMD_TestIntf_shutdownPtr& cb, const Current& current)
{
    current.adapter->deactivate();
    cb->ice_response();
}
#endif

string
CookieI::message() const
{
    return "blahblah";
}
