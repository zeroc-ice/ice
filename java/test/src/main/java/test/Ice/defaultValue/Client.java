// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.defaultValue;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        try(com.zeroc.Ice.Communicator communicator = initialize(args))
        {
            AllTests.allTests(this);
        }
    }
}
