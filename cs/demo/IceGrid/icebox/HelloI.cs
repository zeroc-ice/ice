// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public class HelloI : HelloDisp_
{
    public HelloI(string serviceName)
    {
        _serviceName = serviceName;
    }

    public override void sayHello(Ice.Current current)
    {
#if COMPACT || SILVERLIGHT
        string lang = "en";
#else
        string lang = System.Environment.GetEnvironmentVariable("LANG") != null ? 
                                                            System.Environment.GetEnvironmentVariable("LANG") : "en";
#endif
        string greeting = "Hello, ";
        if(lang.Equals("fr"))
        {
            greeting = "Bonjour, ";
        }
        else if(lang.Equals("de"))
        {
            greeting = "Hallo, ";
        }
        else if(lang.Equals("es"))
        {
            greeting = "Hola, ";
        }
        else if(lang.Equals("it"))
        {
            greeting = "Ciao, ";
        }
        System.Console.Out.WriteLine(greeting + _serviceName);
    }

    private string _serviceName;
}
