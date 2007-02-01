// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class SessionManagerI extends Glacier2._SessionManagerDisp
{
    public Glacier2.SessionPrx
    create(String userId, Glacier2.SessionControlPrx control, Ice.Current current)
    {
        System.out.println("creating session for user `" + userId + "'");
        Glacier2.Session session = new SessionI(userId);
        return Glacier2.SessionPrxHelper.uncheckedCast(current.adapter.addWithUUID(session));
    }
}
