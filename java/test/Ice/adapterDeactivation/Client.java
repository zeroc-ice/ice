// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public class Client
{
    static class TestClient extends Ice.Application
    {
        public int
        run(String[] args)
        {
            AllTests.allTests(communicator());
            return 0;
        }
    }

    public static void
    main(String[] args)
    {
        TestClient app = new TestClient();
        int result = app.main("Client", args);
        System.exit(result);
    }
}
