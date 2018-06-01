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
        java.util.List<String> rargs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, rargs);
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            int num;
            try
            {
                num = rargs.size() == 1 ? Integer.parseInt(rargs.get(0)) : 0;
            }
            catch(NumberFormatException ex)
            {
                num = 0;
            }
            AllTests.allTests(this, num);
        }
    }
}
