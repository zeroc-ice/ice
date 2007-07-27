// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
using System;
using Filesystem;

public class Client : Ice.Application
{
    public override int run(String[] args)
    {
        // Terminate cleanly on receipt of a signal.
        //
        shutdownOnInterrupt();

        // Create a proxy for the root directory
        //
        Ice.ObjectPrx @base = communicator().stringToProxy("RootDir:default -p 10000");
        if(@base == null)
        {
            throw new Error("Could not create proxy");
        }

        // Down-cast the proxy to a Directory proxy.
        //
        DirectoryPrx rootDir = DirectoryPrxHelper.checkedCast(@base);
        if(rootDir == null)
        {
            throw new Error("Invalid proxy");
        }

        Parser p = new Parser(rootDir);
        return p.parse();
    }

    static public void Main(String[] args)
    {
        Client app = new Client();
        app.main(args);
    }

    private class Error : SystemException
    {
        public Error(String msg)
            : base(msg)
        {
        }
    }
}
