// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
