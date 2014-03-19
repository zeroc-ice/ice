// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using IceBox;

class HelloServiceI : IceBox.Service
{
    public void start(string name, Ice.Communicator communicator, string[] args)
    {
        _adapter = communicator.createObjectAdapter("Hello-" + name);
        
        string helloIdentity = communicator.getProperties().getProperty("Hello.Identity");
        _adapter.add(new HelloI(name), communicator.stringToIdentity(helloIdentity));
        _adapter.activate();
    }

    public void stop()
    {
        _adapter.destroy();
    }

    private Ice.ObjectAdapter _adapter;
}
