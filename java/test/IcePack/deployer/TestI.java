// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class TestI extends _TestDisp
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
