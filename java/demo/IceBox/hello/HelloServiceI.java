// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class HelloServiceI implements IceBox.Service
{
    public void
    start(String name, Ice.Communicator communicator, Ice.Properties properties, String[] args)
        throws IceBox.FailureException
    {
        _adapter = communicator.createObjectAdapter(name + "Adapter");
        Ice.Object object = new HelloI(communicator);
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
