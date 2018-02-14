// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;

using Test;

public class TestFacetI : TestFacetDisp_, Ice.PropertiesAdminUpdateCallback
{
    public TestFacetI()
    {
        _called = false;
    }

    override public Dictionary<string, string> getChanges(Ice.Current current)
    {
        //
        // The client calls PropertiesAdmin::setProperties() and then invokes
        // this operation. Since setProperties() is implemented using AMD, the
        // client might receive its reply and then call getChanges() before our
        // updated() method is called. We block here to ensure that updated()
        // gets called before we return the most recent set of changes.
        //
        lock(this)
        {
            while(!_called)
            {
                System.Threading.Monitor.Wait(this);
            }
        }
        _called = false;

        return _changes;
    }

    public void updated(Dictionary<string, string> changes)
    {
        lock(this)
        {
            _changes = changes;
            _called = true;
            System.Threading.Monitor.Pulse(this);
        }
    }

    private Dictionary<string, string> _changes;
    private bool _called;
}
