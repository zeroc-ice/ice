// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class TestI extends _TestDisp
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
