// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
        _adapter = communicator.createObjectAdapter(name);
        Ice.Object object = new HelloI();
        _adapter.add(object, Ice.Util.stringToIdentity("hello"));
        _adapter.activate();
    }

    public void
    stop()
    {
        _adapter.deactivate();
    }

    private Ice.ObjectAdapter _adapter;
}
