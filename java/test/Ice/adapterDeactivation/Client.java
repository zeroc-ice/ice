// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
