// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class TestServiceI implements IceBox.Service
{
    public void
    start(String name, Ice.Communicator communicator, String[] args)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter(name + "OA");
        adapter.add(new TestI(args), communicator.stringToIdentity("test"));
        adapter.activate();
    }

    public void
    stop()
    {
    }
}
