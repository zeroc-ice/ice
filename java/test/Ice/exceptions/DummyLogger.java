// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;


public final class DummyLogger implements Ice.Logger
{
    @Override
    public void
    print(String message)
    {
    }

    @Override
    public void
    trace(String category, String message)
    {
    }

    @Override
    public void
    warning(String message)
    {
    }

    @Override
    public void
    error(String message)
    {
    }

    @Override
    public Ice.Logger
    cloneWithPrefix(String prefix)
    {
        return new DummyLogger();
    }
}
