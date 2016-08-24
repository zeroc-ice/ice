// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.checksum;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010");
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        com.zeroc.Ice.Object object = new ChecksumI();
        adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("test"));
        adapter.activate();
        return WAIT;
    }
    
    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.checksum.server");
        return r;
    }

    public static void main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);
        
        System.gc();
        System.exit(status);
    }
}
