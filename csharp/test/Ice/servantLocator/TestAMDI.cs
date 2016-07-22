// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public sealed class TestI : TestIntfDisp_
{
    public override void requestFailedExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void unknownUserExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void unknownLocalExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void unknownExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void localExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void userExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void csExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    public override void 
    unknownExceptionWithServantExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new Ice.ObjectNotExistException());
    }

    public override void
    impossibleExceptionAsync(bool @throw, Action<string> response, Action<Exception> exception, Ice.Current current)
    {
        if(@throw)
        {
            exception(new TestImpossibleException());
        }
        else
        {
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly iuf finished throws.
            //
            response("Hello");
        }
    }

    public override void
    intfUserExceptionAsync(bool @throw, Action<string> response, Action<Exception> exception, Ice.Current current)
    {
        if(@throw)
        {
            exception(new TestIntfUserException());
        }
        else
        {
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly iuf finished throws.
            //
            response("Hello");
        }
    }

    public override void asyncResponseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
       response();
       throw new Ice.ObjectNotExistException();
    }

    public override void asyncExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new TestIntfUserException());
        throw new Ice.ObjectNotExistException();
    }

    public override void shutdownAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        current.adapter.deactivate();
        response();
    }
}
