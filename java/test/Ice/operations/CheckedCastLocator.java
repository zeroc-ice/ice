// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class CheckedCastLocator extends Ice.LocalObjectImpl implements Ice.ServantLocator
{
    public CheckedCastLocator()
    {
        _servant = new TestCheckedCastI();
    }

    public Ice.Object
    locate(Ice.Current c, Ice.LocalObjectHolder cookie)
    {
        if(c.operation.equals("ice_isA"))
	{
	    _servant.setContext(c.ctx);
	}
        return _servant;
    }

    public void
    finished(Ice.Current current, Ice.Object servant, Ice.LocalObject cookie)
    {
    }

    public void deactivate(String s)
    {
    }

    TestCheckedCastI _servant;
}
