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
    public class App : Ice.Application
    {
        private static void usage()
        {
            Console.Error.WriteLine("Usage: iceboxnet [options] --Ice.Config=<file>\n");
            Console.Error.WriteLine(
                "Options:\n" +
                "-h, --help           Show this message.\n"
            );
        }

        public override int run(string[] args)
        {
            List<String> argSeq = new List<String>(args);
            const String prefix = "IceBox.Service.";
            Ice.Properties properties = communicator().getProperties();
            Dictionary<string, string> services = properties.getPropertiesForPrefix(prefix);
            foreach(KeyValuePair<string, string> pair in services)
            {
                String name = pair.Key.Substring(prefix.Length);
                for(int i = 0; i < argSeq.Count; ++i)
                {
                    if(argSeq[i].StartsWith("--" + name, StringComparison.CurrentCulture))
                    {
                        argSeq.RemoveAt(i);
                        i--;
                    }
                }
            }

            foreach(String s in argSeq)
            {
                if(s.Equals("-h") || s.Equals("--help"))
                {
                    usage();
                    return 0;
                }
                else
                {
                    Console.Error.WriteLine("Server: unknown option `" + s + "'");
                    usage();
                    return 1;
                }
            }

            ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator(), args);
            return serviceManagerImpl.run();
        }
    }

    public static int Main(string[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        App server = new App();
        return server.main(args, initData);
    }
}
}
