// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
