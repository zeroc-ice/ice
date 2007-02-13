// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public class Client
{
    static class TestClient extends Ice.Application
    {
        public int
        run(String[] args)
        {
            TestIntfPrx obj = AllTests.allTests(communicator(), false);
            obj.shutdown();
            return 0;
        }
    }

    public static void
    main(String[] args)
    {
        TestClient app = new TestClient();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
