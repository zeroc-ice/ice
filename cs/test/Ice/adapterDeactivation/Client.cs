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
    internal class TestClient : Ice.Application
    {
        public override int run(string[] args)
        {
            TestPrx obj = AllTests.allTests(communicator());
            
            System.Console.Out.Write("testing whether server is gone... ");
            System.Console.Out.Flush();
            try
            {
                obj.ice_ping();
                throw new System.Exception();
            }
            catch(Ice.LocalException)
            {
                System.Console.Out.WriteLine("ok");
            }
	    catch(System.Exception ex)
	    {
	        System.Console.Error.WriteLine(ex);
		return 1;
	    }
            
            return 0;
        }
    }
    
    public static void Main(string[] args)
    {
        TestClient app = new TestClient();
        int result = app.main(args);
        System.Environment.Exit(result);
    }
}
