// **********************************************************************
//
// Copyright (c) 2002
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
            TestPrx obj = AllTests.allTests(communicator());

            System.out.print("testing whether server is gone... ");
            System.out.flush();
            try
            {
                obj.ice_ping();
                throw new RuntimeException();
            }
            catch(Ice.LocalException ex)
            {
                System.out.println("ok");
            }

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
