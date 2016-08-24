// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ice;

public class IceAdapter 
{
    synchronized static public void add(com.zeroc.Ice.Object servant, com.zeroc.Ice.Identity id)
    {
        if(_objectAdapter == null)
        {
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = com.zeroc.Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Default.Package", "com.zeroc.ice");
            initData.properties.setProperty("Ice.UseApplicationClassLoader", "1");
            com.zeroc.Ice.Communicator communicator = com.zeroc.Ice.Util.initialize(initData);
            _objectAdapter = communicator.createObjectAdapterWithEndpoints("EJB", "tcp -h localhost -p 10000");
            _objectAdapter.activate();
        }
        ++_servantCount;
        _objectAdapter.add(servant, id);
    }

    synchronized static public void remove(com.zeroc.Ice.Identity id)
    {
        assert _servantCount > 0;
        _objectAdapter.remove(id);
        if(--_servantCount == 0)
        {
            _objectAdapter.getCommunicator().destroy();
            _objectAdapter = null;
        }
    }

    synchronized static public com.zeroc.Ice.ObjectPrx stringToProxy(String str)
    {
        assert _objectAdapter != null;
        return _objectAdapter.getCommunicator().stringToProxy(str);
    }

    static private com.zeroc.Ice.ObjectAdapter _objectAdapter = null;
    static private int _servantCount;
}
