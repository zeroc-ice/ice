// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceBox.admin;

import test.IceBox.admin.Test.*;

public class TestFacetI extends _TestFacetDisp implements Ice.PropertiesAdminUpdateCallback
{
    public TestFacetI()
    {
        _called = false;
    }

    @Override
    public synchronized java.util.Map<String, String> getChanges(Ice.Current current)
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
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
            }
        }

        _called = false;

        return _changes;
    }

    @Override
    public synchronized void updated(java.util.Map<String, String> changes)
    {
        _changes = changes;
        _called = true;
        notify();
    }

    private java.util.Map<String, String> _changes;
    private boolean _called;
}
