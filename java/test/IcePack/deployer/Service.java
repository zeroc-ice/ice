// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Service extends Ice.LocalObjectImpl implements IceBox.Service
{
    public void
    start(String name, Ice.Communicator communicator, String[] args)
        throws IceBox.FailureException
    {
	Ice.Properties properties = communicator.getProperties();

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter(name);
        Ice.Object object = new TestI(adapter, communicator.getProperties());
        adapter.add(object, Ice.Util.stringToIdentity(properties.getProperty(name + ".Identity")));
        adapter.activate();
    }

    public void
    stop()
    {
    }
}
