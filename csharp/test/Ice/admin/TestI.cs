// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

public class RemoteCommunicatorI : RemoteCommunicatorDisp_
{
    public RemoteCommunicatorI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    override public Ice.ObjectPrx getAdmin(Ice.Current current)
    {
        return _communicator.getAdmin();
    }

    override public Dictionary<string, string> getChanges(Ice.Current current)
    {
        lock(this)
        {
            return _changes;
        }
    }

    override public void print(string message, Ice.Current current)
    {
        _communicator.getLogger().print(message);
    }

    override public void trace(string category, string message, Ice.Current current)
    {
        _communicator.getLogger().trace(category, message);
    }

    override public void warning(string message, Ice.Current current)
    {
        _communicator.getLogger().warning(message);
    }

    override public void error(string message, Ice.Current current)
    {
        _communicator.getLogger().error(message);
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
        lock(this)
        {
            _changes = changes;
        }
    }

    private Ice.Communicator _communicator;
    private Dictionary<string, string> _changes;
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

        if(init.properties.getPropertyAsInt("NullLogger") > 0)
        {
            init.logger = new NullLogger();
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

        if(propFacet != null)
        {
            Ice.NativePropertiesAdmin admin = (Ice.NativePropertiesAdmin)propFacet;
            Debug.Assert(admin != null);
            admin.addUpdateCallback(servant.updated);
        }

        Ice.ObjectPrx proxy = current.adapter.addWithUUID(servant);
        return RemoteCommunicatorPrxHelper.uncheckedCast(proxy);
    }

    override public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private class NullLogger : Ice.Logger
    {
        public void print(string message)
        {
        }

        public void trace(string category, string message)
        {
        }

        public void warning(string message)
        {
        }

        public void error(string message)
        {
        }

        public string getPrefix()
        {
            return "NullLogger";
        }

        public Ice.Logger cloneWithPrefix(string prefix)
        {
            return this;
        }
    }
}
