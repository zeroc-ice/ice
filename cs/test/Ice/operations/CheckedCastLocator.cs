// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class CheckedCastLocator : Ice.LocalObjectImpl, Ice.ServantLocator
{
    public CheckedCastLocator()
    {
        _servant = new TestCheckedCastI();
    }

    public Ice.Object
    locate(Ice.Current c, out Ice.LocalObject cookie)
    {
        if(c.operation.Equals("ice_isA"))
	{
	    _servant.setContext(c.ctx);
	}
        cookie = null;
        return _servant;
    }

    public void
    finished(Ice.Current current, Ice.Object servant, Ice.LocalObject cookie)
    {
    }

    public void deactivate(string s)
    {
    }

    TestCheckedCastI _servant;
}
