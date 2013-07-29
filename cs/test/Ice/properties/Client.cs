// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    class PropertiesClient : Ice.Application
    {
        public override int
        run(String[] args)
        {
            Ice.Properties properties = communicator().getProperties();
            test(properties.getProperty("Ice.Trace.Network").Equals("1"));
            test(properties.getProperty("Ice.Trace.Protocol").Equals("1"));
            test(properties.getProperty("Config.Path").Equals("./config/中国_client.config"));
            test(properties.getProperty("Ice.ProgramName").Equals("PropertiesClient"));
            test(appName().Equals(properties.getProperty("Ice.ProgramName")));
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        int status = 0;
        try
        {
            Console.Out.Write("testing load properties from UTF-8 path... ");
            Console.Out.Flush();
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("./config/中国_client.config");
            test(properties.getProperty("Ice.Trace.Network").Equals("1"));
            test(properties.getProperty("Ice.Trace.Protocol").Equals("1"));
            test(properties.getProperty("Config.Path").Equals("./config/中国_client.config"));
            test(properties.getProperty("Ice.ProgramName").Equals("PropertiesClient"));
            Console.Out.WriteLine("ok");
            Console.Out.Write("testing load properties from UTF-8 path using Ice::Application... ");
            Console.Out.Flush();
            PropertiesClient c = new PropertiesClient();
            c.main(args, "./config/中国_client.config");
            Console.Out.WriteLine("ok");
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
            status = 1;
        }
        
        //
        // Try to load multiple config files.
        //
        try
        {
            Console.Out.Write("testing using Ice.Config with multiple config files... ");
            Console.Out.Flush();
            string[] args1 = new string[]{"--Ice.Config=config/config.1, config/config.2, config/config.3"};
            Ice.Properties properties = Ice.Util.createProperties(ref args1);
            test(properties.getProperty("Config1").Equals("Config1"));
            test(properties.getProperty("Config2").Equals("Config2"));
            test(properties.getProperty("Config3").Equals("Config3"));
            Console.Out.WriteLine("ok");
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
            status = 1;
        }
        return status;
    }
}
