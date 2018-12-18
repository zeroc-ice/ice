// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.admin;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        try(Ice.Communicator communicator = initialize(args))
        {
            AllTests.allTests(this);
        }
    }
}
