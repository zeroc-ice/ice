// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading.Tasks;
using Test;

public sealed class TestI : TestIntfDisp_
{
    public override Task requestFailedExceptionAsync(Ice.Current current)
    {
        return null;
    }

    public override Task unknownUserExceptionAsync(Ice.Current current)
    {
        return null;
    }

    public override Task unknownLocalExceptionAsync(Ice.Current current)
    {
        return null;
    }

    public override Task unknownExceptionAsync(Ice.Current current)
    {
        return null;
    }

    public override Task localExceptionAsync(Ice.Current current)
    {
        return null;
    }

    public override Task userExceptionAsync(Ice.Current current)
    {
        return null;
    }

    public override Task csExceptionAsync(Ice.Current current)
    {
        return null;
    }

    public override Task
    unknownExceptionWithServantExceptionAsync(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    public override Task<string>
    impossibleExceptionAsync(bool @throw, Ice.Current current)
    {
        if(@throw)
        {
            throw new TestImpossibleException();
        }
        else
        {
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly iuf finished throws.
            //
            return Task.FromResult<string>("Hello");
        }
    }

    public override Task<string>
    intfUserExceptionAsync(bool @throw, Ice.Current current)
    {
        if(@throw)
        {
            throw new TestIntfUserException();
        }
        else
        {
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly iuf finished throws.
            //
            return Task.FromResult<string>("Hello");
        }
    }

    public override Task asyncResponseAsync(Ice.Current current)
    {
       return null;
       throw new Ice.ObjectNotExistException();
    }

    public override Task asyncExceptionAsync(Ice.Current current)
    {
        throw new TestIntfUserException();
        throw new Ice.ObjectNotExistException();
    }

    public override Task shutdownAsync(Ice.Current current)
    {
        current.adapter.deactivate();
        return null;
    }
}
