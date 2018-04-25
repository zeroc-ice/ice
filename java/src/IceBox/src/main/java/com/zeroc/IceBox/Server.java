// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceBox;

public final class Server extends com.zeroc.Ice.Application
{
    private static void usage()
    {
        System.err.println("Usage: com.zeroc.IceBox.Server [options] --Ice.Config=<file>\n");
        System.err.println(
            "Options:\n" +
            "-h, --help           Show this message.\n"
        );
    }

    public static void main(String[] args)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = com.zeroc.Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");

        Server server = new Server();
        System.exit(server.main("IceBox.Server", args, initData));
    }

    @Override
    public int run(String[] args)
    {
        final String prefix = "IceBox.Service.";
        com.zeroc.Ice.Properties properties = communicator().getProperties();
        java.util.Map<String, String> services = properties.getPropertiesForPrefix(prefix);
        java.util.List<String> argSeq = new java.util.ArrayList<>(args.length);
        for(String s : args)
        {
            argSeq.add(s);
        }

        for(java.util.Map.Entry<String, String> entry : services.entrySet())
        {
            String name = entry.getKey().substring(prefix.length());
            for(int i = 0; i < argSeq.size(); ++i)
            {
                if(argSeq.get(i).startsWith("--" + name))
                {
                    argSeq.remove(i);
                    i--;
                }
            }
        }

        for(String arg : argSeq)
        {
            if(arg.equals("-h") || arg.equals("--help"))
            {
                usage();
                return 0;
            }
            else
            {
                System.err.println("Server: unknown option `" + arg + "'");
                usage();
                return 1;
            }
        }

        ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator(), args);
        return serviceManagerImpl.run();
    }
}
