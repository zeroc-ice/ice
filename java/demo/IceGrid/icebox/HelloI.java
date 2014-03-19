// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class HelloI extends _HelloDisp
{
    HelloI(String serviceName)
    {
        _serviceName = serviceName;
    }

    public void
    sayHello(Ice.Current current)
    {
        java.util.Map<String,String> env =  System.getenv();
        String lang = env.containsKey("LANG") ? env.get("LANG") : "en";
        String greeting = "Hello, ";
        if(lang.equals("fr"))
        {
            greeting = "Bonjour, ";
        }
        else if(lang.equals("de"))
        {
            greeting = "Hallo, ";
        }
        else if(lang.equals("es"))
        {
            greeting = "Hola, ";
        }
        else if(lang.equals("it"))
        {
            greeting = "Ciao, ";
        }        
        System.out.println(greeting + _serviceName);
    }

    private String _serviceName;
}
