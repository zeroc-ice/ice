// Copyright (c) ZeroC, Inc.

using System.Collections.Immutable;

namespace IceBox;

public static class Server
{
    private static void usage()
    {
        Console.Error.WriteLine("Usage: iceboxnet [options] --Ice.Config=<file>\n");
        Console.Error.WriteLine(
            "Options:\n" +
            "-h, --help           Show this message.\n" +
            "-v, --version        Display the Ice version.");
    }

    private static int run(Ice.Communicator communicator, string[] args)
    {
        const string prefix = "IceBox.Service.";
        Ice.Properties properties = communicator.getProperties();
        Dictionary<string, string> services = properties.getPropertiesForPrefix(prefix);

        var argSeq = new List<string>(args);
        foreach (KeyValuePair<string, string> pair in services)
        {
            string name = pair.Key[prefix.Length..];
            argSeq.RemoveAll(v => v.StartsWith($"--{name}", StringComparison.Ordinal));
        }

        foreach (string arg in args)
        {
            if (arg == "-h" || arg == "--help")
            {
                usage();
                return 0;
            }
            else if (arg == "-v" || arg == "--version")
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

        var serviceManagerImpl = new ServiceManagerI(communicator, args);
        return serviceManagerImpl.run();
    }

    public static int Main(string[] args)
    {
        int status = 0;

        var initData = new Ice.InitializationData
        {
            properties = new Ice.Properties(ref args, ["IceBox"])
        };
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");

        try
        {
            using Ice.Communicator communicator = Ice.Util.initialize(initData);
            Console.CancelKeyPress += (sender, eventArgs) =>
            {
                eventArgs.Cancel = true;
                communicator.shutdown();
            };

            status = run(communicator, args);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }

        return status;
    }
}
