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
    public
    TestI(Ice.ObjectAdapter adapter, Ice.Properties properties)
    {
        _adapter = adapter;
	_properties = properties;
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    public String
    getProperty(String name, Ice.Current current)
    {
	return _properties.getProperty(name);
    }

    private Ice.ObjectAdapter _adapter;
    private Ice.Properties _properties;
}
