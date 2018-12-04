// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;

namespace IceBox
{

public class Server
{
    private static void usage()
    {
        Console.Error.WriteLine("Usage: iceboxnet [options] --Ice.Config=<file>\n");
        Console.Error.WriteLine(
            "Options:\n" +
            "-h, --help           Show this message.\n"
        );
    }

    public static int Main(string[] args)
    {
        int status = 0;
        List<string> argSeq = new List<string>();
        const string prefix = "IceBox.Service.";

        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");

        try
        {
            using(var communicator = Ice.Util.initialize(ref args, initData))
            {
                Console.CancelKeyPress += (sender, eventArgs) =>
                {
                    eventArgs.Cancel = true;
                    communicator.shutdown();
                };

                Ice.Properties properties = communicator.getProperties();
                Dictionary<string, string> services = properties.getPropertiesForPrefix(prefix);

                foreach(string arg in argSeq)
                {
                    bool valid = false;
                    foreach(KeyValuePair<string, string> pair in services)
                    {
                        string name = pair.Key.Substring(prefix.Length);
                        if(arg.StartsWith("--" + name, StringComparison.CurrentCulture))
                        {
                            valid = true;
                            break;
                        }
                    }
                    if(!valid)
                    {
                        if(arg.Equals("-h") || arg.Equals("--help"))
                        {
                            usage();
                            status = 1;
                            break;
                        }
                        else if(arg.Equals("-v") || arg.Equals("--version"))
                        {
                            Console.Out.WriteLine(Ice.Util.stringVersion());
                            status = 1;
                            break;
                        }
                        else
                        {
                            Console.Error.WriteLine("IceBox.Server: unknown option `" + arg + "'");
                            usage();
                            status = 1;
                            break;
                        }
                    }
                }

                ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, argSeq.ToArray());
                status = serviceManagerImpl.run();
            }
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }

        return status;
    }
}
}
