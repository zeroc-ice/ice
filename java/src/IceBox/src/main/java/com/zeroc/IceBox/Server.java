// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceBox;

public final class Server
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
        int status = 0;
        java.util.List<String> argSeq = new java.util.ArrayList<String>();

        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = com.zeroc.Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");

        try(com.zeroc.Ice.Communicator communicator = com.zeroc.Ice.Util.initialize(args, initData, argSeq))
        {
            Runtime.getRuntime().addShutdownHook(new Thread(() ->
            {
                communicator.shutdown();
            }));

            final String prefix = "IceBox.Service.";
            com.zeroc.Ice.Properties properties = communicator.getProperties();
            java.util.Map<String, String> services = properties.getPropertiesForPrefix(prefix);

            for(String arg : argSeq)
            {
                boolean valid = false;
                for(java.util.Map.Entry<String, String> entry : services.entrySet())
                {
                    String name = entry.getKey().substring(prefix.length());
                    if(arg.startsWith("--" + name))
                    {
                        valid = true;
                    }
                }

                if(!valid)
                {
                    if(arg.equals("-h") || arg.equals("--help"))
                    {
                        usage();
                        status = 0;
                        break;
                    }
                    else
                    {
                        System.err.println("Server: unknown option `" + arg + "'");
                        usage();
                        status = 1;
                        break;
                    }
                }
            }

            ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, argseq.toArray(new String[argseq.size()]));
            status = serviceManagerImpl.run();
        }

        System.exit(status);
    }
}
