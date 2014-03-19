// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        _m.Lock();
        try
        {
            while(!_called)
            {
                _m.Wait();
            }
        }
        finally
        {
            _m.Unlock();
        }
        _called = false;

        return _changes;
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

    private Dictionary<string, string> _changes;
    private bool _called;
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
}
