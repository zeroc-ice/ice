// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class DummyLogger extends Ice.LocalObjectImpl implements Ice.Logger
{
    public void
    print(String message)
    {
    }

    public void
    trace(String category, String message)
    {
    }

    public void
    warning(String message)
    {
    }

    public void
    error(String message)
    {
    }
}
