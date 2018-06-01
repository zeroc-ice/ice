// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.defaultServant;

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
