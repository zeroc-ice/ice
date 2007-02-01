// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public sealed class SessionI : Glacier2.SessionDisp_
{
    public SessionI(string userId)
    {
        _userId = userId;
    }

    public override void destroy(Ice.Current current)
    {
        Console.WriteLine("destroying session for user `" + _userId + "'");
        current.adapter.remove(current.id);
    }

    private readonly string _userId;
}
