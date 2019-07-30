//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.scope;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.scope");
        java.io.PrintWriter out = getWriter();
        try(Ice.Communicator communicator = initialize(properties))
        {
            out.print("test using same type name in different Slice modules... ");
            out.flush();
            AllTests.allTests(this);
            out.println("ok");
        }
    }
}
