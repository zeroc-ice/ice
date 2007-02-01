// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class SessionI extends Glacier2._SessionDisp
{
    SessionI(String userId)
    {
        _userId = userId;
    }

    public void
    destroy(Ice.Current current)
    {
        System.out.println("destroying session for user `" + _userId + "'");
        current.adapter.remove(current.id);
    }

    final private String _userId;
}
