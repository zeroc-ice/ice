// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
