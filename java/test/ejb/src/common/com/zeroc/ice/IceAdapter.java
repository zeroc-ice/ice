// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ice;

public class IceAdapter 
{
    //
    // This interceptor wraps the servant dispatch to setup the thread
    // context class loader. This is required with Ice 3.5b but won't
    // be necessary with 3.5 where the thread context class loader
    // will be set automatically to the servant class loader if the
    // Ice.UseServantClassLoader property is set to 1.
    //
    public static class InterceptorI extends Ice.DispatchInterceptor
    {
        public InterceptorI(Ice.Object servant)
        {
            _servant = servant;
        }

        public Ice.DispatchStatus
        dispatch(Ice.Request request)
        {
            final Thread thread = Thread.currentThread();
            thread.setContextClassLoader(_servant.getClass().getClassLoader());
            try
            {
                return _servant.ice_dispatch(request);
            }
            finally
            {
                thread.setContextClassLoader(null);
            }
        }

        final private Ice.Object _servant;
    };

    synchronized static public void
    add(Ice.Object servant, Ice.Identity id)
    {
        if(_objectAdapter == null)
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Default.Package", "com.zeroc.ice");
            //initData.properties.setProperty("Ice.UseServantClassLoader", "1");
            Ice.Communicator communicator = Ice.Util.initialize(initData);
            _objectAdapter = communicator.createObjectAdapterWithEndpoints("EJB", "tcp -h localhost -p 10000");
            _objectAdapter.activate();
        }
        ++_servantCount;
        _objectAdapter.add(new InterceptorI(servant), id);
        //_objectAdapter.add(servant, id);
    }

    synchronized static public void
    remove(Ice.Identity id)
    {
        assert _servantCount > 0;
        _objectAdapter.remove(id);
        if(--_servantCount == 0)
        {
            _objectAdapter.getCommunicator().destroy();
            _objectAdapter = null;
        }
    }

    synchronized static public Ice.ObjectPrx
    stringToProxy(String str)
    {
        assert _objectAdapter != null;
        return _objectAdapter.getCommunicator().stringToProxy(str);
    }

    static private Ice.ObjectAdapter _objectAdapter = null;
    static private int _servantCount;
};
