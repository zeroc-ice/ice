// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class HelloServiceI extends Ice.LocalObjectImpl implements IceBox.Service
{
    public void
    start(String name, Ice.Communicator communicator, String[] args)
        throws IceBox.FailureException
    {
        _adapter = communicator.createObjectAdapter("Hello");

        String id = communicator.getProperties().getProperty("Identity");

        Ice.Object object = new HelloFactoryI();
        _adapter.add(object, Ice.Util.stringToIdentity(id));
        _adapter.activate();
    }

    public void
    stop()
    {
        _adapter.deactivate();
    }

    private Ice.ObjectAdapter _adapter;
}
