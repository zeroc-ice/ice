// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.udp;

import test.Ice.udp.Test.*;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        java.util.List<String> rargs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, rargs);
        properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.RcvSize", "16384");
        properties.setProperty("Ice.UDP.SndSize", "16384");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);
            int num;
            try
            {
                num = rargs.size() == 1 ? Integer.parseInt(rargs.get(0)) : 1;
            }
            catch(NumberFormatException ex)
            {
                num = 1;
            }

            for(int i = 0; i < num; ++i)
            {
                com.zeroc.Ice.ObjectPrx prx = communicator().stringToProxy("control:" + getTestEndpoint(i, "tcp"));
                TestIntfPrx.uncheckedCast(prx).shutdown();
            }
        }
    }
}
