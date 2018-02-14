// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using IceBox;

class TestServiceI : IceBox.Service
{
    public void
    start(string name, Ice.Communicator communicator, string[] args)
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
