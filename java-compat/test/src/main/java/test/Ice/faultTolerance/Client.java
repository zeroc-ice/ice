//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.faultTolerance;

import java.io.PrintWriter;

public class Client extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.Properties properties = createTestProperties(argsH);
        properties.setProperty("Ice.Package.Test", "test.Ice.faultTolerance");
        //
        // This test aborts servers, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");
        PrintWriter out = getWriter();

        try(Ice.Communicator communicator = initialize(properties))
        {
            java.util.List<Integer> ports = new java.util.ArrayList<Integer>(argsH.value.length);
            for(String arg : argsH.value)
            {
                if(arg.charAt(0) == '-')
                {
                    //
                    // Arguments recognized by the communicator are not
                    // removed from the argument list.
                    //
                    continue;
                }
                ports.add(Integer.parseInt(arg));
            }

            if(ports.isEmpty())
            {
                throw new RuntimeException("Client: no ports specified");
            }

            int[] arr = new int[ports.size()];
            for(int i = 0; i < arr.length; i++)
            {
                arr[i] = ports.get(i).intValue();
            }

            AllTests.allTests(this, arr);
        }
    }
}
