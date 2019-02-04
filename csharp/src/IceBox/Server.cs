//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
            "-h, --help           Show this message.\n" +
            "-v, --version        Display the Ice version."
        );
    }

    private static int run(Ice.Communicator communicator, string[] args)
    {
        const string prefix = "IceBox.Service.";
        Ice.Properties properties = communicator.getProperties();
        Dictionary<string, string> services = properties.getPropertiesForPrefix(prefix);

        var argSeq = new List<string>(args);
        foreach(KeyValuePair<string, string> pair in services)
        {
            string name = pair.Key.Substring(prefix.Length);
            argSeq.RemoveAll(v => v.StartsWith("--" + name));
        }

        foreach(string arg in args)
        {
            if(arg.Equals("-h") || arg.Equals("--help"))
            {
                usage();
                return 0;
            }
            else if(arg.Equals("-v") || arg.Equals("--version"))
            {
                Console.Out.WriteLine(Ice.Util.stringVersion());
                return 0;
            }
            else
            {
                Console.Error.WriteLine("IceBox.Server: unknown option `" + arg + "'");
                usage();
                return 1;
            }
        }

        ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, args);
        return serviceManagerImpl.run();
    }

    public static int Main(string[] args)
    {
        int status = 0;

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

                status = run(communicator, args);
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
