// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceGrid.simple;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        try(Ice.Communicator communicator = initialize(properties))
        {
            boolean withDeploy = false;
            for(String arg : args)
            {
                if(arg.equals("--with-deploy"))
                {
                    withDeploy = true;
                    break;
                }
            }

            if(!withDeploy)
            {
                AllTests.allTests(this);
            }
            else
            {
                AllTests.allTestsWithDeploy(this);
            }
        }
    }
}
