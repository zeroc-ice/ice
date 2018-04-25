// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.properties;

import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

public class Client extends test.Util.Application
{
    public static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    class PropertiesClient extends com.zeroc.Ice.Application
    {
        @Override
        public int run(String[] args)
        {
            Properties properties = communicator().getProperties();
            test(properties.getProperty("Ice.Trace.Network").equals("1"));
            test(properties.getProperty("Ice.Trace.Protocol").equals("1"));
            test(properties.getProperty("Config.Path").equals(configPath));
            test(properties.getProperty("Ice.ProgramName").equals("PropertiesClient"));
            test(appName().equals(properties.getProperty("Ice.ProgramName")));
            return 0;
        }
    }

    @Override
    public int run(String[] args)
    {
        {
            System.out.print("testing load properties from UTF-8 path... ");
            Properties properties = Util.createProperties();
            properties.load(configPath);
            test(properties.getProperty("Ice.Trace.Network").equals("1"));
            test(properties.getProperty("Ice.Trace.Protocol").equals("1"));
            test(properties.getProperty("Config.Path").equals(configPath));
            test(properties.getProperty("Ice.ProgramName").equals("PropertiesClient"));
            System.out.println("ok");
            System.out.print("testing load properties from UTF-8 path using Ice::Application... ");
            PropertiesClient c = new PropertiesClient();
            c.main("", args, configPath);
            System.out.println("ok");
        }
        {
            //
            // Try to load multiple config files.
            //
            System.out.print("testing using Ice.Config with multiple config files... ");
            String[] args1 = new String[]{"--Ice.Config=config/config.1, config/config.2, config/config.3"};
            Properties properties = Util.createProperties(args1);
            test(properties.getProperty("Config1").equals("Config1"));
            test(properties.getProperty("Config2").equals("Config2"));
            test(properties.getProperty("Config3").equals("Config3"));
            System.out.println("ok");
        }

        {
            System.out.print("testing configuration file escapes... ");
            String[] args1 = new String[]{"--Ice.Config=config/escapes.cfg"};
            Properties properties = Util.createProperties(args1);

            String[] props = new String[]{"Foo\tBar", "3",
                                          "Foo\\tBar", "4",
                                          "Escape\\ Space", "2",
                                          "Prop1", "1",
                                          "Prop2", "2",
                                          "Prop3", "3",
                                          "My Prop1", "1",
                                          "My Prop2", "2",
                                          "My.Prop1", "a property",
                                          "My.Prop2", "a     property",
                                          "My.Prop3", "  a     property  ",
                                          "My.Prop4", "  a     property  ",
                                          "My.Prop5", "a \\ property",
                                          "foo=bar", "1",
                                          "foo#bar", "2",
                                          "foo bar", "3",
                                          "A", "1",
                                          "B", "2 3 4",
                                          "C", "5=#6",
                                          "AServer", "\\\\server\\dir",
                                          "BServer", "\\server\\dir",
                                          ""};

            for(int i = 0; !props[i].isEmpty(); i += 2)
            {
                test(properties.getProperty(props[i]).equals(props[i + 1]));
            }
            System.out.println("ok");
        }

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
