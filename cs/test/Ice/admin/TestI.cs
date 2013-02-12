// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

using System.Diagnostics;
using System.Collections.Generic;

public class TestFacetI : TestFacetDisp_
{
    override public void op(Ice.Current current)
    {
    }
}

public class RemoteCommunicatorI : RemoteCommunicatorDisp_, Ice.PropertiesAdminUpdateCallback
{
    public RemoteCommunicatorI(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _called = false;
    }

    override public Ice.ObjectPrx getAdmin(Ice.Current current)
    {
        return _communicator.getAdmin();
    }

    override public Dictionary<string, string> getChanges(Ice.Current current)
    {
        _m.Lock();
        try
        {
            //
            // The client calls PropertiesAdmin::setProperties() and then invokes
            // this operation. Since setProperties() is implemented using AMD, the
            // client might receive its reply and then call getChanges() before our
            // updated() method is called. We block here to ensure that updated()
            // gets called before we return the most recent set of changes.
            //
            while(!_called)
            {
                _m.Wait();
            }
            
            _called = false;
            
            return _changes;
        }
        finally
        {
            _m.Unlock();
        }
    }

    override public void shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    override public void waitForShutdown(Ice.Current current)
    {
        //
        // Note that we are executing in a thread of the *main* communicator,
        // not the one that is being shut down.
        //
        _communicator.waitForShutdown();
    }

    override public void destroy(Ice.Current current)
    {
        _communicator.destroy();
    }

    public void updated(Dictionary<string, string> changes)
    {
        _m.Lock();
        try
        {
            _changes = changes;
            _called = true;
            _m.Notify();
        }
        finally
        {
            _m.Unlock();
        }
    }

    private Ice.Communicator _communicator;
    private Dictionary<string, string> _changes;
    private bool _called;
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
}

public class RemoteCommunicatorFactoryI : RemoteCommunicatorFactoryDisp_
{
    override public RemoteCommunicatorPrx createCommunicator(Dictionary<string, string> props, Ice.Current current)
    {
        //
        // Prepare the property set using the given properties.
        //
        Ice.InitializationData init = new Ice.InitializationData();
        init.properties = Ice.Util.createProperties();
        foreach(KeyValuePair<string, string> e in props)
        {
            init.properties.setProperty(e.Key, e.Value);
        }

        //
        // Initialize a new communicator.
        //
        Ice.Communicator communicator = Ice.Util.initialize(init);

        //
        // Install a custom admin facet.
        //
        communicator.addAdminFacet(new TestFacetI(), "TestFacet");

        //
        // The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
        // Set the callback on the admin facet.
        //
        RemoteCommunicatorI servant = new RemoteCommunicatorI(communicator);
        Ice.Object propFacet = communicator.findAdminFacet("Properties");

        Ice.NativePropertiesAdmin admin = (Ice.NativePropertiesAdmin)propFacet;
        Debug.Assert(admin != null);
        admin.addUpdateCallback(servant);

        Ice.ObjectPrx proxy = current.adapter.addWithUUID(servant);
        return RemoteCommunicatorPrxHelper.uncheckedCast(proxy);
    }

    override public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
