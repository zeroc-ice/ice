// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public class TestI extends _TestIntfDisp
{
    TestI(Ice.ObjectAdapter adapter)
    {
	_adapter = adapter;

	Ice.Object servant = new HelloI();
	_adapter.add(servant, Ice.Util.stringToIdentity("hello"));
    }

    public void
    shutdown(Ice.Current current)
    {
	_adapter.getCommunicator().shutdown();
    }

    public HelloPrx
    getHello(Ice.Current current)
    {
	return HelloPrxHelper.uncheckedCast(_adapter.createProxy(Ice.Util.stringToIdentity("hello")));
    }

    private Ice.ObjectAdapter _adapter;
}
