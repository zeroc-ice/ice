// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.properties;

public class Client extends test.Util.Application
{
    public static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    class PropertiesClient extends Ice.Application
    {
        public int
        run(String[] args)
        {
            Ice.Properties properties = communicator().getProperties();
            test(properties.getProperty("Ice.Trace.Network").equals("1"));
            test(properties.getProperty("Ice.Trace.Protocol").equals("1"));
            test(properties.getProperty("Config.Path").equals(configPath));
            test(properties.getProperty("Ice.ProgramName").equals("PropertiesClient"));
            test(appName().equals(properties.getProperty("Ice.ProgramName")));
            return 0;
        };
    };

    public int run(String[] args)
    {
        System.out.print("testing load properties from UTF-8 path... ");
        Ice.InitializationData id = new Ice.InitializationData();
        id.properties = Ice.Util.createProperties();
        id.properties.load(configPath);
        test(id.properties.getProperty("Ice.Trace.Network").equals("1"));
        test(id.properties.getProperty("Ice.Trace.Protocol").equals("1"));
        test(id.properties.getProperty("Config.Path").equals(configPath));
        test(id.properties.getProperty("Ice.ProgramName").equals("PropertiesClient"));
        System.out.println("ok");
        System.out.print("testing load properties from UTF-8 path using Ice::Application... ");
        PropertiesClient c = new PropertiesClient();
        c.main("", args, configPath);
        System.out.println("ok");
        return 0;
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);
        System.gc();
        System.exit(status);
    }

    private static String configPath = "./config/\u4E2D\u56FD_client.config";
}
