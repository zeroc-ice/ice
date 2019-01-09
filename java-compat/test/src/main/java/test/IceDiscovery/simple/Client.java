// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceDiscovery.simple;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.Properties properties = createTestProperties(argsH);
        try(Ice.Communicator communicator = initialize(properties))
        {
            int num;
            try
            {
                num = argsH.value.length == 1 ? Integer.parseInt(argsH.value[0]) : 0;
            }
            catch(NumberFormatException ex)
            {
                num = 0;
            }
            AllTests.allTests(communicator, num);
        }
    }
}
