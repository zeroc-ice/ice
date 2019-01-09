// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.udp;

import test.Ice.udp.Test.*;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.Properties properties = createTestProperties(argsH);
        properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.RcvSize", "16384");
        properties.setProperty("Ice.UDP.SndSize", "16384");
        try(Ice.Communicator communicator = initialize(properties))
        {
            AllTests.allTests(this);
            int num = argsH.value.length == 1 ? Integer.parseInt(argsH.value[0]) : 1;
            for(int i = 0; i < num; ++i)
            {
                Ice.ObjectPrx prx = communicator.stringToProxy("control:" + getTestEndpoint(i, "tcp"));
                TestIntfPrxHelper.uncheckedCast(prx).shutdown();
            }
        }
    }
}
