// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public sealed class SessionManagerI : Glacier2._SessionManagerDisp
{
    public override Glacier2.SessionPrx create(string userId, Ice.Current current)
    {
	Console.WriteLine("creating session for user `" + userId + "'");
	Glacier2.Session session = new SessionI(userId);
	return Glacier2.SessionPrxHelper.uncheckedCast(current.adapter.addWithUUID(session));
    }
}
