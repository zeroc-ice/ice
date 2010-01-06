// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Filesystem;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceLifecycleClient")]
[assembly: AssemblyDescription("Ice lifecycle demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        public override int run(String[] args)
        {
            //
            // Terminate cleanly on receipt of a signal.
            //
            shutdownOnInterrupt();

            //
            // Create a proxy for the root directory
            //
            Ice.ObjectPrx @base = communicator().stringToProxy("RootDir:default -p 10000");

            //
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

        private class Error : SystemException
        {
            public Error(String msg)
                : base(msg)
            {
            }
        }
    }

    static public void Main(String[] args)
    {
        App app = new App();
        app.main(args);
    }
}
